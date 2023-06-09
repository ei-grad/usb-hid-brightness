#include <libusb.h>

typedef struct DeviceInfo {
    char product[256];
    char manufacturer[256];
    libusb_device_handle *handle;
    int interface;
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
