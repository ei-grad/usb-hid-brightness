#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb.h>

#include "brightness.h"

const BrightnessProtocol BRIGHTNESS_PROTOCOL_LG = {
    .report_id = 0,
    .report_len = 6,
    .value_offset = 0,
    .value_size = 2,
    .min = 0,
    .max = 54000,
};

const BrightnessProtocol BRIGHTNESS_PROTOCOL_APPLE = {
    .report_id = 1,
    .report_len = 7,
    .value_offset = 1,
    .value_size = 4,
    .min = 400,
    .max = 60000,
};

uint32_t get_brightness(libusb_device_handle *hdev, int iface,
                        const BrightnessProtocol *proto) {
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    int res = libusb_control_transfer(
            hdev,
            0b10100001, // in|class|interface
            0x01, // Get_Report
            (uint16_t)(0x0300 | proto->report_id), // feature report type | report id
            iface, data, proto->report_len, 0
    );

    if (res < 0) {
        fprintf(stderr, "can't get brightness: %s\n",
                libusb_error_name(res));
        exit(EXIT_FAILURE);
    }

    uint32_t val = 0;
    for (uint8_t i = 0; i < proto->value_size; ++i) {
        val |= (uint32_t)data[proto->value_offset + i] << (8 * i);
    }

    return val;
}

void set_brightness(libusb_device_handle *hdev, int iface,
                    const BrightnessProtocol *proto, uint32_t val) {
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // For numbered reports the report id must lead the payload; for unnumbered
    // reports (report_id == 0) this byte is part of the value and stays zero.
    data[0] = proto->report_id;
    for (uint8_t i = 0; i < proto->value_size; ++i) {
        data[proto->value_offset + i] = (uint8_t)((val >> (8 * i)) & 0xff);
    }

    int res = libusb_control_transfer(
            hdev,
            0b00100001, // out|class|interface
            0x09, // Set_Report
            (uint16_t)(0x0300 | proto->report_id), // feature report type | report id
            iface, data, proto->report_len, 0
    );

    if (res < 0) {
        fprintf(stderr, "can't set brightness: %s\n",
                libusb_error_name(res));
        exit(EXIT_FAILURE);
    }
}
