# USB HID Brightness Controller

This is a simple utility to control the brightness of USB and Thunderbolt monitors providing HID BRIGHTNESS interface over USB like LG UltraFine 5K. It is written in C using the libusb library.

## Prerequisites

- A modern C compiler.
- CMake version 3.10 or later.
- libusb development files.

You can install libusb and cmake on Ubuntu or other Debian-based Linux distributions using:

```bash
sudo apt-get install libusb-1.0-0-dev cmake
```

On Fedora, CentOS, or RHEL, you can use:

```bash
sudo dnf install libusb-devel cmake
```

## Building

First, clone the repository:

```bash
git clone https://github.com/ei-grad/usb-hid-brightness.git
cd usb-hid-brightness
```

Create a build directory:

```
mkdir build
cd build
```

Next, run `cmake` to configure the build:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Then build the project:

```bash
cmake --build .
```

This will create the `usb-hid-brightness` executable in the build directory.

## Installing

To install `usb-hid-brightness` system-wide, you will typically need superuser permissions. From the build directory, run:

```bash
sudo cmake --install .
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

To use `usb-hid-brightness` without needing superuser permissions every time, you can set up a udev rule to grant your user permissions to access the device. Here's an example of such a rule for LG UltraFine Displays:

Create a file in the `/etc/udev/rules.d/` directory to store your udev rules. You can name it something like `99-usb-hid-brightness.rules`.

In this file, add rules like the following:

```bash
# LG UltraFine 24MD4KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a63", MODE="0666"
# LG UltraFine 27MD5KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a70", MODE="0666"
# LG UltraFine 27MD5KA
SUBSYSTEM=="usb", ATTRS{idVendor}=="43e", ATTRS{idProduct}=="9a40", MODE="0666"
```

In these examples, `"43e"` is the vendor ID for LG, and `"9a63"`, `"9a70"`, and `"9a40"` are the product IDs for specific LG UltraFine Display models. Replace or extend these values with your device's vendor and product IDs. You can find these IDs using the `lsusb` command in your terminal (look for devices named like "LG UltraFine Display Controls").

The `MODE="0666"` part of the rule gives all users read and write permissions for the device.

After saving the file, reload the udev rules with the following command:

```bash
sudo udevadm control --reload-rules
```

And trigger the new rules:

```bash
sudo udevadm trigger
```

After you unplug and replug the device, you should now be able to use `usb-hid-brightness` without superuser permissions.

Please note, this rule allows all users on the system to read and write to the device, which may not be suitable in a multi-user environment. You might want to create a dedicated group for users who are allowed to control the brightness, and use `GROUP="group-name", MODE="0660"` instead of `MODE="0666"` for a more secure setup.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an Issue on the GitHub repository.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
