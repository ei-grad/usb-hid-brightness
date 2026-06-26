#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb.h>

#include "device_list.h"

#define APPLE_VENDOR_ID       0x05ac
#define APPLE_STUDIO_DISPLAY  0x1114
#define APPLE_PRO_DISPLAY_XDR 0x9243

// Apple displays expose several indistinguishable "HID Relay" interfaces; the
// brightness control is the HID interface whose report descriptor declares the
// Monitor usage page (0x05 0x80).
#define HID_USAGE_PAGE_MONITOR_PREFIX_0 0x05
#define HID_USAGE_PAGE_MONITOR_PREFIX_1 0x80

static void read_device_strings(libusb_device_handle *hdev,
                                const struct libusb_device_descriptor *desc,
                                DeviceInfo *device, int idx) {
    int err = libusb_get_string_descriptor_ascii(
            hdev, desc->iProduct,
            (unsigned char *)device->product, sizeof(device->product));
    if (err < 0) {
        fprintf(stderr, "Failed to get product string for device %d: %s\n",
                idx, libusb_error_name(err));
        device->product[0] = '\0';
    }

    err = libusb_get_string_descriptor_ascii(
            hdev, desc->iManufacturer,
            (unsigned char *)device->manufacturer, sizeof(device->manufacturer));
    if (err < 0) {
        fprintf(stderr, "Failed to get manufacturer string for device %d: %s\n",
                idx, libusb_error_name(err));
        device->manufacturer[0] = '\0';
    }
}

// Returns the brightness control interface number of an Apple display, or -1.
static int find_apple_brightness_iface(libusb_device_handle *hdev,
                                       libusb_device *dev, int idx) {
    struct libusb_config_descriptor *config = NULL;
    if (libusb_get_active_config_descriptor(dev, &config) != 0) {
        fprintf(stderr, "Failed to get active config for device %d\n", idx);
        return -1;
    }

    int found = -1;
    for (int k = 0; k < config->bNumInterfaces && found < 0; ++k) {
        const struct libusb_interface *interface = &config->interface[k];
        for (int l = 0; l < interface->num_altsetting && found < 0; ++l) {
            const struct libusb_interface_descriptor *interDesc = &interface->altsetting[l];
            if (interDesc->bInterfaceClass != LIBUSB_CLASS_HID) {
                continue;
            }

            int iface = interDesc->bInterfaceNumber;
#ifdef __linux__
            libusb_set_auto_detach_kernel_driver(hdev, iface);
#endif
            if (libusb_claim_interface(hdev, iface) != 0) {
                continue;
            }

            // Reading the HID report descriptor requires the interface claimed.
            unsigned char report[256];
            int n = libusb_control_transfer(
                    hdev,
                    LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE,
                    LIBUSB_REQUEST_GET_DESCRIPTOR,
                    (uint16_t)(LIBUSB_DT_REPORT << 8), // report descriptor, index 0
                    iface, report, sizeof(report), 1000
            );
            for (int m = 0; n > 0 && m + 1 < n; ++m) {
                if (report[m] == HID_USAGE_PAGE_MONITOR_PREFIX_0 &&
                    report[m + 1] == HID_USAGE_PAGE_MONITOR_PREFIX_1) {
                    found = iface;
                    break;
                }
            }

            libusb_release_interface(hdev, iface);
#ifdef __linux__
            libusb_attach_kernel_driver(hdev, iface);
#endif
        }
    }

    libusb_free_config_descriptor(config);
    return found;
}

int get_hid_brightness_devices(libusb_context *ctx, DeviceInfo **device_list, int *device_count) {
    libusb_device **devs;
    libusb_device *dev;
    int i = 0;
    int err = 0;

    *device_count = 0;

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
            fprintf(stderr, "Failed to get device %d descriptor\n", i);
            continue;
        }

        libusb_device_handle *hdev = NULL;
        err = libusb_open(dev, &hdev);
        if (err != 0) {
#ifndef NDEBUG
            fprintf(stderr, "Failed to open device %d: %s\n", i, libusb_error_name(err));
#endif
            continue;
        }

        int matched = 0;

        if (desc.idVendor == APPLE_VENDOR_ID &&
            (desc.idProduct == APPLE_STUDIO_DISPLAY ||
             desc.idProduct == APPLE_PRO_DISPLAY_XDR)) {

            int iface = find_apple_brightness_iface(hdev, dev, i);
            if (iface >= 0) {
                DeviceInfo *device = &(*device_list)[(*device_count)++];
                device->handle = hdev;
                device->interface = iface;
                device->protocol = &BRIGHTNESS_PROTOCOL_APPLE;
                read_device_strings(hdev, &desc, device, i);
                matched = 1;
            }
        } else {
            for (int j = 0; j < desc.bNumConfigurations && !matched; ++j) {
                struct libusb_config_descriptor *config;
                err = libusb_get_config_descriptor(dev, j, &config);
                if (err != 0) {
                    fprintf(stderr, "Failed to get config descriptor for device %d configuration %d: %s\n",
                            i, j, libusb_error_name(err));
                    continue;
                }

                for (int k = 0; k < config->bNumInterfaces && !matched; ++k) {
                    const struct libusb_interface *interface = &config->interface[k];

                    for (int l = 0; l < interface->num_altsetting && !matched; ++l) {
                        const struct libusb_interface_descriptor *interDesc = &interface->altsetting[l];

                        unsigned char str_desc[256];
                        err = libusb_get_string_descriptor_ascii(hdev, interDesc->iInterface, str_desc, sizeof(str_desc));
                        if (err < 0) {
                            continue;
                        }

                        if (strcmp((char *)str_desc, "HID BRIGHTNESS") == 0) {
                            DeviceInfo *device = &(*device_list)[(*device_count)++];
                            device->handle = hdev;
                            device->interface = interDesc->bInterfaceNumber;
                            device->protocol = &BRIGHTNESS_PROTOCOL_LG;
                            read_device_strings(hdev, &desc, device, i);
                            matched = 1;
                        }
                    }
                }
                libusb_free_config_descriptor(config);
            }
        }

        if (!matched) {
            libusb_close(hdev);
        }
    }

    libusb_free_device_list(devs, 1);

    return 0;
}

#ifndef NDEBUG
void print_devices(DeviceInfo *device_list, int device_count) {
    for (int i = 0; i < device_count; ++i) {
        printf("Device Found: %s - %s\n", device_list[i].manufacturer, device_list[i].product);
    }
}
#endif
