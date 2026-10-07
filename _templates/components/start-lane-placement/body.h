
using winrt::Windows::UI::Xaml::DependencyObject;
using winrt::Windows::UI::Xaml::DependencyProperty;
using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::Foundation::IInspectable;
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

// BEFORE or AFTER Start along the taskbar. On a bottom or top taskbar that is
// left or right of Start; on a left or right taskbar, where Start sits at the
// top and the task list runs down, Left means ABOVE Start and Right BELOW it.
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
    IInspectable taskItemsPanelMarginLocal{nullptr};
    IInspectable startRenderTransformLocal{nullptr};
    bool startInTaskItemsPanel = false;
    winrt::event_token layoutToken{};
    Side side = Side::Left;
    double spacing = 0.0;
    // A left or right taskbar: the lane runs vertically. Fixed for the
    // lease's life; a move between edges re-acquires.
    bool vertical = false;
};

// The leading edge of a margin along the lane: Left across a horizontal
// taskbar, Top down a vertical one.
inline double& LaneLead(Thickness& thickness, bool vertical) {
    return vertical ? thickness.Top : thickness.Left;
}

inline void RestoreLocalValue(DependencyObject const& object,
                              DependencyProperty const& property,
                              IInspectable const& value) {
    if (value == DependencyProperty::UnsetValue()) {
        object.ClearValue(property);
    } else {
        object.SetValue(property, value);
    }
}

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
        // ALONG is the lane's direction (x on a horizontal taskbar, y on a
        // vertical one); ACROSS is the taskbar's thickness. Every quantity
        // below is read along or across, so one arithmetic serves both.
        bool vertical = lease.vertical;
        auto const& original = lease.groupOriginalMargin;
        double groupWidth = lease.group.Width() + original.Left + original.Right;
        double groupHeight =
            lease.group.Height() + original.Top + original.Bottom;
        double groupAlong = vertical ? groupHeight : groupWidth;
        double groupAcross = vertical ? groupWidth : groupHeight;
        bool startHidden =
            lease.startButton.Visibility() == Visibility::Collapsed;
        double startAlong = vertical ? lease.startButton.ActualHeight()
                                     : lease.startButton.ActualWidth();
        double startAcross = vertical ? lease.startButton.ActualWidth()
                                      : lease.startButton.ActualHeight();
        if (startAlong <= 0.0 && !startHidden)
            startAlong = 44.0;
        if (startAcross <= 0.0)
            startAcross = groupAcross;

        // rawAlong is Start's live layout position with our own counter-shift
        // backed out. It is re-read on every layout pass, so task-list churn
        // on a center-aligned taskbar re-centers the group naturally.
        auto transform = lease.startButton.TransformToVisual(lease.rootGrid);
        auto point = transform.TransformPoint({0.0f, 0.0f});
        auto existingShift =
            lease.startButton.RenderTransform().try_as<TranslateTransform>();
        double currentShift =
            existingShift ? (vertical ? existingShift.Y() : existingShift.X())
                          : 0.0;
        double pointAlong = vertical ? point.Y : point.X;
        double pointAcross = vertical ? point.X : point.Y;
        double rawAlong = pointAlong - currentShift;

        double spacing = std::max(0.0, lease.spacing);
        double push = groupAlong + spacing;
        if (lease.taskItemsPanel) {
            auto margin = lease.taskItemsPanel.Margin();
            auto originalPanel = lease.taskItemsPanelOriginalMargin;
            double needed = LaneLead(originalPanel, vertical) + push;
            if (std::fabs(LaneLead(margin, vertical) - needed) > 0.5) {
                LaneLead(margin, vertical) = needed;
                lease.taskItemsPanel.Margin(margin);
            }
        }

        // The Start counter-shift is a constant per mode, not an absolute-
        // anchor correction. When Start rides the repeater-margin push, room
        // for a Left group already opens at the block's leading edge (no
        // shift), and a Right group needs Start pulled back so the gap opens
        // between Start and the task items. When Start sits outside the
        // repeater the roles invert: the pushed items leave the Right gap by
        // themselves, and a Left group needs Start pushed out of the way.
        double neededShift;
        if (lease.side == Side::Left)
            neededShift = lease.startInTaskItemsPanel ? 0.0 : push;
        else
            neededShift = lease.startInTaskItemsPanel ? -push : 0.0;
        if (startHidden)
            neededShift = 0.0;

        if (std::fabs(neededShift) <= 0.5) {
            RestoreLocalValue(lease.startButton,
                              UIElement::RenderTransformProperty(),
                              lease.startRenderTransformLocal);
        } else if (std::fabs(currentShift - neededShift) > 0.5) {
            TranslateTransform startShift;
            if (vertical)
                startShift.Y(neededShift);
            else
                startShift.X(neededShift);
            lease.startButton.RenderTransform(startShift);
        }

        // Place the group relative to where Start actually ends up.
        double startFinal = rawAlong + neededShift;
        double lead = lease.side == Side::Left
                          ? startFinal - groupAlong - spacing
                          : startFinal + startAlong + spacing;
        if (lead < 0.0)
            lead = 0.0;

        // Center across the taskbar root; Start's own box is not a reliable
        // reference for the thickness.
        double rootAcross = vertical ? lease.rootGrid.ActualWidth()
                                     : lease.rootGrid.ActualHeight();
        double startCentered =
            pointAcross + (startAcross - groupAcross) / 2.0;
        double across = rootAcross > 0.0 ? (rootAcross - groupAcross) / 2.0
                                         : startCentered;
        if (across < 0.0)
            across = 0.0;
        double rootAlong = vertical ? lease.rootGrid.ActualHeight()
                                    : lease.rootGrid.ActualWidth();
        if (rootAlong > 0.0 && lead + groupAlong > rootAlong)
            lead = std::max(0.0, rootAlong - groupAlong);

        auto target = lease.groupOriginalMargin;
        target.Left += vertical ? across : lead;
        target.Top += vertical ? lead : across;
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
            RestoreLocalValue(lease.taskItemsPanel,
                              FrameworkElement::MarginProperty(),
                              lease.taskItemsPanelMarginLocal);
        if (lease.startButton)
            RestoreLocalValue(lease.startButton,
                              UIElement::RenderTransformProperty(),
                              lease.startRenderTransformLocal);
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

// `vertical` is true on a left or right taskbar (taskbar_metrics::
// RunsDownSide); the lane then runs down from Start instead of across.
inline bool Acquire(FrameworkElement const& root, Grid const& group,
                    Side side, double spacing, Lease& lease,
                    bool vertical = false) {
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
    lease.startRenderTransformLocal = startButton.ReadLocalValue(
        UIElement::RenderTransformProperty());
    lease.side = side;
    lease.spacing = spacing;
    lease.vertical = vertical;

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
        lease.taskItemsPanelMarginLocal = lease.taskItemsPanel.ReadLocalValue(
            FrameworkElement::MarginProperty());
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
