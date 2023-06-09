#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>

#include "brightness.h"

uint16_t get_brightness(libusb_device_handle *hdev, int iface) {
    uint8_t data[6] = {0, 0, 0, 0, 0, 0};

    int res = libusb_control_transfer(
            hdev,
            0b10100001, // in|class|interface
            0x01, // Get_Report
            0x0300, // feature report type, zero report id
            iface, data, 6, 0
    );

    if (res < 0) {
        fprintf(stderr, "can't get brightness: %s\n",
                libusb_error_name(res));
        exit(EXIT_FAILURE);
    }

    return (uint16_t)data[0] + ((uint16_t)data[1] << 8);
}

void set_brightness(libusb_device_handle *hdev, int iface, uint16_t val) {
    uint8_t data[6] = {
        (uint8_t)(val & 0x00ff),
        (uint8_t)((val >> 8) & 0x00ff),
        0x00, 0x00, 0x00, 0x00
    };

    int res = libusb_control_transfer(
            hdev,
            0b00100001, // out|class|interface
            0x09, // Set_Report
            0x0300, // feature report type, zero report id
            iface, data, 6, 0
    );

    if (res < 0) {
        fprintf(stderr, "can't set brightness: %s\n",
                libusb_error_name(res));
        exit(EXIT_FAILURE);
    }
}
