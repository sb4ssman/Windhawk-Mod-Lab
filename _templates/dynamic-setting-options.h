#pragma once

#include <cwchar>
#include <cwctype>
#include <span>
#include <string>

// Include Windhawk's API before this header. These are option labels in local
// mod storage, not user settings; publishing never changes a selected value.
namespace windhawk_mod_templates::dynamic_setting_options {

struct Option {
    wchar_t const* value;
    wchar_t const* label;
    bool available = true;
};

inline bool ValidKeyPart(wchar_t const* text, bool allowEmpty) {
    if (!text || (!allowEmpty && !*text)) return false;
    size_t length = wcslen(text);
    return !wcspbrk(text, L"=\r\n") &&
           (!length || !iswspace(text[length - 1]));
}

// Supply the complete set this publisher owns, including unavailable choices.
// Empty labels remove unavailable choices from the 2.0 dropdown and clear old
// labels from a previous OS/session. Other storage keys are never touched.
// Validate the whole set before writing; report any storage failure to caller.
inline bool Publish(wchar_t const* settingPath, std::span<Option const> options) {
    if (!ValidKeyPart(settingPath, false)) return false;
    for (size_t i = 0; i < options.size(); ++i) {
        auto const& option = options[i];
        if (!ValidKeyPart(option.value, true)) return false;
        if (option.available && (!option.label || !*option.label ||
                                wcspbrk(option.label, L"\r\n"))) return false;
        for (size_t j = 0; j < i; ++j)
            if (wcscmp(option.value, options[j].value) == 0) return false;
    }
    bool success = true;
    for (auto const& option : options) {
        std::wstring key = L"::wh_select_option::";
        key += settingPath;
        key += L"::";
        key += option.value;
        bool written = Wh_SetStringValue(key.c_str(), option.available ? option.label : L"") != 0;
        success = written && success;
    }
    return success;
}

} // namespace windhawk_mod_templates::dynamic_setting_options
