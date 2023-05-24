#include <libusb.h>

#include "brightness.h"

uint16_t get_brightness(libusb_device_handle *hdev) {
    uint8_t data[6];

    libusb_control_transfer(hdev,
                            LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                            LIBUSB_REQUEST_CLEAR_FEATURE, 0x0300, 1, data, 6, 0);

    return (uint16_t)data[0] + ((uint16_t)data[1] << 8);
}

void set_brightness(libusb_device_handle *hdev, uint16_t val) {
    uint8_t data[6] = {
        (uint8_t)(val & 0x00ff),
        (uint8_t)((val >> 8) & 0x00ff),
        0x00, 0x00, 0x00, 0x00
    };

    libusb_control_transfer(hdev,
                            // cppcheck-suppress badBitmaskCheck
                            LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                            LIBUSB_REQUEST_SET_CONFIGURATION, 0x0300, 1, data, 6, 0);
}
