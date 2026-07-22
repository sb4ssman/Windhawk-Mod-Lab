#pragma once

#undef GetCurrentTime

// Copy-source template v1.2: place an owned taskbar group immediately left or
// right of Start. The group is appended to TaskbarFrame/RootGrid, the task
// repeater is given matching left-side room, and Start is counter-shifted by a
// constant amount so the group tracks Start's live layout position. Adopters:
// tray-utility-customizer (v1.2) and privacy-indicator-anchor (v1.1).
//
// v1.2 geometry fix: the group is centered vertically against the taskbar
// RootGrid, not against Start's reported box — Start's ActualHeight can
// include asymmetric padding, which visibly mis-centered groups in
// tray-utility-customizer live testing.
//
// v1.1 geometry fix: v1.0 pinned Start to an absolute X captured at Acquire
// and placed the Left-of-Start group at the taskbar's left edge. On a
// center-aligned taskbar Start's layout X moves with every task-list change,
// so the pinned anchor fought the layout and both sides drifted. The group is
// now positioned relative to Start's live position on every layout pass, and
// the Start counter-shift is a constant chosen from whether Start rides the
// repeater-margin push (visual-tree containment, checked at Acquire).

#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <algorithm>
#include <cmath>
#include <utility>

namespace windhawk_mod_templates::start_placement {

using winrt::Windows::UI::Xaml::DependencyObject;
using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::HorizontalAlignment;
using winrt::Windows::UI::Xaml::Thickness;
using winrt::Windows::UI::Xaml::UIElement;
using winrt::Windows::UI::Xaml::VerticalAlignment;
using winrt::Windows::UI::Xaml::Visibility;
using winrt::Windows::UI::Xaml::Automation::AutomationProperties;
using winrt::Windows::UI::Xaml::Controls::Canvas;
using winrt::Windows::UI::Xaml::Controls::Grid;
using winrt::Windows::UI::Xaml::Media::TranslateTransform;
using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;

enum class Side {
    Left,
    Right,
};

struct Lease {
    Grid group{nullptr};
    Grid rootGrid{nullptr};
    FrameworkElement startButton{nullptr};
    FrameworkElement taskItemsPanel{nullptr};
    Thickness groupOriginalMargin{};
    Thickness taskItemsPanelOriginalMargin{};
    bool startInTaskItemsPanel = false;
    winrt::event_token layoutToken{};
    Side side = Side::Left;
    double spacing = 0.0;
};

template<typename Predicate>
inline FrameworkElement FindDescendant(FrameworkElement const& root,
                                       Predicate&& predicate,
                                       int depth = 0) {
    if (!root || depth > 64)
        return nullptr;
    if (predicate(root))
        return root;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i)
                         .try_as<FrameworkElement>();
        auto match = FindDescendant(
            child, std::forward<Predicate>(predicate), depth + 1);
        if (match)
            return match;
    }
    return nullptr;
}

inline Grid FindTaskbarRootGrid(FrameworkElement const& root) {
    auto taskbarFrame = FindDescendant(
        root, [](FrameworkElement const& element) {
            return winrt::get_class_name(element) ==
                   L"Taskbar.TaskbarFrame";
        });
    if (!taskbarFrame)
        return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(taskbarFrame);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(taskbarFrame, i)
                         .try_as<Grid>();
        if (child && child.Name() == L"RootGrid")
            return child;
    }
    return nullptr;
}

inline FrameworkElement FindStartButton(FrameworkElement const& root) {
    return FindDescendant(
        root, [](FrameworkElement const& element) {
            return winrt::get_class_name(element) ==
                       L"Taskbar.ExperienceToggleButton" &&
                   AutomationProperties::GetAutomationId(element) ==
                       L"StartButton";
        });
}

