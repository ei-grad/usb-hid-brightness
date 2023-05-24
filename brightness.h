#include <libusb.h>

uint16_t get_brightness(libusb_device_handle *hdev);
void set_brightness(libusb_device_handle *hdev, uint16_t val);
