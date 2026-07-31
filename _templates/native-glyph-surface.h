#pragma once

// Copy-source template v1.2: styling a NATIVE taskbar item the mod does not
// own. Use this for system-tray glyphs, the OmniButton's wifi/volume/battery
// items, privacy indicators — anything Windows drew that the mod is borrowing.
// For XAML Buttons the mod itself created, use button-surface.h instead.
//
// WHY THIS EXISTS. Every mod in the family had grown its own version of "walk
// down to the styleable leaf and set Foreground/FontSize/FontFamily", and they
// were all wrong in the same way: they assumed the leaf is a TextBlock. It
// usually is — a Windows 11 tray icon is a font glyph in a TextBlock named
// InnerTextBlock — but not always. The OmniButton's battery is a Grid of
// SHAPES, because it has to draw a fill level and a charging bolt, which a
// font glyph cannot do. A blind "find the first TextBlock anywhere below"
// search still finds *a* TextBlock down there and binds to it, so the settings
// look wired up and silently do nothing.
//
// So this template PROBES before it styles, and reports what it found. A mod
// asks the surface what it supports and can then present only the settings
// that will actually do something, instead of offering a font-family box for
// something with no font.
//
// PROPERTY LEASING. This template deliberately does not own a snapshot store.
// The mod already leases every property it mutates so it can restore the exact
// prior local value on unload (see taskbar-xaml-lifecycle.template.cpp), and
// two competing stores would restore in the wrong order. Pass the mod's own
// tracking function in; every mutation here routes through it first.

#include <functional>
#include <string>
#include <vector>

#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

namespace windhawk_mod_templates::native_glyph_surface {

using winrt::Windows::UI::Xaml::DependencyObject;
using winrt::Windows::UI::Xaml::DependencyProperty;
using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::UIElement;
using winrt::Windows::UI::Xaml::Controls::Control;
using winrt::Windows::UI::Xaml::Controls::TextBlock;
using winrt::Windows::UI::Xaml::Media::Brush;
using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;
using winrt::Windows::UI::Xaml::Shapes::Shape;

// How the native item draws itself, and therefore what can be changed.
enum class Kind {
    None,       // nothing stylable was found under the host
    TextGlyph,  // one or more TextBlocks. Color always applies; size and font
                // only when there is a SINGLE glyph — see Capabilities
    Shapes,     // Path/Rectangle/Ellipse drawing: only color applies, via
                // Fill and Stroke — there is no font to size or replace
    Opaque,     // something we can position and fade, but not recolor
};

inline wchar_t const* KindName(Kind kind) {
    switch (kind) {
        case Kind::TextGlyph: return L"text glyph";
        case Kind::Shapes:    return L"shapes";
        case Kind::Opaque:    return L"opaque";
        default:              return L"nothing";
    }
}

// What a probed surface will actually honor. Drive the settings UI from this
// rather than offering every control for every item.
struct Capabilities {
    bool color = false;
    bool fontSize = false;
    bool fontFamily = false;
    bool opacity = false;  // true whenever there is a host element at all
};

struct Surface {
    Kind kind = Kind::None;
    FrameworkElement host{nullptr};  // owns layout, position, and opacity
    TextBlock text{nullptr};         // set iff kind == TextGlyph
    Control anchor{nullptr};         // templated parent that owns text props
    std::vector<Shape> shapes;       // set iff kind == Shapes
    int glyphLayers = 0;             // stacked TextBlocks drawing ONE icon
    bool textWasUnnamed = false;     // matched by fallback, not by identity
    std::wstring detail;             // one line, for the mod's log

    explicit operator bool() const { return kind != Kind::None; }

