#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libusb.h>

#include "brightness.h"
#include "device_list.h"

#define MAX_BRIGHTNESS 54000

int main(int argc, char **argv) {
    if(argc > 2) {
        fprintf(stderr, "Usage: %s [0-54000]\n", argv[0]);
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
        fprintf(stderr, "No HID BRIGHTNESS devices found.\n");
        libusb_exit(ctx);
        return EXIT_FAILURE;
    }

    if (argc == 2) {

        char *endptr;
        errno = 0; // Reset errno before calling strtol
        long brightness = strtol(argv[1], &endptr, 10);

        // Check if the input is not a valid integer
        if (endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Usage: %s [0-54000]\n", argv[0]);
            libusb_exit(ctx);
            return EXIT_FAILURE;
        }

        // Check for overflow or underflow
        if (errno == ERANGE || brightness < 0 || brightness > MAX_BRIGHTNESS) {
            fprintf(stderr, "Usage: %s [0-54000]\n", argv[0]);
            libusb_exit(ctx);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < device_count; ++i) {
            libusb_device_handle *hdev = device_list[i].handle;
            int iface = device_list[i].interface;
            int ret = 0;
#ifdef __linux__
            ret = libusb_set_auto_detach_kernel_driver(hdev, iface);
            if(ret != LIBUSB_SUCCESS) {
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
            printf("Setting brightness to %ld on %s - %s\n", brightness,
                    device_list[i].manufacturer, device_list[i].product);
            set_brightness(hdev, iface, (uint16_t)brightness);
            libusb_release_interface(hdev, iface);
            libusb_attach_kernel_driver(hdev, iface);
            libusb_close(hdev);
        }
    } else {
        for (int i = 0; i < device_count; ++i) {
            libusb_device_handle *hdev = device_list[i].handle;
            int iface = device_list[i].interface;
            int ret = 0;
#ifdef __linux__
            ret = libusb_set_auto_detach_kernel_driver(hdev, iface);
            if(ret != LIBUSB_SUCCESS) {
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
            uint16_t brightness = get_brightness(hdev, iface);
            printf("%s - %s: %u\n", device_list[i].manufacturer,
                    device_list[i].product, brightness);
            libusb_release_interface(hdev, iface);
            libusb_attach_kernel_driver(hdev, iface);
            libusb_close(hdev);
        }
    }

    libusb_exit(ctx);

    // free memory allocated for device list
    free(device_list);

    return EXIT_SUCCESS;
}
