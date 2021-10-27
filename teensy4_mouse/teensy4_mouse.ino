/*
    Teensy4_mouse.ino - minimal firmware for an 8khz mouse written in arduino.
    Requires a teensy4 MCU and a mouse with the PMW3360 sensor.
    Use the Serial + Keyboard + Mouse + Joystick USB type.
    (part of the code inspired by https://github.com/SunjunKim/PMW3360 and https://github.com/mrjohnk/PMW3360DM-T2QU)

    Copyright (C) 2021  Herbert Trip

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    Contact email: hbtrip93@gmail.com
*/

#include "teensy4_mouse.h"
#include "srom_3360_0x04.h"
#include <SPI.h>

#define SPI_SETTINGS_PMW  SPISettings      (30000000,  MSBFIRST, SPI_MODE3)
#define ACTIVATE_CS_PMW   digitalWriteFast (PMW_CS,    LOW); delayMicroseconds(1)
#define DEACTIVATE_CS_PMW delayMicroseconds(1);              digitalWriteFast (PMW_CS, HIGH)
#define RESET_PMW         digitalWriteFast (PMW_RESET, LOW); delayMicroseconds(1); digitalWriteFast(PMW_RESET, HIGH); delayMicroseconds(1)
#define RESET_SPI         digitalWriteFast (PMW_CS,    LOW); delayMicroseconds(1); digitalWriteFast(PMW_CS,    HIGH); delayMicroseconds(1)

static elapsedMicros burst_timer  = 0;
static elapsedMillis scroll_timer = 0;
static elapsedMillis button_timer = 0;

static int8_t   mouse_data[5] = {0};
static uint16_t cpi[5]        = {400, 800, 1600, 2000, 3200};
static uint8_t  cpi_index     = 3;
static bool     dpi_pressed   = false;
static uint8_t  scroll        = 0;
static int8_t   buttons_old   = 0;

// Writes to a register on the PMW3360
void write_reg_PMW(uint8_t reg, uint8_t value)
{
  reg               |= 0b10000000;
  uint16_t transfer  = (reg << 8) | value;

  ACTIVATE_CS_PMW;
  SPI.transfer16(transfer);
  DEACTIVATE_CS_PMW;

  delayMicroseconds(180);
}

// Reads out a register on the PMW3360
uint8_t read_reg_PMW(uint8_t reg)
{
  reg &= 0b01111111;

  ACTIVATE_CS_PMW;
  SPI.transfer     (reg);
  delayMicroseconds(160);
  uint8_t value = SPI.transfer(0);
  DEACTIVATE_CS_PMW;

  delayMicroseconds(20);

  return value;
}

// Helper function for burst load uploads 1 byte to a preset address
void upload_byte(uint8_t value)
{
  SPI.transfer     (value);
  delayMicroseconds(15);
}

// Uploads a binary firmware file over SPI to the PMW3360
void upload_firmware()
{
  // Prepare to upload the binary file
  write_reg_PMW(REG_Config2, 0x00);
  write_reg_PMW(REG_SROM_Enable, 0x1d);
  delay        (10);
  write_reg_PMW(REG_SROM_Enable, 0x18);

  // Upload the binary file here
  ACTIVATE_CS_PMW;
  upload_byte(REG_SROM_Load_Burst | 0x80);
  for        (uint32_t index = 0; index < 4094; index++) upload_byte(firmware_data[index]);
  DEACTIVATE_CS_PMW;

  read_reg_PMW (REG_SROM_ID);
  write_reg_PMW(REG_Config2, 0x00);
}

// Initializes the PMW3360
uint32_t begin_PMW()
{
  pinMode(PMW_CS   , OUTPUT);
  pinMode(PMW_RESET, OUTPUT);

  SPI.begin();
  RESET_SPI;
  RESET_PMW;
  delay(50);
  SPI.beginTransaction(SPI_SETTINGS_PMW);
  Serial.println("Starting firmware upload");
  upload_firmware();
  SPI.endTransaction();
  delay(10);

  return check_signature();
}

