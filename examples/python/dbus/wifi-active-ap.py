#!/usr/bin/env python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2015 Red Hat, Inc.
#

#
# This example starts or stops a wifi hotspot
#
# Configuration settings are described at
# https://developer.gnome.org/NetworkManager/1.0/ref-settings.html
#

import dbus, sys, time

bus = dbus.SystemBus()
service_name = "org.freedesktop.NetworkManager"
proxy = bus.get_object(service_name, "/org/freedesktop/NetworkManager/Settings")
settings = dbus.Interface(proxy, "org.freedesktop.NetworkManager.Settings")

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <ifname>")
    sys.exit(0)

# Get the device object path based on interface name
iface = sys.argv[1]
proxy = bus.get_object(service_name, "/org/freedesktop/NetworkManager")
nm = dbus.Interface(proxy, "org.freedesktop.NetworkManager")
devpath = nm.GetDeviceByIpIface(iface)

# Get a proxy to the wifi device and get the active access point's object path
proxy = bus.get_object(service_name, devpath)
props = dbus.Interface(proxy, "org.freedesktop.DBus.Properties")
active_ap_path = props.Get("org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint")
if active_ap_path == "/":
    print(f"{sys.argv[1]} is not currently associated")
    sys.exit(0)

# Get the active access point's SSID and BSSID
ap_proxy = bus.get_object(service_name, active_ap_path)
ap_props = dbus.Interface(ap_proxy, "org.freedesktop.DBus.Properties")
raw_ssid = ap_props.Get("org.freedesktop.NetworkManager.AccessPoint", "Ssid")
bssid = ap_props.Get("org.freedesktop.NetworkManager.AccessPoint", "HwAddress")

# Convert the SSID from a byte array to a string, assuming ASCII encoding
ssid = ""
for c in raw_ssid:
    ssid = ssid + chr(c)

print(f"{sys.argv[1]} is associated to '{ssid}' ({bssid})")

sys.exit(0)

