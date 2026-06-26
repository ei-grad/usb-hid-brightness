# USB HID Brightness Controller

This utility allows you to control the brightness of USB and Thunderbolt monitors over USB HID. It supports two kinds of devices:

* Displays exposing a `HID BRIGHTNESS` interface, such as the LG UltraFine.
* Apple displays (Studio Display and Pro Display XDR), detected by their USB vendor/product id.

Written in C and powered by the libusb library, it runs on Linux. Windows support is implemented but **has not been tested yet** (see [Windows](#windows)). While it may also work on macOS, these monitors usually support native brightness control on that platform.

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

### Windows build (MSYS2 MinGW-w64)

Install MSYS2 from https://www.msys2.org/, then run the following commands in the MSYS2 terminal:

```
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-pkg-config mingw-w64-x86_64-libusb
# to build a static executable (which will not require libusb-1.0.dll), use the following command:
cmake -B build -DCMAKE_EXE_LINKER_FLAGS="-static" -DCMAKE_BUILD_TYPE=Release
# or, if you wish just to build a usual dynamic executable:
#cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Installing

To install `usb-hid-brightness` system-wide, you will typically need superuser permissions. You can use the following command to install the binary built in the previous step:

```bash
sudo cmake --install ./build
```

This will install the `usb-hid-brightness` executable to your system's binary directory (usually `/usr/local/bin`), making it available system-wide.

Remember, to set up a udev rule for the device to allow non-superuser access, follow the instructions in the [Setting up udev rules](#setting-up-udev-rules) section.

There is also:
- A pre-built binary available in the [Releases](https://github.com/ei-grad/usb-hid-brightness/releases/latest) section of the GitHub repository.
- AUR package available for Arch Linux users: [usb-hid-brightness](https://aur.archlinux.org/packages/usb-hid-brightness/).

### Windows

> **Note:** Windows support has not been tested yet. The steps below are expected to be required but are unverified — feedback is welcome.

On Windows, libusb can only access a device bound to the WinUSB (or libusbK) driver. The display's brightness interface is normally claimed by the built-in Windows HID driver, so you will likely need to assign WinUSB to that interface using [Zadig](https://zadig.akeo.ie/) before the utility can detect the display.

Once the driver is in place, I'd recommend to make a couple of .bat files on your desktop to run the utility with the desired brightness level. Or a .bat file which prompt you to enter the desired brightness level before running the utility. Here's an example:

```bat
@echo off
set /p brightness=Enter brightness level (0-54000):
path/to/usb-hid-brightness.exe %brightness%
```

(Replace `path/to/usb-hid-brightness.exe` with the actual path to the executable.)

This will prompt you to enter the desired brightness level and then run the utility with the entered value.

## Usage

The utility can be used to either set or get the brightness of your HID devices.

To set the brightness:

```bash
usb-hid-brightness 27000
```

This sets the brightness of all found HID brightness devices to 27000. The accepted range depends on the device: LG UltraFine accepts `0`–`54000`, while Apple displays accept `400`–`60000`. Values outside a device's range are skipped for that device.

To get the current brightness:

```bash
usb-hid-brightness
```

This will display the current brightness of all found HID brightness devices.

## Setting up udev rules

To use usb-hid-brightness without superuser privileges, you can configure a udev rule that grants your user account access to the device. Here's an example of such rules for LG UltraFine and Apple displays:

```bash
# LG UltraFine 24MD4KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="043e", ATTRS{idProduct}=="9a63", MODE="0666"
# LG UltraFine 27MD5KL
SUBSYSTEM=="usb", ATTRS{idVendor}=="043e", ATTRS{idProduct}=="9a70", MODE="0666"
# LG UltraFine 27MD5KA
SUBSYSTEM=="usb", ATTRS{idVendor}=="043e", ATTRS{idProduct}=="9a40", MODE="0666"
# Apple Studio Display
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="1114", MODE="0666"
# Apple Pro Display XDR
SUBSYSTEM=="usb", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="9243", MODE="0666"
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

Now you should be able to use `usb-hid-brightness` from your user account without superuser privileges.

Please note, this rule allows all users on the system to read and write to the device, which may not be suitable in a multi-user environment. You might want to create a dedicated group for users who are allowed to control the brightness, and use `GROUP="group-name", MODE="0660"` instead of `MODE="0666"` for a more secure setup.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an Issue on the GitHub repository.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
