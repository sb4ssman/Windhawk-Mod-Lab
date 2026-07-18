#pragma once

// Copy-source template v1.0: visual states for XAML Buttons owned by the mod.
// Do not use this for native glyphs, TextBlocks, or host-owned taskbar buttons.

#include <algorithm>
#include <cstdint>
#include <string>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

namespace windhawk_mod_templates::button_surface {

using winrt::Windows::UI::Xaml::Controls::Button;
using winrt::Windows::UI::Xaml::Controls::Control;
using winrt::Windows::UI::Xaml::Media::Brush;
using winrt::Windows::UI::Xaml::Media::GradientStop;
using winrt::Windows::UI::Xaml::Media::LinearGradientBrush;
using winrt::Windows::UI::Xaml::Media::SolidColorBrush;

struct Colors {
    std::wstring foreground;
    std::wstring background;
    std::wstring hoverBackground;
    std::wstring pressedBackground;
    std::wstring border;
};

struct Options {
    int opacityPercent = 100;
    int borderThickness = -1; // -1 preserves native value
    int cornerRadius = -1;    // -1 preserves native value
    bool shine = false;
};

// Accepted tokens: "#RRGGBB" or "#AARRGGBB" hex (alpha honored), the generics
// "accent" / "accentLight" / "accentDark" / "transparent", the numbered
// Windows shades "accentLight1"-"3" and "accentDark1"-"3" (accepted silently,
// undocumented), or empty for nullptr — callers must treat nullptr as "leave
// the native surface alone" (ClearValue, not a fallback color).
inline Brush ParseColor(std::wstring const& value) {
    using winrt::Windows::UI::ViewManagement::UIColorType;

    if (_wcsicmp(value.c_str(), L"transparent") == 0) {
        SolidColorBrush brush;
        brush.Color(winrt::Windows::UI::Color{0, 0, 0, 0});
        return brush;
    }

    static const struct { const wchar_t* token; UIColorType type; } kAccentTokens[] = {
        {L"accent",       UIColorType::Accent},
        {L"accentLight",  UIColorType::AccentLight2},
        {L"accentDark",   UIColorType::AccentDark1},
        {L"accentLight1", UIColorType::AccentLight1},
        {L"accentLight2", UIColorType::AccentLight2},
        {L"accentLight3", UIColorType::AccentLight3},
        {L"accentDark1",  UIColorType::AccentDark1},
        {L"accentDark2",  UIColorType::AccentDark2},
        {L"accentDark3",  UIColorType::AccentDark3},
    };
    for (auto const& entry : kAccentTokens) {
        if (_wcsicmp(value.c_str(), entry.token) == 0) {
            try {
                winrt::Windows::UI::ViewManagement::UISettings settings;
                auto color = settings.GetColorValue(entry.type);
                SolidColorBrush brush;
                brush.Color(color);
                return brush;
            } catch (...) {
                return nullptr;
            }
        }
    }

    if (value.empty() || value.front() != L'#')
        return nullptr;
    std::wstring hex = value.substr(1);
    if (hex.size() == 6) hex = L"FF" + hex;
    if (hex.size() != 8) return nullptr;

    uint32_t packed = 0;
    for (wchar_t c : hex) {
        packed <<= 4;
        if (c >= L'0' && c <= L'9') packed |= c - L'0';
        else if (c >= L'A' && c <= L'F') packed |= 10 + c - L'A';
        else if (c >= L'a' && c <= L'f') packed |= 10 + c - L'a';
        else return nullptr;
    }

    winrt::Windows::UI::Color color{
        static_cast<uint8_t>(packed >> 24), static_cast<uint8_t>(packed >> 16),
        static_cast<uint8_t>(packed >> 8), static_cast<uint8_t>(packed)};
    SolidColorBrush brush;
    brush.Color(color);
    return brush;
}

inline Brush MakeShine(Brush const& base, bool enabled) {
    auto solid = enabled && base ? base.try_as<SolidColorBrush>() : nullptr;
    if (!solid) return base;
    auto color = solid.Color();
    auto adjust = [](uint8_t value, int delta) {
        return static_cast<uint8_t>(std::clamp(static_cast<int>(value) + delta,
                                               0, 255));
    };

    LinearGradientBrush brush;
    brush.StartPoint({0.0, 0.0});
    brush.EndPoint({0.0, 1.0});

    GradientStop top;
    top.Color({180, 255, 255, 255});
    top.Offset(0.0);
    brush.GradientStops().Append(top);

    GradientStop light;
    light.Color({color.A, adjust(color.R, 34), adjust(color.G, 34),
                 adjust(color.B, 34)});
    light.Offset(0.42);
    brush.GradientStops().Append(light);

    GradientStop middle;
    middle.Color(color);
    middle.Offset(0.52);
    brush.GradientStops().Append(middle);

    GradientStop bottom;
    bottom.Color({color.A, adjust(color.R, -28), adjust(color.G, -28),
                  adjust(color.B, -28)});
    bottom.Offset(1.0);
    brush.GradientStops().Append(bottom);
    return brush;
}

inline void PutResource(Button const& button, wchar_t const* key,
                        Brush const& brush) {
    if (brush)
        button.Resources().Insert(winrt::box_value(key), brush);
    else
        button.Resources().Remove(winrt::box_value(key));
}

inline void Apply(Button const& button, Colors const& colors,
                  Options const& options) {
    auto foreground = ParseColor(colors.foreground);
    auto background = MakeShine(ParseColor(colors.background), options.shine);
    auto hover = MakeShine(ParseColor(colors.hoverBackground), options.shine);
    auto pressed = MakeShine(ParseColor(colors.pressedBackground), options.shine);
    auto border = ParseColor(colors.border);

    if (foreground) button.Foreground(foreground);
    else button.ClearValue(Control::ForegroundProperty());
    PutResource(button, L"ButtonForeground", foreground);
    PutResource(button, L"ButtonForegroundPointerOver", foreground);
    PutResource(button, L"ButtonForegroundPressed", foreground);

    if (background) button.Background(background);
    else button.ClearValue(Control::BackgroundProperty());
    PutResource(button, L"ButtonBackground", background);

    // Missing state colors deliberately preserve the native theme state.
    PutResource(button, L"ButtonBackgroundPointerOver", hover);
    PutResource(button, L"ButtonBackgroundPressed", pressed);

    if (border) button.BorderBrush(border);
    else button.ClearValue(Control::BorderBrushProperty());
    PutResource(button, L"ButtonBorderBrush", border);
    PutResource(button, L"ButtonBorderBrushPointerOver", border);
    PutResource(button, L"ButtonBorderBrushPressed", border);

    if (options.borderThickness >= 0) {
        double value = static_cast<double>(options.borderThickness);
        button.BorderThickness({value, value, value, value});
    } else {
        button.ClearValue(Control::BorderThicknessProperty());
    }

    if (options.cornerRadius >= 0) {
        double value = static_cast<double>(options.cornerRadius);
        button.CornerRadius({value, value, value, value});
    } else {
        button.ClearValue(Control::CornerRadiusProperty());
    }
    button.Opacity(std::clamp(options.opacityPercent, 0, 100) / 100.0);
}

} // namespace windhawk_mod_templates::button_surface
