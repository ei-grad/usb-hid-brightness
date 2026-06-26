#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <stdint.h>

#include <libusb.h>

// Describes how a particular device encodes its HID brightness feature report.
typedef struct BrightnessProtocol {
    uint8_t report_id;    // HID report id, placed in the low byte of wValue
    uint8_t report_len;   // number of bytes exchanged in the feature report
    uint8_t value_offset; // offset of the little-endian brightness value in the report
    uint8_t value_size;   // width of the brightness value in bytes
    uint32_t min;         // minimum brightness accepted by the device
    uint32_t max;         // maximum brightness accepted by the device
} BrightnessProtocol;

// LG UltraFine and other displays exposing a "HID BRIGHTNESS" interface.
extern const BrightnessProtocol BRIGHTNESS_PROTOCOL_LG;
// Apple Studio Display / Pro Display XDR.
extern const BrightnessProtocol BRIGHTNESS_PROTOCOL_APPLE;

uint32_t get_brightness(libusb_device_handle *hdev, int iface,
                        const BrightnessProtocol *proto);
void set_brightness(libusb_device_handle *hdev, int iface,
                    const BrightnessProtocol *proto, uint32_t val);

#endif // BRIGHTNESS_H
