#include <stdio.h>
#include <stdlib.h>

#include <libusb.h>

#include "open_device.h"
#include "device_list.h"

#define MAX_BRIGHTNESS 54000


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

void close_device(libusb_device_handle *hdev) {
    libusb_close(hdev);
    libusb_exit(NULL);
}

int main(int argc, char **argv) {
    if(argc > 2) {
        printf("Usage: %s [0-54000]\n", argv[0]);
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
        int brightness = atoi(argv[1]);
        if (brightness < 0 || brightness > MAX_BRIGHTNESS) {
            fprintf(stderr, "Brightness should be a number between 0 and %d\n", MAX_BRIGHTNESS);
            libusb_exit(ctx);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < device_count; ++i) {
            libusb_device_handle *hdev = device_list[i].handle;
            set_brightness(hdev, (uint16_t)brightness);
            close_device(hdev);
        }
    } else {
        for (int i = 0; i < device_count; ++i) {
            libusb_device_handle *hdev = device_list[i].handle;
            uint16_t brightness = get_brightness(hdev);
            printf("%s - %s: %u\n", device_list[i].manufacturer, device_list[i].product, brightness);
            close_device(hdev);
        }
    }

    libusb_exit(ctx);

    // free memory allocated for device list
    free(device_list);

    return EXIT_SUCCESS;
}
