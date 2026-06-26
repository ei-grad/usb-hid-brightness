# GNOME brightness-key mirror

Make the multimedia brightness keys control an external USB HID display (e.g.
Apple Studio Display, LG UltraFine) in addition to the built-in laptop screen.

On GNOME (Wayland or X11) the brightness keys are owned by gnome-settings-daemon
and only drive the internal backlight. Rebinding them is unreliable, so instead
this small daemon *mirrors* the change: it watches the GNOME brightness
percentage on D-Bus and proportionally sets the external display via
`usb-hid-brightness`.

## Requirements

- A GNOME session (uses `org.gnome.SettingsDaemon.Power`).
- `usb-hid-brightness` installed and on `PATH` (e.g. `sudo cmake --install build`
  installs it to `/usr/local/bin`).
- A udev rule granting your user access to the display — see the main
  [README](../../README.md#setting-up-udev-rules).
- System PyGObject (`python3-gi`), which ships with GNOME by default:
  - Debian/Ubuntu: `sudo apt install python3-gi`
  - Fedora: `sudo dnf install python3-gobject`

## Install

```bash
install -Dm755 gnome-brightness-mirror.py ~/.local/bin/gnome-brightness-mirror.py
install -Dm644 gnome-brightness-mirror.service ~/.config/systemd/user/gnome-brightness-mirror.service
systemctl --user daemon-reload
systemctl --user enable --now gnome-brightness-mirror.service
```

Press the brightness keys: the external display should follow the built-in one.

## Tuning

The display value is mapped linearly from the GNOME percentage between
`MIRROR_MIN` (at 0%) and `MIRROR_MAX` (at 100%). The defaults suit the Apple
Studio Display (`400`–`60000`); for an LG UltraFine use `0`–`54000`. Raise
`MIRROR_MIN` if the external display gets uncomfortably dim at low levels.

Override with a drop-in, then restart:

```bash
systemctl --user edit gnome-brightness-mirror.service
# add under [Service], e.g.:
#   [Service]
#   Environment=MIRROR_MIN=3000
systemctl --user restart gnome-brightness-mirror.service
```

Other variables: `MIRROR_MAX`, `MIRROR_DEBOUNCE_MS` (default 80), `MIRROR_TOOL`
(path to `usb-hid-brightness`).

## How it works / limitations

- Subscribes to `PropertiesChanged` for the `Brightness` property of
  `org.gnome.SettingsDaemon.Power.Screen` and applies the latest value after a
  short debounce.
- The external display *follows* the internal brightness percentage; the two
  cannot be controlled independently.
- GNOME-specific. Other desktops expose brightness control differently.

## Uninstall

```bash
systemctl --user disable --now gnome-brightness-mirror.service
rm ~/.local/bin/gnome-brightness-mirror.py \
   ~/.config/systemd/user/gnome-brightness-mirror.service
systemctl --user daemon-reload
```
