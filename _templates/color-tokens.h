#pragma once

// Copy-source template v1.0: THE color token parser. One implementation, two
// return shapes.
//
// Three copies of this table existed across the lab — button-surface.h
// (Brush-returning), OmniButton's ParseHexColor (Color-returning), and Privacy
// Anchor's. They agreed by luck rather than by construction, which is exactly
// how a mod ends up accepting `accentLight2` while its neighbour does not.
//
// THE VOCABULARY, identical in every mod and every settings description:
//
//   ""                  keep the native value. NOT a color. A parser failure
//                       and an empty setting are the same answer — "leave it
//                       alone" — so callers must ClearValue or simply not
//                       write, never substitute a default color.
//   "#RRGGBB"           opaque
//   "#AARRGGBB"         alpha honored
//   "transparent"       fully transparent: nothing drawn, element still
//                       present, still hit-testable
//   "accent"            the Windows accent color
//   "accentLight"       AccentLight2 — the natural hover shade
//   "accentDark"        AccentDark1 — the natural pressed shade
//   "accentLight1".."3" / "accentDark1".."3"
//                       accepted silently. Deliberately kept OUT of the
//                       user-facing descriptions to keep them short; they are
//                       here so a user who knows Windows' shade names is not
//                       told they are wrong.
//
// Bare hex without the leading '#' is accepted for back-compat with settings
// written before the '#' was documented. Do not advertise it.

#include <cwctype>
#include <cstdlib>

#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.Media.h>

namespace windhawk_mod_templates::color_tokens {

using winrt::Windows::UI::Color;
using winrt::Windows::UI::Xaml::Media::Brush;
using winrt::Windows::UI::Xaml::Media::SolidColorBrush;

// Reported when the Windows accent color cannot be read, so the mod can log.
using AccentErrorFn = void (*)();

// false means "no color here" — an empty setting, an unknown token, or bad
// hex. Callers must treat all three the same: leave the native value alone.
inline bool Parse(wchar_t const* value, Color& out,
                  AccentErrorFn onAccentError = nullptr) {
    using winrt::Windows::UI::ViewManagement::UIColorType;
    if (!value || !*value) return false;

    if (_wcsicmp(value, L"transparent") == 0) {
        out = {0, 0, 0, 0};
        return true;
    }

    static const struct {
        wchar_t const* token;
        UIColorType type;
    } kAccentTokens[] = {
        {L"accent", UIColorType::Accent},
        {L"accentLight", UIColorType::AccentLight2},
        {L"accentDark", UIColorType::AccentDark1},
        {L"accentLight1", UIColorType::AccentLight1},
        {L"accentLight2", UIColorType::AccentLight2},
        {L"accentLight3", UIColorType::AccentLight3},
        {L"accentDark1", UIColorType::AccentDark1},
        {L"accentDark2", UIColorType::AccentDark2},
        {L"accentDark3", UIColorType::AccentDark3},
    };
    for (auto const& entry : kAccentTokens) {
        if (_wcsicmp(value, entry.token) != 0) continue;
        try {
            winrt::Windows::UI::ViewManagement::UISettings settings;
            out = settings.GetColorValue(entry.type);
            return true;
        } catch (...) {
            if (onAccentError) onAccentError();
            return false;
        }
    }

    wchar_t const* digits = (*value == L'#') ? value + 1 : value;
    size_t length = wcslen(digits);
    if (length != 6 && length != 8) return false;
    for (size_t i = 0; i < length; ++i) {
        if (!iswxdigit(digits[i])) return false;
    }
    wchar_t buffer[9]{};
    wcsncpy(buffer, digits, 8);
    unsigned long packed = wcstoul(buffer, nullptr, 16);
    if (length == 6) {
        out = {255, BYTE(packed >> 16), BYTE(packed >> 8), BYTE(packed)};
    } else {
        out = {BYTE(packed >> 24), BYTE(packed >> 16), BYTE(packed >> 8),
               BYTE(packed)};
    }
    return true;
}

// nullptr means "no color here". Never a fallback brush — a caller that wrote
// a default color on parse failure would make an empty setting paint.
inline Brush ParseBrush(wchar_t const* value,
                        AccentErrorFn onAccentError = nullptr) {
    Color color{};
    if (!Parse(value, color, onAccentError)) return nullptr;
    SolidColorBrush brush;
    brush.Color(color);
    return brush;
}

}  // namespace windhawk_mod_templates::color_tokens
