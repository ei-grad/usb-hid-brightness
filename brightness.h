#include <libusb.h>

uint16_t get_brightness(libusb_device_handle *hdev, int iface);
void set_brightness(libusb_device_handle *hdev, int iface, uint16_t val);
