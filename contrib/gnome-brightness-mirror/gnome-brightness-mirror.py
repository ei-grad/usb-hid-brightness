#!/usr/bin/env python3
"""Mirror the GNOME screen-brightness level onto an external USB HID display.

GNOME publishes the internal screen brightness as a 0-100 percentage on the
session bus (org.gnome.SettingsDaemon.Power.Screen.Brightness) and updates it
when the multimedia brightness keys are pressed. This daemon listens for those
changes and proportionally drives an external display via usb-hid-brightness,
so the brightness keys affect the external monitor as well as the built-in one.

Configuration (environment variables):
  MIRROR_TOOL         path to usb-hid-brightness (default /usr/local/bin/usb-hid-brightness)
  MIRROR_MIN          display value at 0%    (default 400;   use 0 for LG UltraFine)
  MIRROR_MAX          display value at 100%  (default 60000; use 54000 for LG UltraFine)
  MIRROR_DEBOUNCE_MS  coalescing delay in ms (default 80)
"""
import os
import subprocess

import gi  # noqa: F401  (required before gi.repository import)
from gi.repository import Gio, GLib

TOOL = os.environ.get("MIRROR_TOOL", "/usr/local/bin/usb-hid-brightness")
DISPLAY_MIN = int(os.environ.get("MIRROR_MIN", "400"))
DISPLAY_MAX = int(os.environ.get("MIRROR_MAX", "60000"))
DEBOUNCE_MS = int(os.environ.get("MIRROR_DEBOUNCE_MS", "80"))

BUS_NAME = "org.gnome.SettingsDaemon.Power"
OBJ_PATH = "/org/gnome/SettingsDaemon/Power"
PROPS_IFACE = "org.freedesktop.DBus.Properties"
SCREEN_IFACE = "org.gnome.SettingsDaemon.Power.Screen"


class Mirror:
    def __init__(self):
        self.bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
        self._timer = 0
        self._pending = None
        self._last_applied = None

    def pct_to_value(self, pct):
        pct = max(0, min(100, pct))
        return round(DISPLAY_MIN + pct * (DISPLAY_MAX - DISPLAY_MIN) / 100)

    def apply(self):
        self._timer = 0
        if self._pending is None:
            return False
        value = self.pct_to_value(self._pending)
        if value != self._last_applied:
            self._last_applied = value
            try:
                subprocess.run([TOOL, str(value)],
                               stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL,
                               timeout=5)
            except (OSError, subprocess.SubprocessError):
                # Display may be unplugged or busy; ignore and retry on the
                # next brightness change.
                pass
        return False  # one-shot timer

    def schedule(self, pct):
        self._pending = pct
        if self._timer == 0:
            self._timer = GLib.timeout_add(DEBOUNCE_MS, self.apply)

    def read_brightness(self):
        try:
            reply = self.bus.call_sync(
                BUS_NAME, OBJ_PATH, PROPS_IFACE, "Get",
                GLib.Variant("(ss)", (SCREEN_IFACE, "Brightness")),
                GLib.VariantType("(v)"), Gio.DBusCallFlags.NONE, -1, None)
            return reply.unpack()[0]
        except GLib.Error:
            return None

    def on_props_changed(self, conn, sender, path, iface, signal, params):
        iface_name, changed, _invalidated = params.unpack()
        if iface_name != SCREEN_IFACE:
            return
        pct = changed.get("Brightness")
        if isinstance(pct, int) and pct >= 0:
            self.schedule(pct)

    def run(self):
        self.bus.signal_subscribe(
            BUS_NAME, PROPS_IFACE, "PropertiesChanged", OBJ_PATH, None,
            Gio.DBusSignalFlags.NONE, self.on_props_changed)
        pct = self.read_brightness()
        if isinstance(pct, int) and pct >= 0:
            self.schedule(pct)
        GLib.MainLoop().run()


if __name__ == "__main__":
    Mirror().run()
