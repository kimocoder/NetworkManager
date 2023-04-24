#!/usr/bin/env python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2015 Red Hat, Inc.
#

#
# This example configures a Bond from ethernet devices and activates it
#
# NetworkManager D-Bus API:
# https://developer.gnome.org/NetworkManager/stable/spec.html
#

import dbus, sys, uuid
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GObject

DBusGMainLoop(set_as_default=True)

def add_connection(con):
    bus = dbus.SystemBus()
    proxy = bus.get_object("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings")
    settings = dbus.Interface(proxy, "org.freedesktop.NetworkManager.Settings")
    return settings.AddConnection(con)

def create_bond(bond_name):
    bond_opts = dbus.Dictionary({'mode': '4'})
    s_bond = dbus.Dictionary({'options': bond_opts})
    s_con = dbus.Dictionary({
        'type': 'bond',
        'uuid': str(uuid.uuid4()),
        'id': bond_name,
        'interface-name': bond_name,
        'autoconnect': False,
        'autoconnect-slaves': 1})
    s_ip4 = dbus.Dictionary({'method': 'auto'})
    s_ip6 = dbus.Dictionary({'method': 'ignore'})

    con = dbus.Dictionary({
        'bond': s_bond,
        'connection': s_con,
        'ipv4': s_ip4,
        'ipv6': s_ip6})
    print(f"Creating bond connection: {bond_name}")
    return add_connection(con)

def create_slave(device, master):
    slave_name = f'bond-{master}-slave-{device}'
    s_wired = dbus.Dictionary({'duplex': 'full'})
    s_con = dbus.Dictionary({
        'type': '802-3-ethernet',
        'uuid': str(uuid.uuid4()),
        'id': slave_name,
        'interface-name': device,
        'autoconnect': False,
        'master': master,
        'slave-type': 'bond'})

    con = dbus.Dictionary({
        '802-3-ethernet': s_wired,
        'connection': s_con})
    print(f"Creating slave connection: {slave_name}")
    add_connection(con)

def usage():
    print(f"Usage: {sys.argv[0]} <bond_name> <ifname1> ...")
    sys.exit(0)


if len(sys.argv) < 3:
    usage()

# Create bond master and slave connections
bond_name = sys.argv[1]
bond_path = create_bond(bond_name)
for ifname in sys.argv[2:]:
    create_slave(ifname, bond_name)

# Activate the bond
bus = dbus.SystemBus()
proxy = bus.get_object("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager")
manager = dbus.Interface(proxy, "org.freedesktop.NetworkManager")
ac = manager.ActivateConnection(bond_path, "/", "/")
print(f"Activating bond: {bond_name} ({ac})")

# Monitor the active bond connection
loop = GObject.MainLoop()
def properties_changed(props):
    if 'State' in props:
        if props['State'] == 2:
            print("Successfully connected")
            loop.quit()
        if props['State'] in [3, 4]:
            print("Bond activation failed")
            loop.quit()

obj = bus.get_object("org.freedesktop.NetworkManager", ac)
iface = dbus.Interface(obj, "org.freedesktop.NetworkManager.Connection.Active")
iface.connect_to_signal("PropertiesChanged", properties_changed)

loop.run()
