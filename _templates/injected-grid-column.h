#pragma once

// Copy-source template v1.0: reversible SystemTrayFrameGrid column lease.

#include <algorithm>
#include <string>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

namespace windhawk_mod_templates::injected_grid_column {

using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::GridUnitType;
using winrt::Windows::UI::Xaml::Controls::ColumnDefinition;
using winrt::Windows::UI::Xaml::Controls::Grid;

enum class Anchor {
    BeforeIcons,
    BeforeOmni,
    BeforeClock,
    AfterClock,
    AfterShowDesktop,
};

struct Lease {
    std::wstring markerName;
    int column = -1;
};

inline FrameworkElement FindDirectChild(Grid const& parent,
                                        wchar_t const* name) {
    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (element && element.Name() == name)
            return element;
    }
    return nullptr;
}

inline bool ResolveColumn(Grid const& parent, Anchor anchor, int& column) {
    if (anchor == Anchor::BeforeIcons) {
        column = 0;
        return true;
    }

    wchar_t const* referenceName = nullptr;
    bool after = false;
    switch (anchor) {
        case Anchor::BeforeOmni:
            referenceName = L"ControlCenterButton";
            break;
        case Anchor::BeforeClock:
            referenceName = L"NotificationCenterButton";
            break;
        case Anchor::AfterClock:
            referenceName = L"ShowDesktopStack";
            break;
        case Anchor::AfterShowDesktop:
            referenceName = L"ShowDesktopStack";
            after = true;
            break;
        case Anchor::BeforeIcons:
            break;
    }

    auto reference = FindDirectChild(parent, referenceName);
    if (!reference)
        return false; // Never silently turn an unavailable anchor into column 0.
    column = Grid::GetColumn(reference) + (after ? 1 : 0);
    return true;
}

inline bool Acquire(Grid const& parent, Anchor anchor,
                    std::wstring const& markerName, Lease& lease) {
    if (!parent || markerName.empty() || FindDirectChild(parent, markerName.c_str()))
        return false;

    int column = -1;
    if (!ResolveColumn(parent, anchor, column))
        return false;

    ColumnDefinition definition;
    definition.Width({1.0, GridUnitType::Auto});
    if (static_cast<uint32_t>(column) < parent.ColumnDefinitions().Size())
        parent.ColumnDefinitions().InsertAt(column, definition);
    else
        parent.ColumnDefinitions().Append(definition);

    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (!element) continue;
        int start = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (start >= column)
            Grid::SetColumn(element, start + 1);
        else if (start + span > column)
            Grid::SetColumnSpan(element, span + 1);
    }

    Grid marker;
    marker.Name(markerName);
    marker.Width(0.0);
    marker.Height(0.0);
    marker.IsHitTestVisible(false);
    Grid::SetColumn(marker, column);
    parent.Children().Append(marker);

    lease = {markerName, column};
    return true;
}

inline bool Release(Grid const& parent, Lease& lease) {
    if (!parent || lease.markerName.empty())
        return false;

    uint32_t markerIndex = 0;
    bool found = false;
    int liveColumn = lease.column;
    for (uint32_t i = 0; i < parent.Children().Size(); ++i) {
        auto element = parent.Children().GetAt(i).try_as<FrameworkElement>();
        if (element && element.Name() == lease.markerName) {
            markerIndex = i;
            liveColumn = Grid::GetColumn(element);
            found = true;
            break;
        }
    }
    if (!found || liveColumn < 0)
        return false;

    parent.Children().RemoveAt(markerIndex);
    if (static_cast<uint32_t>(liveColumn) < parent.ColumnDefinitions().Size())
        parent.ColumnDefinitions().RemoveAt(liveColumn);

    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (!element) continue;
        int start = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (start > liveColumn)
            Grid::SetColumn(element, start - 1);
        else if (start < liveColumn && start + span > liveColumn)
            Grid::SetColumnSpan(element, std::max(1, span - 1));
    }

    lease = {};
    return true;
}

} // namespace windhawk_mod_templates::injected_grid_column