    Capabilities Supports() const {
        Capabilities capabilities;
        capabilities.opacity = host != nullptr;
        switch (kind) {
            case Kind::TextGlyph:
                // Colour is safe on a stack: it goes to the ANCHOR, which
                // every layer inherits from, so they all move together.
                capabilities.color = true;
                // SIZE AND FONT ARE NOT.
                //
                // A native tray icon is frequently drawn by SEVERAL TextBlocks
                // stacked on top of each other, each holding one glyph of a
                // composite. Windows 11's wifi and volume are three deep —
                // AdaptiveTextBlocks named Underlay, Base and AccentOverlay —
                // and the battery is two, an outline and a fill. That is how
                // an icon shows signal strength, a mute slash, or a charge
                // level at all.
                //
                // Resizing or re-fonting such a stack pulls the layers apart:
                // they are only one icon because they are exactly registered
                // on top of each other. Verified 2026-07-26 — setting a glyph
                // size on wifi produced a visible GHOST, a large glyph over
                // the original, because the layers stopped coinciding.
                //
                // Probe() finds the FIRST matching TextBlock, so without this
                // count a three-layer icon reports itself as a single glyph
                // and the mod offers two controls that can only damage it.
                capabilities.fontSize = glyphLayers <= 1;
                capabilities.fontFamily = glyphLayers <= 1;
                break;
            case Kind::Shapes:
                capabilities.color = true;
                break;
            default:
                break;
        }
        return capabilities;
    }
};

// The standard Windows 11 tray glyph element. Named, so this is an identity
// match rather than a guess.
inline constexpr wchar_t const* kInnerTextBlock = L"InnerTextBlock";

// The element that OWNS a glyph's text properties, when that is not the glyph.
//
// SystemTray.IconView's `InnerTextBlock` is TEMPLATE-BOUND: its Foreground and
// FontSize come from its templated parent, not from itself. Writing a local
// value onto the TextBlock is one level too deep — the template re-asserts the
// binding and the write disappears, silently and permanently. The designed way
// to restyle such a glyph is to set the property on the templated parent and
// let it flow down.
//
// VERIFIED 2026-07-26 on the Windows 11 tray: with identical code, the battery
// and its percentage accepted colour and font size while wifi and volume
// accepted neither. Opacity worked on all four, because opacity is applied to
// the outer host, which no template owns. The dividing line was exactly
// "inside a SystemTray.IconView or not".
//
// Found by walking UP from the leaf: the parent chain is unambiguous, whereas
// a downward search would have to guess which of several Controls is the
// templated parent. Returns the OUTERMOST Control strictly between leaf and
// host, which is the IconView rather than some inner presenter.
inline Control FindStyleAnchor(FrameworkElement const& host,
                               DependencyObject const& leaf) {
    if (!host || !leaf) return nullptr;
    Control anchor = nullptr;
    DependencyObject node = leaf;
    for (int depth = 0; depth < 32; ++depth) {
        auto parent = VisualTreeHelper::GetParent(node);
        if (!parent) break;
        if (parent.try_as<FrameworkElement>() == host) break;
        if (auto control = parent.try_as<Control>()) anchor = control;
        node = parent;
    }
    return anchor;
}

inline TextBlock FindNamedTextBlock(DependencyObject const& root,
                                    wchar_t const* name, int maxDepth,
                                    int depth = 0) {
    if (!root || depth > maxDepth) return nullptr;
    if (auto element = root.try_as<FrameworkElement>())
        if (element.Name() == name)
            if (auto text = element.try_as<TextBlock>())
                return text;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        if (auto found = FindNamedTextBlock(child, name, maxDepth, depth + 1))
            return found;
    }
    return nullptr;
}

inline TextBlock FindAnyTextBlock(DependencyObject const& root, int maxDepth,
                                  int depth = 0) {
    if (!root || depth > maxDepth) return nullptr;
    if (auto text = root.try_as<TextBlock>()) return text;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        if (auto found = FindAnyTextBlock(child, maxDepth, depth + 1))
            return found;
    }
    return nullptr;
}

// How many TextBlocks draw this item. One is a plain glyph; more than one is a
// STACK that must be resized together or not at all — see Capabilities.
inline int CountTextBlocks(DependencyObject const& root, int maxDepth,
                           int depth = 0) {
    if (!root || depth > maxDepth) return 0;
    if (root.try_as<TextBlock>()) return 1;  // leaves; no TextBlock nests one
    int total = 0;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        total += CountTextBlocks(child, maxDepth, depth + 1);
    }
    return total;
}

