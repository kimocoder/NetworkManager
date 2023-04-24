#!/usr/bin/env python
# coding=utf-8
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2013 Red Hat, Inc.
#

import locale
import gi
gi.require_version('NM', '1.0')
from gi.repository import NM

#
# This example lists Wi-Fi access points NetworkManager scanned on Wi-Fi devices.
# It calls libnm functions using GObject introspection.
#
# Note the second line of the file: coding=utf-8
# It is necessary because we use unicode characters and python would produce
# an error without it: http://www.python.org/dev/peps/pep-0263/
#

def clamp(value, minvalue, maxvalue):
    return max(minvalue, min(value, maxvalue))

def ssid_to_utf8(ap):
    ssid = ap.get_ssid()
    return NM.utils_ssid_to_utf8(ap.get_ssid().get_data()) if ssid else ""

def print_device_info(device):
    active_ap = dev.get_active_access_point()
    ssid = ssid_to_utf8(active_ap) if active_ap is not None else None
    info = f"Device: {dev.get_iface()} | Driver: {dev.get_driver()} | Active AP: {ssid}"
    print(info)
    print('=' * len(info))

def mode_to_string(mode):
    if mode == getattr(NM, '80211Mode').INFRA:
        return "INFRA"
    if mode == getattr(NM, '80211Mode').ADHOC:
        return "ADHOC"
    return "AP" if mode == getattr(NM, '80211Mode').AP else "UNKNOWN"

def flags_to_string(flags):
    return "PRIVACY" if flags & getattr(NM, '80211ApFlags').PRIVACY else "NONE"

def security_flags_to_string(flags):
    NM_AP_FLAGS = getattr(NM, '80211ApSecurityFlags')
    str = ""
    if flags & NM_AP_FLAGS.PAIR_WEP40:
        str += " PAIR_WEP40"
    if flags & NM_AP_FLAGS.PAIR_WEP104:
        str += " PAIR_WEP104"
    if flags & NM_AP_FLAGS.PAIR_TKIP:
        str += " PAIR_TKIP"
    if flags & NM_AP_FLAGS.PAIR_CCMP:
        str += " PAIR_CCMP"
    if flags & NM_AP_FLAGS.GROUP_WEP40:
        str += " GROUP_WEP40"
    if flags & NM_AP_FLAGS.GROUP_WEP104:
        str += " GROUP_WEP104"
    if flags & NM_AP_FLAGS.GROUP_TKIP:
        str += " GROUP_TKIP"
    if flags & NM_AP_FLAGS.GROUP_CCMP:
        str += " GROUP_CCMP"
    if flags & NM_AP_FLAGS.KEY_MGMT_PSK:
        str += " KEY_MGMT_PSK"
    if flags & NM_AP_FLAGS.KEY_MGMT_802_1X:
        str += " KEY_MGMT_802_1X"
    return str.lstrip() if str else "NONE"

def flags_to_security(flags, wpa_flags, rsn_flags):
    str = ""
    if ((flags & getattr(NM, '80211ApFlags').PRIVACY) and
        (wpa_flags == 0) and (rsn_flags == 0)):
        str += " WEP"
    if wpa_flags != 0:
        str += " WPA1"
    if rsn_flags != 0:
        str += " WPA2"
    if ((wpa_flags & getattr(NM, '80211ApSecurityFlags').KEY_MGMT_802_1X) or
        (rsn_flags & getattr(NM, '80211ApSecurityFlags').KEY_MGMT_802_1X)):
        str += " 802.1X"
    return str.lstrip()

def print_ap_info(ap):
    strength = ap.get_strength()
    frequency = ap.get_frequency()
    flags = ap.get_flags()
    wpa_flags = ap.get_wpa_flags()
    rsn_flags = ap.get_rsn_flags()
    print(f"SSID:      {ssid_to_utf8(ap)}")
    print(f"BSSID:     {ap.get_bssid()}")
    print(f"Frequency: {frequency}")
    print(f"Channel:   {NM.utils_wifi_freq_to_channel(frequency)}")
    print(f"Mode:      {mode_to_string(ap.get_mode())}")
    print(f"Flags:     {flags_to_string(flags)}")
    print(f"WPA flags: {security_flags_to_string(wpa_flags)}")
    print(f"RSN flags: {security_flags_to_string(rsn_flags)}")
    print(f"Security:  {flags_to_security(flags, wpa_flags, rsn_flags)}")
    print("Strength:  %s %s%%" % (NM.utils_wifi_strength_bars(strength), strength))
    print

if __name__ == "__main__":
    # Python apparently doesn't call setlocale() on its own? We have to call this or else
    # NM.utils_wifi_strength_bars() will think the locale is ASCII-only, and return the
    # fallback characters rather than the unicode bars
    locale.setlocale(locale.LC_ALL, '')

    nmc = NM.Client.new(None)
    devs = nmc.get_devices()

    for dev in devs:
        if dev.get_device_type() == NM.DeviceType.WIFI:
            print_device_info(dev)
            for ap in dev.get_access_points():
                print_ap_info(ap)

