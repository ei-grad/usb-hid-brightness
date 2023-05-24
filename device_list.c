#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb.h>

#include "device_list.h"

int get_hid_brightness_devices(libusb_context *ctx, DeviceInfo **device_list, int *device_count) {
    libusb_device **devs;
    libusb_device *dev;
    int i = 0;
    int count = 0;

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get device list\n");
        return -1;
    }

    *device_list = (DeviceInfo *)malloc(sizeof(DeviceInfo) * cnt);
    if (!(*device_list)) {
        fprintf(stderr, "Failed to allocate memory\n");
        libusb_free_device_list(devs, 1);
        return -1;
    }

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            continue;
        }

        libusb_device_handle *hdev = NULL;
        libusb_open(dev, &hdev);
        if (!hdev) {
            continue;
        }

        for (int j = 0; j < desc.bNumConfigurations; ++j) {
            struct libusb_config_descriptor *config;
            libusb_get_config_descriptor(dev, j, &config);

            for (int k = 0; k < config->bNumInterfaces; ++k) {
                const struct libusb_interface *interface = &config->interface[k];

                for (int l = 0; l < interface->num_altsetting; ++l) {
                    const struct libusb_interface_descriptor *interDesc = &interface->altsetting[l];

                    unsigned char str_desc[256];
                    libusb_get_string_descriptor_ascii(hdev, interDesc->iInterface, str_desc, sizeof(str_desc));

                    if (strcmp((char *)str_desc, "HID BRIGHTNESS") == 0) {
                        libusb_get_string_descriptor_ascii(hdev, desc.iProduct, (unsigned char*)(*device_list)[count].product, sizeof((*device_list)[count].product));
                        libusb_get_string_descriptor_ascii(hdev, desc.iManufacturer, (unsigned char*)(*device_list)[count].manufacturer, sizeof((*device_list)[count].manufacturer));
                        (*device_list)[count].handle = hdev;
                        count++;
                    }
                }
            }
            libusb_free_config_descriptor(config);
        }
    }

    libusb_free_device_list(devs, 1);

    *device_count = count;
    return 0;
}

#ifndef NDEBUG
void print_devices(DeviceInfo *device_list, int device_count) {
    for (int i = 0; i < device_count; ++i) {
        printf("Device Found: %s - %s\n", device_list[i].manufacturer, device_list[i].product);
    }
}
#endif
