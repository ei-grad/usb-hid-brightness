#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#include <libusb.h>

#include "brightness.h"
#include "device_list.h"

// Upper bound accepted on the command line; the per-device protocol enforces the
// real range (e.g. 0-54000 for LG, 400-60000 for Apple).
#define MAX_INPUT_BRIGHTNESS 60000

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [0-%d]\n", argv[0], MAX_INPUT_BRIGHTNESS);
        return EXIT_FAILURE;
    }

    libusb_context *ctx = NULL;

    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "Unable to initialize libusb.\n");
        return EXIT_FAILURE;
    }

    DeviceInfo *device_list = NULL;
    int device_count = 0;
    if (get_hid_brightness_devices(ctx, &device_list, &device_count) != 0) {
        fprintf(stderr, "Failed to get device list.\n");
        libusb_exit(ctx);
        return EXIT_FAILURE;
    }

#ifndef NDEBUG
    print_devices(device_list, device_count);
#endif

    if (device_count == 0) {
        fprintf(stderr, "No HID brightness devices found.\n");
        free(device_list);
        libusb_exit(ctx);
        return EXIT_FAILURE;
    }

    int set_mode = (argc == 2);
    long brightness = 0;

    if (set_mode) {
        char *endptr;
        errno = 0; // Reset errno before calling strtol
        brightness = strtol(argv[1], &endptr, 10);

        if (endptr == argv[1] || *endptr != '\0' ||
            errno == ERANGE || brightness < 0 || brightness > MAX_INPUT_BRIGHTNESS) {
            fprintf(stderr, "Usage: %s [0-%d]\n", argv[0], MAX_INPUT_BRIGHTNESS);
            free(device_list);
            libusb_exit(ctx);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < device_count; ++i) {
        libusb_device_handle *hdev = device_list[i].handle;
        int iface = device_list[i].interface;
        const BrightnessProtocol *proto = device_list[i].protocol;
        int ret;

#ifdef __linux__
        ret = libusb_set_auto_detach_kernel_driver(hdev, iface);
        if (ret != LIBUSB_SUCCESS) {
            fprintf(stderr, "Failed to set auto detach kernel driver: %s\n",
                    libusb_error_name(ret));
            libusb_close(hdev);
            continue;
        }
#endif
        ret = libusb_claim_interface(hdev, iface);
        if (ret != LIBUSB_SUCCESS) {
            fprintf(stderr, "failed to claim interface: %s\n",
                    libusb_error_name(ret));
            libusb_close(hdev);
            continue;
        }

        if (set_mode) {
            if ((uint32_t)brightness < proto->min || (uint32_t)brightness > proto->max) {
                fprintf(stderr, "%s - %s: %ld out of range [%u-%u], skipping\n",
                        device_list[i].manufacturer, device_list[i].product,
                        brightness, proto->min, proto->max);
            } else {
                printf("Setting brightness to %ld on %s - %s\n", brightness,
                        device_list[i].manufacturer, device_list[i].product);
                set_brightness(hdev, iface, proto, (uint32_t)brightness);
            }
        } else {
            uint32_t value = get_brightness(hdev, iface, proto);
            printf("%s - %s: %u\n", device_list[i].manufacturer,
                    device_list[i].product, value);
        }

        libusb_release_interface(hdev, iface);
#ifdef __linux__
        libusb_attach_kernel_driver(hdev, iface);
#endif
        libusb_close(hdev);
    }

    libusb_exit(ctx);

    // free memory allocated for device list
    free(device_list);

    return EXIT_SUCCESS;
}
