cmake_minimum_required(VERSION 3.0)
project(main)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_executable(
    usb-hid-brightness
    main.c
    device_list.c
    #find_by_interface.c
    #open_device_simple.c
    #find_by_id.c
)
target_include_directories(usb-hid-brightness PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(usb-hid-brightness ${LIBUSB_LIBRARIES})
install(TARGETS usb-hid-brightness DESTINATION bin)