inline bool Position(Lease& lease) noexcept {
    if (!lease.group || !lease.rootGrid || !lease.startButton)
        return false;

    try {
        double groupWidth = lease.group.Width() +
                            lease.groupOriginalMargin.Left +
                            lease.groupOriginalMargin.Right;
        double groupHeight = lease.group.Height() +
                             lease.groupOriginalMargin.Top +
                             lease.groupOriginalMargin.Bottom;
        bool startHidden =
            lease.startButton.Visibility() == Visibility::Collapsed;
        double startWidth = lease.startButton.ActualWidth();
        double startHeight = lease.startButton.ActualHeight();
        if (startWidth <= 0.0 && !startHidden)
            startWidth = 44.0;
        if (startHeight <= 0.0)
            startHeight = groupHeight;

        // rawX is Start's live layout position with our own counter-shift
        // backed out. It is re-read on every layout pass, so task-list churn
        // on a center-aligned taskbar re-centers the group naturally.
        auto transform = lease.startButton.TransformToVisual(lease.rootGrid);
        auto point = transform.TransformPoint({0.0f, 0.0f});
        auto existingShift =
            lease.startButton.RenderTransform().try_as<TranslateTransform>();
        double currentShift = existingShift ? existingShift.X() : 0.0;
        double rawX = point.X - currentShift;

        double spacing = std::max(0.0, lease.spacing);
        double push = groupWidth + spacing;
        if (lease.taskItemsPanel) {
            auto margin = lease.taskItemsPanel.Margin();
            double needed =
                lease.taskItemsPanelOriginalMargin.Left + push;
            if (std::fabs(margin.Left - needed) > 0.5) {
                margin.Left = needed;
                lease.taskItemsPanel.Margin(margin);
            }
        }

        // The Start counter-shift is a constant per mode, not an absolute-
        // anchor correction. When Start rides the repeater-margin push, room
        // for a Left group already opens at the block's left edge (no shift),
        // and a Right group needs Start pulled back so the gap opens between
        // Start and the task items. When Start sits outside the repeater the
        // roles invert: the pushed items leave the Right gap by themselves,
        // and a Left group needs Start pushed out of the way instead.
        double neededShift;
        if (lease.side == Side::Left)
            neededShift = lease.startInTaskItemsPanel ? 0.0 : push;
        else
            neededShift = lease.startInTaskItemsPanel ? -push : 0.0;
        if (startHidden)
            neededShift = 0.0;

        if (std::fabs(neededShift) <= 0.5) {
            if (existingShift || lease.startButton.RenderTransform())
                lease.startButton.ClearValue(
                    UIElement::RenderTransformProperty());
        } else if (std::fabs(currentShift - neededShift) > 0.5) {
            TranslateTransform startShift;
            startShift.X(neededShift);
            lease.startButton.RenderTransform(startShift);
        }

        // Place the group relative to where Start actually ends up.
        double startFinalX = rawX + neededShift;
        double left = lease.side == Side::Left
                          ? startFinalX - groupWidth - spacing
                          : startFinalX + startWidth + spacing;
        if (left < 0.0)
            left = 0.0;

        // v1.2: center against the taskbar root; Start's own box is not a
        // reliable vertical reference.
        double rootHeight = lease.rootGrid.ActualHeight();
        double top = rootHeight > 0.0
                         ? (rootHeight - groupHeight) / 2.0
                         : point.Y + (startHeight - groupHeight) / 2.0;
        if (top < 0.0)
            top = 0.0;
        double rootWidth = lease.rootGrid.ActualWidth();
        if (rootWidth > 0.0 && left + groupWidth > rootWidth)
            left = std::max(0.0, rootWidth - groupWidth);

        auto target = lease.groupOriginalMargin;
        target.Left += left;
        target.Top += top;
        auto current = lease.group.Margin();
        if (std::fabs(current.Left - target.Left) > 0.5 ||
            std::fabs(current.Top - target.Top) > 0.5) {
            lease.group.Margin(target);
        }
        return true;
    } catch (...) {
        return false;
    }
}

inline bool Release(Lease& lease) noexcept {
    if (!lease.group)
        return false;

    try {
        if (lease.rootGrid && lease.layoutToken)
            lease.rootGrid.LayoutUpdated(lease.layoutToken);
        if (lease.taskItemsPanel)
            lease.taskItemsPanel.Margin(
                lease.taskItemsPanelOriginalMargin);
        if (lease.startButton)
            lease.startButton.ClearValue(
                UIElement::RenderTransformProperty());
        lease.group.Margin(lease.groupOriginalMargin);
        if (lease.rootGrid) {
            uint32_t index = 0;
            if (lease.rootGrid.Children().IndexOf(lease.group, index))
                lease.rootGrid.Children().RemoveAt(index);
        }
    } catch (...) {
        lease = {};
        return false;
    }
    lease = {};
    return true;
}

inline bool Acquire(FrameworkElement const& root, Grid const& group,
                    Side side, double spacing, Lease& lease) {
    if (!root || !group || lease.group || group.Width() <= 0.0 ||
        group.Height() <= 0.0)
        return false;

    auto rootGrid = FindTaskbarRootGrid(root);
    auto startButton = FindStartButton(root);
    if (!rootGrid || !startButton)
        return false;

    lease.group = group;
    lease.rootGrid = rootGrid;
    lease.startButton = startButton;
    lease.groupOriginalMargin = group.Margin();
    lease.side = side;
    lease.spacing = spacing;

    group.HorizontalAlignment(HorizontalAlignment::Left);
    group.VerticalAlignment(VerticalAlignment::Top);
    Grid::SetColumn(group, 0);
    Grid::SetColumnSpan(
        group,
        std::max(1, static_cast<int>(
                        rootGrid.ColumnDefinitions().Size())));
    Canvas::SetZIndex(group, 1000);
    rootGrid.Children().Append(group);

    lease.taskItemsPanel = FindDescendant(
        rootGrid, [](FrameworkElement const& element) {
            return element.Name() == L"TaskbarFrameRepeater";
        });
    if (lease.taskItemsPanel) {
        lease.taskItemsPanelOriginalMargin =
            lease.taskItemsPanel.Margin();
        // Whether Start rides the repeater-margin push is build-dependent.
        // Resolve it from the visual tree instead of inferring from motion.
        try {
            auto panel = lease.taskItemsPanel.as<DependencyObject>();
            for (auto node = startButton.as<DependencyObject>(); node;
                 node = VisualTreeHelper::GetParent(node)) {
                if (node == panel) {
                    lease.startInTaskItemsPanel = true;
                    break;
                }
            }
        } catch (...) {
            lease.startInTaskItemsPanel = false;
        }
    }

    if (!Position(lease)) {
        Release(lease);
        return false;
    }
    lease.layoutToken = rootGrid.LayoutUpdated(
        [&lease](auto const&, auto const&) {
            Position(lease);
        });
    return true;
}

} // namespace windhawk_mod_templates::start_placement
