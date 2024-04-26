# USB HID Brightness Controller

This utility enables brightness control for USB and Thunderbolt monitors that support the HID BRIGHTNESS interface, such as the LG UltraFine 5K. It is written in C and utilizes the libusb library.

## Prerequisites

To build and use this utility, you will need the following:
* A C compiler (GCC or Clang)
* CMake, make and pkg-config
* libusb-1.0 development files
* libc-dev

On Ubuntu or Debian, you can install the required packages with:

```bash
sudo apt install -y --no-install-recommends libusb-1.0-0-dev cmake make gcc pkg-config libc-dev
```

On Fedora, CentOS, or RHEL, you can use:

```bash
sudo dnf install libusb1-devel cmake make gcc pkgconfig glibc-devel
```

## Building

```bash
git clone https://github.com/ei-grad/usb-hid-brightness.git
cd usb-hid-brightness
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
```

This will build the `usb-hid-brightness` executable in the `build` directory.

## Installing

To install `usb-hid-brightness` system-wide, you will typically need superuser permissions. You can use the following command to install the utility:

```bash
sudo cmake --install ./build
```

This will install the `usb-hid-brightness` executable to your system's binary directory (usually `/usr/local/bin`), making it available system-wide.

Remember, to set up a udev rule for the device to allow non-superuser access, follow the instructions in the [Setting up udev rules](#setting-up-udev-rules) section.

## Usage

The utility can be used to either set or get the brightness of your HID devices.

To set the brightness:

```bash
usb-hid-brightness 27000
```

This sets the brightness of all found HID brightness devices to 27000 (out of a maximum of 54000).

To get the current brightness:

```bash
usb-hid-brightness
```

This will display the current brightness of all found HID brightness devices.

## Setting up udev rules

To use usb-hid-brightness without superuser privileges, you can configure a udev rule that grants your user account access to the device. Here's an example of such a rules for LG UltraFine Displays:

```bash
# LG UltraFine 24MD4KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a63", MODE="0666"
# LG UltraFine 27MD5KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a70", MODE="0666"
# LG UltraFine 27MD5KA
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a40", MODE="0666"
```

Put it into the `/etc/udev/rules.d/` directory, you can name it like `99-usb-hid-brightness.rules`.

In these examples, `"43e"` is the vendor ID for LG, and `"9a63"`, `"9a70"`, and `"9a40"` are the product IDs for specific LG UltraFine Display models. Replace or extend these values with your device's vendor and product IDs. You can find these IDs using the `lsusb` command in your terminal (look for devices named like "LG UltraFine Display Controls"). For other devices, you may need to adjust the vendor and product IDs accordingly.

The `MODE="0666"` part of the rule gives all users read and write permissions for the device.

After saving the file, reload the udev rules with the following command:

```bash
sudo udevadm control --reload-rules
```

And trigger the new rules:

```bash
sudo udevadm trigger
```

After you unplug and replug the device, you should be able to use `usb-hid-brightness` from your user account without superuser privileges.

Please note, this rule allows all users on the system to read and write to the device, which may not be suitable in a multi-user environment. You might want to create a dedicated group for users who are allowed to control the brightness, and use `GROUP="group-name", MODE="0660"` instead of `MODE="0666"` for a more secure setup.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an Issue on the GitHub repository.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
