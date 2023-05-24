#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>

const uint16_t LG_ID_Vendor = 0x43e;

struct support_device {
    uint16_t product_id;
    char *name;
};

struct support_device support_devices[] = {
    {0x9a63, "24MD4KL"},
    {0x9a70, "27MD5KL"},
    {0x9a40, "27MD5KA"},
    {0, NULL}
};

int is_supported_device(uint16_t product_id) {
    struct support_device *dev = support_devices;
    while (dev->name != NULL) {
        if (dev->product_id == product_id)
            return 1;
        dev++;
    }
    return 0;
}

libusb_device* find_lg_ultrafine_usb_device(libusb_context *ctx) {
    libusb_device **devs;

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Unable to get USB device list.\n");
        libusb_exit(ctx);
        return NULL;
    }

    ssize_t i = 0;
    libusb_device *device = NULL;
    for (i = 0; i < cnt; i++) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0) {
            continue;
        }

        if (desc.idVendor == LG_ID_Vendor && is_supported_device(desc.idProduct)) {
            device = devs[i];
            break;
        }
    }

    if (device == NULL) {
        fprintf(stderr, "No supported devices found.\n");
        libusb_free_device_list(devs, 1);
        libusb_exit(ctx);
        return NULL;
    }

    libusb_device *found_device = libusb_ref_device(device);
    libusb_free_device_list(devs, 1);

    return found_device;
}

libusb_device_handle* open_device(libusb_context *ctx) {
    libusb_device *device = find_lg_ultrafine_usb_device(ctx);
    if (device == NULL) {
        return NULL;
    }

    libusb_device_handle *handle = NULL;
    int err = libusb_open(device, &handle);
    if (err != 0) {
        fprintf(stderr, "Error opening device: %s\n", libusb_error_name(err));
        libusb_unref_device(device);
        return NULL;
    }

    libusb_unref_device(device);
    return handle;
}
