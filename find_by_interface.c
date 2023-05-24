#include <stdio.h>
#include <string.h>
#include <libusb.h>

#include "open_device.h"

libusb_device_handle* open_device(libusb_context *ctx) {
    libusb_device **devs;
    libusb_device *dev;
    int i = 0;

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get device list\n");
        return NULL;
    }

    libusb_device_handle *hdev = NULL;

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            continue;
        }

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
                        libusb_free_device_list(devs, 1);
                        return hdev;
                    }
                }
            }
            libusb_free_config_descriptor(config);
        }
        libusb_close(hdev);
        hdev = NULL;
    }

    libusb_free_device_list(devs, 1);
    fprintf(stderr, "Failed to find device with interface 'HID BRIGHTNESS'\n");

    return NULL;
}
