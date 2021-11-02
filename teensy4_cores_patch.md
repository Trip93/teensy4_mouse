This patch can be used to add 16 bit support for usb_mouse_move. However if you only use the faster polling rates (over a 1000Hz) do not bother it does not matter.
If you want to try polling rates of a 1000Hz and lower 16 bit is recommended to not overflow the 8 bit x and y values as this causes the mouse to spin out.

Considering there are different versions of teensyduino I would recommend to make these changes manually

The following functions need to be adjusted to enable 16 bit x and y support for usb_mouse_move

In C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy4 there are three files usb_mouse.c, usb_desc.c and usb_mouse.h

In usb_mouse.h replace usb_mouse_move with:
```
int usb_mouse_move(int16_t x, int16_t y, int8_t wheel, int8_t horiz);
```
In usb_mouse.c replace usb_mouse_move with:
```
// Move the mouse.  x, y and wheel are -32767 to 32767.  Use 0 for no movement.
int usb_mouse_move(int16_t x, int16_t y, int8_t wheel, int8_t horiz)
{
  //printf("move\n");
  if (x == -32768) x = -32767;
  if (y == -32768) y = -32767;
  if (wheel == -128) wheel = -127;
  if (horiz == -128) horiz = -127;
  
  int8_t buffer[8];
  buffer[0] = 1;
  buffer[1] = usb_mouse_buttons_state;
  buffer[2] = x;
  buffer[3] = (x >> 8);
  buffer[4] = y;
  buffer[5] = (y >> 8);
  buffer[6] = wheel;
  buffer[7] = horiz; 
  return usb_mouse_transmit(buffer, 8);
}
```
In usb_desc.c replace the MOUSE_INTERFACE descriptor with:
```
#ifdef MOUSE_INTERFACE
// Mouse Protocol 1, HID 1.11 spec, Appendix B, page 59-60, with wheel extension
static uint8_t mouse_report_desc[] = {
        0x05, 0x01,                     // Usage Page (Generic Desktop)
        0x09, 0x02,                     // Usage (Mouse)
        0xA1, 0x01,                     // Collection (Application)
        0x85, 0x01,                     //   REPORT_ID (1)
        0x05, 0x09,                     //   Usage Page (Button)
        0x19, 0x01,                     //   Usage Minimum (Button #1)
        0x29, 0x08,                     //   Usage Maximum (Button #8)
        0x15, 0x00,                     //   Logical Minimum (0)
        0x25, 0x01,                     //   Logical Maximum (1)
        0x95, 0x08,                     //   Report Count (8)
        0x75, 0x01,                     //   Report Size (1)
        0x81, 0x02,                     //   Input (Data, Variable, Absolute)
        0x05, 0x01,                     //   Usage Page (Generic Desktop)
        0x09, 0x30,                     //   Usage (X)
        0x09, 0x31,                     //   Usage (Y)
        0x16, 0x01, 0x80,               //   Logical Minimum (-32767)
        0x26, 0xFF, 0x7F,               //   Logical Maximum (32767)
        0x75, 0x10,                     //   Report Size (16),
        0x95, 0x02,                     //   Report Count (2),
        0x81, 0x06,                     //   Input (Data, Variable, Relative)
        0x05, 0x01,                     //   Usage Page (Generic Desktop)
        0x09, 0x38,                     //   Usage (Wheel)
        0x15, 0x81,                     //   Logical Minimum (-127)
        0x25, 0x7F,                     //   Logical Maximum (127)
        0x75, 0x08,                     //   Report Size (8),
        0x95, 0x01,                     //   Report Count (1),
        0x81, 0x06,                     //   Input (Data, Variable, Relative)
        0x05, 0x0C,                     //   Usage Page (Consumer)
        0x0A, 0x38, 0x02,               //   Usage (AC Pan)
        0x15, 0x81,                     //   Logical Minimum (-127)
        0x25, 0x7F,                     //   Logical Maximum (127)
        0x75, 0x08,                     //   Report Size (8),
        0x95, 0x01,                     //   Report Count (1),
        0x81, 0x06,                     //   Input (Data, Variable, Relative)
        0xC0,                           // End Collection
        0x05, 0x01,                     // Usage Page (Generic Desktop)
        0x09, 0x02,                     // Usage (Mouse)
        0xA1, 0x01,                     // Collection (Application)
        0x85, 0x02,                     //   REPORT_ID (2)
        0x05, 0x01,                     //   Usage Page (Generic Desktop)
        0x09, 0x30,                     //   Usage (X)
        0x09, 0x31,                     //   Usage (Y)
        0x15, 0x00,                     //   Logical Minimum (0)
        0x26, 0xFF, 0x7F,               //   Logical Maximum (32767)
        0x75, 0x10,                     //   Report Size (16),
        0x95, 0x02,                     //   Report Count (2),
        0x81, 0x02,                     //   Input (Data, Variable, Absolute)
        0xC0                            // End Collection
};
#endif
```
