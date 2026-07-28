#pragma once

// Copy-source template v1.0: a mod setting that drives a WINDOWS setting.
//
// This is a different animal from every other setting in the family and it
// deserves its own component. A normal setting changes something the mod owns:
// a XAML property it leased, a layout it computed. This one reaches out and
// changes shared machine state that Settings, Explorer, and other apps also
// read and write. Three consequences follow, and getting any of them wrong
// produces a toggle that looks connected and is not:
//
//   1. YOU MUST WRITE THE VALUE WINDOWS ACTUALLY READS. Registry names in this
//      area are a graveyard of near-misses. The taskbar battery percentage is
//      `IsBatteryPercentageEnabled`; `TaskbarBatteryPercent` sits in the same
//      key, looks exactly as plausible, and Windows 11's Settings app does not
//      read it. A mod that wrote the second one for months looked correct in
//      its own log the entire time — it reported writing, and it did write,
//      just to nothing. VERIFY against the live registry with the OS UI open,
//      toggling it there and watching which value moves. Never infer the name.
//
//   2. WRITE EVERY ALIAS YOU HAVE CONFIRMED. Windows builds disagree about
//      which value is authoritative, and a mod cannot detect the build's
//      opinion cheaply. Write all known aliases and restore all of them.
//
//   3. RESTORE EXACTLY, AND ONLY WHAT YOU CHANGED. Snapshot before the first
//      write, restore on unload. A value the mod created where none existed
//      must be DELETED on restore, not set to zero — leaving a zero behind is
//      a mod editing the user's machine permanently.
//
// The bridge deliberately does not re-assert its value. If the user opens
// Settings and changes it, they win; a mod that fights the OS control panel is
// a bug, not a feature. Re-assert only on the mod's own settings change.

#include <string>
#include <vector>

#include <windows.h>

namespace windhawk_mod_templates::os_setting_bridge {

// One registry value the OS reads for a given feature.
struct ValueName {
    wchar_t const* name;
    bool existed = false;   // filled in by Capture
    DWORD original = 0;     // filled in by Capture
};

// A DWORD-valued feature under one key, possibly spelled several ways.
struct DwordSetting {
    HKEY root = HKEY_CURRENT_USER;
    wchar_t const* subKey = nullptr;
    std::vector<ValueName> values;
    bool captured = false;

    // Broadcast target. Explorer-owned settings want the key path as lParam of
    // WM_SETTINGCHANGE; leave null to skip the broadcast.
    wchar_t const* broadcast = nullptr;
};

inline bool ReadDword(HKEY key, wchar_t const* name, DWORD& out) {
    DWORD value = 0;
    DWORD size = sizeof(value);
    DWORD type = 0;
    if (RegQueryValueExW(key, name, nullptr, &type,
                         reinterpret_cast<BYTE*>(&value), &size) !=
        ERROR_SUCCESS)
        return false;
    if (type != REG_DWORD) return false;
    out = value;
    return true;
}

// Snapshot the prior state once, before anything is written. Safe to call
// repeatedly; only the first call records.
inline bool Capture(DwordSetting& setting) {
    if (setting.captured) return true;
    HKEY key = nullptr;
    if (RegOpenKeyExW(setting.root, setting.subKey, 0, KEY_READ, &key) !=
        ERROR_SUCCESS)
        return false;
    for (auto& value : setting.values) {
        DWORD original = 0;
        value.existed = ReadDword(key, value.name, original);
        value.original = value.existed ? original : 0;
    }
    RegCloseKey(key);
    setting.captured = true;
    return true;
}

inline void Notify(DwordSetting const& setting) {
    if (!setting.broadcast) return;
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       reinterpret_cast<LPARAM>(setting.broadcast));
}

// Write the same value to every known alias. Captures first if it has not.
inline bool Write(DwordSetting& setting, DWORD value) {
    Capture(setting);
    HKEY key = nullptr;
    if (RegOpenKeyExW(setting.root, setting.subKey, 0, KEY_SET_VALUE, &key) !=
        ERROR_SUCCESS)
        return false;
    bool wrote = false;
    for (auto const& name : setting.values) {
        if (RegSetValueExW(key, name.name, 0, REG_DWORD,
                           reinterpret_cast<BYTE const*>(&value),
                           sizeof(value)) == ERROR_SUCCESS)
            wrote = true;
    }
    RegCloseKey(key);
    if (wrote) Notify(setting);
    return wrote;
}

// Put back exactly what was there. A value the mod invented is deleted rather
// than zeroed, so the machine ends up as it started.
inline void Restore(DwordSetting& setting) {
    if (!setting.captured) return;
    HKEY key = nullptr;
    if (RegOpenKeyExW(setting.root, setting.subKey, 0, KEY_SET_VALUE, &key) ==
        ERROR_SUCCESS) {
        for (auto const& value : setting.values) {
            if (value.existed) {
                RegSetValueExW(key, value.name, 0, REG_DWORD,
                               reinterpret_cast<BYTE const*>(&value.original),
                               sizeof(value.original));
            } else {
                RegDeleteValueW(key, value.name);
            }
        }
        RegCloseKey(key);
        Notify(setting);
    }
    setting.captured = false;
}

// ---- Known-good definitions -------------------------------------------------
//
// Add to this list only after confirming the value against the live registry
// with the OS UI open. An unverified name here is worse than no entry.

inline constexpr wchar_t const* kExplorerAdvanced =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";

// Settings > System > Power & battery > Battery percentage.
// VERIFIED 2026-07-26: the Settings app reflects `IsBatteryPercentageEnabled`.
// `TaskbarBatteryPercent` lives in the same key and is NOT what Settings reads;
// it is written too, because some builds are reported to consult it.
inline DwordSetting BatteryPercentage() {
    DwordSetting setting;
    setting.root = HKEY_CURRENT_USER;
    setting.subKey = kExplorerAdvanced;
    setting.values = {{L"IsBatteryPercentageEnabled"},
                      {L"TaskbarBatteryPercent"}};
    setting.broadcast = kExplorerAdvanced;
    return setting;
}

}  // namespace windhawk_mod_templates::os_setting_bridge
