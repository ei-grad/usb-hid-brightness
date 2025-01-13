cmake_minimum_required(VERSION 3.10)
project(usb-hid-brightness C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_executable(
    usb-hid-brightness
    main.c
    device_list.c
    brightness.c
)

target_include_directories(usb-hid-brightness PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(usb-hid-brightness ${LIBUSB_LIBRARIES})

install(TARGETS usb-hid-brightness DESTINATION bin)