inline void CollectShapes(DependencyObject const& root, int maxDepth,
                          std::vector<Shape>& out, int depth = 0) {
    if (!root || depth > maxDepth) return;
    if (auto shape = root.try_as<Shape>()) out.push_back(shape);
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        CollectShapes(child, maxDepth, out, depth + 1);
    }
}

// Work out what `host` is made of.
//
// PRECEDENCE MATTERS, and it is not the obvious order:
//
//   1. host is itself a TextBlock            — nothing to search for
//   2. a descendant named InnerTextBlock     — identity, the documented glyph
//   3. any Shape descendants                 — a drawn icon
//   4. any TextBlock at all                  — last resort, flagged as such
//
// Steps 3 and 4 are in this order on purpose. Searching for a bare TextBlock
// before checking for shapes is exactly the bug this template replaces: a
// drawn icon can still have some incidental TextBlock buried under it, and
// binding to that produces settings that appear to work and never do.
//
// A host whose template has not expanded yet legitimately probes to None —
// XAML materializes a ContentPresenter's content after the presenter itself
// appears. Callers must re-probe rather than treat the first None as final.
inline Surface Probe(FrameworkElement const& host, int maxDepth = 12) {
    Surface surface;
    surface.host = host;
    if (!host) {
        surface.detail = L"no host element";
        return surface;
    }

    if (auto text = host.try_as<TextBlock>()) {
        surface.kind = Kind::TextGlyph;
        surface.text = text;
        surface.glyphLayers = 1;
        surface.detail = L"host is itself a TextBlock";
        return surface;
    }

    if (auto text = FindNamedTextBlock(host, kInnerTextBlock, maxDepth)) {
        surface.kind = Kind::TextGlyph;
        surface.text = text;
        surface.anchor = FindStyleAnchor(host, text);
        surface.glyphLayers = CountTextBlocks(host, maxDepth);
        surface.detail =
            surface.glyphLayers > 1
                ? L"a STACK of " + std::to_wstring(surface.glyphLayers) +
                      L" layered glyphs - colour only, no size or font"
                : (surface.anchor ? L"TextBlock named InnerTextBlock, styled "
                                    L"through its templated parent"
                                  : L"TextBlock named InnerTextBlock");
        return surface;
    }

    CollectShapes(host, maxDepth, surface.shapes);
    if (!surface.shapes.empty()) {
        surface.kind = Kind::Shapes;
        surface.detail = L"drawn from " +
                         std::to_wstring(surface.shapes.size()) +
                         L" shape(s) - no font to size or replace";
        return surface;
    }

    if (auto text = FindAnyTextBlock(host, maxDepth)) {
        surface.kind = Kind::TextGlyph;
        surface.text = text;
        surface.anchor = FindStyleAnchor(host, text);
        surface.glyphLayers = CountTextBlocks(host, maxDepth);
        surface.textWasUnnamed = true;
        surface.detail =
            surface.glyphLayers > 1
                ? L"a STACK of " + std::to_wstring(surface.glyphLayers) +
                      L" unnamed TextBlocks - colour only, no size or font"
                : L"unnamed TextBlock (fallback - verify it is really what "
                  L"draws this item)";
        return surface;
    }

    surface.kind = host ? Kind::Opaque : Kind::None;
    surface.detail = L"no text or shapes found; opacity and position only";
    return surface;
}

// The mod's property-lease hook. Every mutation below is announced through
// this before it happens, so the mod can snapshot the prior local value.
using TrackFn =
    std::function<void(DependencyObject const&, DependencyProperty const&)>;