// Checks if the firmware was correctly uploaded
uint32_t check_signature()
{
  SPI.beginTransaction(SPI_SETTINGS_PMW);

  uint8_t pid      = read_reg_PMW(REG_Product_ID);
  uint8_t iv_pid   = read_reg_PMW(REG_Inverse_Product_ID);
  uint8_t SROM_ver = read_reg_PMW(REG_SROM_ID);

  SPI.endTransaction();

  return (pid == 0x42 && iv_pid == 0xBD && SROM_ver == 0x04); // signature for SROM 0x04
}

// Sets the CPI/DPI value of the PMW3360
void set_CPI(uint16_t cpi)
{
  // Limits cpi to 100 - 12000 effectivly with steps of a 100
  cpi = constrain((cpi / 100) - 1, 0, 119);
  write_reg_PMW(REG_Config1, cpi);
}

// Activates motion burst from the PMW3360
void read_burst_start()
{
  ACTIVATE_CS_PMW;
  SPI.transfer(REG_Motion_Burst);
  burst_timer = 0;
}

// Updates mouse x and y axis
void read_burst_end()
{
  int8_t burst[6] = {0};
  //read out the data send by the PMW3360 only read the first 6 bytes
  SPI.transfer(burst, 6);
  DEACTIVATE_CS_PMW;

  // 16 bit x movement data
  mouse_data[0] = burst[2];
  mouse_data[1] = burst[3];
  // 16 bit y movement data
  mouse_data[2] = burst[4];
  mouse_data[3] = burst[5];
}

// Updates the scroll movement
void update_scroll()
{
  // Important to put some time between samples
  if (scroll_timer > 0)
  {
    //get new sample to compare with the old sample 0b0000NNOO
    (!digitalReadFast(MS_1)) ? scroll |= 0b00000100 : scroll &= 0b11111011;
    (!digitalReadFast(MS_0)) ? scroll |= 0b00001000 : scroll &= 0b11110111;

    // Grey code coming from the encoder
    if      ((scroll >> 2         ) == (scroll  & 0b00000011))                                                     mouse_data[4] &= 0b00111111;              // No scrolling if old sample is equal to new sample
    else if ((scroll == 0b00000100) || (scroll == 0b00001101) || (scroll == 0b00001011) || (scroll == 0b00000010)) mouse_data[4] |= 0b10000000 & 0b10111111; // Scroll up   00 -> 01 -> 11 -> 10 -> 00
    else if ((scroll == 0b00001000) || (scroll == 0b00001110) || (scroll == 0b00000111) || (scroll == 0b00000001)) mouse_data[4] |= 0b01000000 & 0b01111111; // Scroll down 00 -> 10 -> 11 -> 01 -> 00

    //save new sample so it can be used in the next loop
    scroll   >>= 2;
    scroll_timer = 0;
  }
}

// Updates the clickable buttons on the mouse
void update_buttons()
{
  // Update main mouse buttons at highest possible speed
  if (!digitalReadFast(M1_NO) && digitalReadFast(M1_NC)) mouse_data[4] |= 0b00000001;
  if (!digitalReadFast(M1_NC) && digitalReadFast(M1_NO)) mouse_data[4] &= 0b11111110;
  if (!digitalReadFast(M2_NO) && digitalReadFast(M2_NC)) mouse_data[4] |= 0b00000010;
  if (!digitalReadFast(M2_NC) && digitalReadFast(M2_NO)) mouse_data[4] &= 0b11111101;
  if (!digitalReadFast(M4_NO) && digitalReadFast(M4_NC)) mouse_data[4] |= 0b00001000;
  if (!digitalReadFast(M4_NC) && digitalReadFast(M4_NO)) mouse_data[4] &= 0b11110111;
  if (!digitalReadFast(M5_NO) && digitalReadFast(M5_NC)) mouse_data[4] |= 0b00010000;
  if (!digitalReadFast(M5_NC) && digitalReadFast(M5_NO)) mouse_data[4] &= 0b11101111;

  // Some browser functionallity bugs out when update rate of middle mouse button exceeds a 1000hz (page drag with middle mouse button)
  // No point updating the dpi button beyond a 1000hz
  if (button_timer > 0)
  {
    button_timer = 0;
    if (!digitalReadFast(M3_NO) && digitalReadFast(M3_NC)) mouse_data[4] |= 0b00000100;
    if (!digitalReadFast(M3_NC) && digitalReadFast(M3_NO)) mouse_data[4] &= 0b11111011;
    if (!digitalReadFast(MD_NO) && digitalReadFast(MD_NC)) mouse_data[4] |= 0b00100000;
    if (!digitalReadFast(MD_NC) && digitalReadFast(MD_NO)) mouse_data[4] &= 0b11011111;
  }
}

