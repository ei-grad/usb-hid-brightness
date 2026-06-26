#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <libusb.h>

#include "brightness.h"

typedef struct DeviceInfo {
    char product[256];
    char manufacturer[256];
    libusb_device_handle *handle;
    int interface;
    const BrightnessProtocol *protocol;
} DeviceInfo;


int get_hid_brightness_devices(
        libusb_context *ctx,
        DeviceInfo **device_list,
        int *device_count
);

void print_devices(
        DeviceInfo *devices,
        int device_count
);

libusb_device_handle* open_first_device(
        DeviceInfo *device_list,
        int device_count
);

#endif // DEVICE_LIST_H