// WRITE THE ANCHOR FIRST, THEN THE LEAF. When the glyph is template-bound the
// anchor is the only write that survives; when it is not, the leaf's local
// value wins and the anchor's is harmlessly inherited past. Both are leased,
// so the restore is unaffected either way, and one code path covers both
// shapes of tray item instead of a per-item special case.
//
// A null brush means "leave the native color alone" — never a fallback color.
inline bool ApplyColor(Surface const& surface, Brush const& brush,
                       TrackFn const& track) {
    if (!brush || !surface.Supports().color) return false;
    if (surface.kind == Kind::TextGlyph) {
        if (surface.anchor) {
            if (track) track(surface.anchor, Control::ForegroundProperty());
            try {
                surface.anchor.Foreground(brush);
            } catch (...) {
            }
        }
        if (track) track(surface.text, TextBlock::ForegroundProperty());
        try {
            surface.text.Foreground(brush);
        } catch (...) {
            return false;
        }
        return true;
    }

    bool applied = false;
    for (auto const& shape : surface.shapes) {
        // Only repaint what the shape already paints. A shape with no fill is
        // an outline and a shape with no stroke is a solid; overriding the
        // absent one would add a border or a blob that was never there.
        if (shape.Fill()) {
            if (track) track(shape, Shape::FillProperty());
            try {
                shape.Fill(brush);
                applied = true;
            } catch (...) {
            }
        }
        if (shape.Stroke()) {
            if (track) track(shape, Shape::StrokeProperty());
            try {
                shape.Stroke(brush);
                applied = true;
            } catch (...) {
            }
        }
    }
    return applied;
}

// Sizes in points; 0 or less means "keep the native size".
inline bool ApplyFontSize(Surface const& surface, double points,
                          TrackFn const& track) {
    if (points <= 0.0 || !surface.Supports().fontSize) return false;
    if (surface.anchor) {
        if (track) track(surface.anchor, Control::FontSizeProperty());
        try {
            surface.anchor.FontSize(points);
        } catch (...) {
        }
    }
    if (track) track(surface.text, TextBlock::FontSizeProperty());
    try {
        surface.text.FontSize(points);
    } catch (...) {
        return false;
    }
    return true;
}

// An empty family means "keep the native font".
inline bool ApplyFontFamily(Surface const& surface, std::wstring const& family,
                            TrackFn const& track) {
    if (family.empty() || !surface.Supports().fontFamily) return false;
    if (surface.anchor) {
        if (track) track(surface.anchor, Control::FontFamilyProperty());
        try {
            surface.anchor.FontFamily(
                winrt::Windows::UI::Xaml::Media::FontFamily(family));
        } catch (...) {
        }
    }
    if (track) track(surface.text, TextBlock::FontFamilyProperty());
    try {
        surface.text.FontFamily(
            winrt::Windows::UI::Xaml::Media::FontFamily(family));
    } catch (...) {
        return false;
    }
    return true;
}

// Percent, or a negative value to keep the native opacity. Applied to the HOST
// rather than the glyph, so it fades the whole item including any chrome.
inline bool ApplyOpacity(Surface const& surface, int percent,
                         TrackFn const& track) {
    if (percent < 0 || !surface.Supports().opacity) return false;
    if (track) track(surface.host, UIElement::OpacityProperty());
    try {
        surface.host.Opacity((double)percent / 100.0);
    } catch (...) {
        return false;
    }
    return true;
}

// Natural size of a native element with nothing the mod imposed on it.
//
// A TEXT item must be measured, never assumed. "9%", "80%", and "100%" are
// three different widths and a font or locale change moves them again, so one
// Size.ItemWidth cannot describe both a 20x16 drawn icon and a string. Feed
// the result into the layout's SizeResolver so the arrangement RESERVES the
// width, rather than discovering the overflow at paint time and clipping it.
inline winrt::Windows::Foundation::Size MeasureNatural(
    FrameworkElement const& element, TrackFn const& track) {
    if (!element) return {};
    try {
        if (track) {
            track(element, FrameworkElement::WidthProperty());
            track(element, FrameworkElement::HeightProperty());
        }
        element.Width(std::numeric_limits<double>::quiet_NaN());
        element.Height(std::numeric_limits<double>::quiet_NaN());
        element.Measure({std::numeric_limits<float>::infinity(),
                         std::numeric_limits<float>::infinity()});
        return element.DesiredSize();
    } catch (...) {
        return {};
    }
}

}  // namespace windhawk_mod_templates::native_glyph_surface