// updates dpi avoid using this function within SPI transactions like motion burst
void update_dpi()
{
  // Update dpi on release
  if (  mouse_data[4] & 0b00100000) dpi_pressed = true;
  if (!(mouse_data[4] & 0b00100000) && dpi_pressed)
  {
    dpi_pressed = false;
    cpi_index < 4 ? cpi_index++ : cpi_index = 0;
    SPI.beginTransaction(SPI_SETTINGS_PMW);
    set_CPI(cpi[cpi_index]);
    //start motion burst mode again
    write_reg_PMW(REG_Motion_Burst, 0x00);
    SPI.endTransaction();
  }
}

// Passes the button, scroll and mouse movment information to the USB bus for the next poll
void update_usb()
{
  int8_t scroll           = (mouse_data[4] & 0b10000000) ? scroll =  1 : (mouse_data[4] & 0b01000000) ? scroll = -1 : scroll = 0;
  usb_mouse_buttons_state =  mouse_data[4];
  buttons_old             =  mouse_data[4];
  // This function is blocking and blocks until the next USB poll (125uS at USB 2.0 HIGH speed and 1mS below that speed)
  // USB only accepts 8 bit data other 8 bits ignored, data should always be smaller then 256 at high polling rates (1000Hz and higher)
  usb_mouse_move(mouse_data[0], mouse_data[2], scroll, 0);
}

// Sets up all the buttons and the sensor
void setup()
{
  // Mouse buttons
  pinMode(M1_NO, INPUT_PULLUP);
  pinMode(M1_NC, INPUT_PULLUP);
  pinMode(M2_NO, INPUT_PULLUP);
  pinMode(M2_NC, INPUT_PULLUP);
  pinMode(M3_NO, INPUT_PULLUP);
  pinMode(M3_NC, INPUT_PULLUP);
  pinMode(M4_NO, INPUT_PULLUP);
  pinMode(M4_NC, INPUT_PULLUP);
  pinMode(M4_NO, INPUT_PULLUP);
  pinMode(M4_NC, INPUT_PULLUP);
  pinMode(M5_NO, INPUT_PULLUP);
  pinMode(M5_NC, INPUT_PULLUP);
  pinMode(MD_NO, INPUT_PULLUP);
  pinMode(MD_NC, INPUT_PULLUP);

  // Scroll wheel
  pinMode(MS_0, INPUT_PULLUP);
  pinMode(MS_1, INPUT_PULLUP);

  Serial.println("Starting PMW");
  while (!begin_PMW()) Serial.println("Problem starting PMW");
  Serial.println("PMW succesfully started");

  SPI.beginTransaction(SPI_SETTINGS_PMW);

  //set cpi then start burst mode after
  write_reg_PMW(REG_Lift_Config , 0b00000011);
  set_CPI      (2000);
  write_reg_PMW(REG_Motion_Burst, 0x00);
  SPI.endTransaction();
}

// Gathers mouse data and sends it over the USB
void loop()
{
  mouse_data[0] = 0;
  mouse_data[1] = 0;
  mouse_data[2] = 0;
  mouse_data[3] = 0;
  //check if there is new motion data from interrupt pin
  if (!digitalReadFast(PMW_MOTION))
  {
    SPI.beginTransaction(SPI_SETTINGS_PMW);
    read_burst_start();
    // Burst data only available after 35 microseconds update other things in the mean time
    while (burst_timer < 35)
    {
      update_buttons();
      update_scroll();
    }
    read_burst_end();
    SPI.endTransaction();

    update_usb();
  }
  // If there is no motion data only update the buttons and scroll wheel
  else
  {
    update_dpi();
    update_buttons();
    update_scroll();
    if (mouse_data[4] != buttons_old) update_usb();
  }
}
