#include <libusb.h>

#define VENDOR_ID 0x43e
#define PRODUCT_ID 0x9a70

libusb_device_handle *open_device(libusb_context *ctx) {
    libusb_device_handle *dev_handle;
    if(libusb_init(NULL) < 0) {
        return NULL;
    }
    dev_handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    return dev_handle;
}
