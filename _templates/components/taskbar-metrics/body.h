// ---- Taskbar metrics, edge and orientation ----------------------------------
//
// WHERE THE TASKBAR IS, AND WHETHER THIS MOD CAN WORK THERE.
//
// Windows 11 builds with the native taskbar position setting (September 2026
// update) put the taskbar on any edge themselves. On those builds Windows
// ANNOUNCES the edge, and that announcement is what this component reads:
// the taskbar root Grid (Taskbar.TaskbarFrame > Grid#RootGrid) sits in a
// DockingStates visual state — DockedBottom, DockedTop, DockedLeft or
// DockedRight. It is the same signal m417z's own mods read.
//
// A NATIVE SIDE TASKBAR IS SUPPORTED. The tree is the same tree laid out
// vertically: every tray anchor keeps its name, order and parent. A written
// arrangement is laid out exactly as written there; only generated layouts
// fill across the taskbar's width (the arrangement component's `across`).
//
// A ROTATED TASKBAR IS NOT. m417z's Vertical Taskbar, with its native mode
// turned off (or on a build without the native setting), rotates a horizontal
// taskbar with RenderTransform on the very tray children this family positions
// — one property, two owners, last writer wins. Windows still reports a
// horizontal dock there while the window runs down the side, and that
// mismatch is how it is recognised. A mod stands down rather than paint
// garbage.
//
// The rect is in PHYSICAL pixels and every XAML size is a DIP, so conversion
// belongs here instead of being re-derived at each call site.

using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::VisualStateManager;
using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;

enum class Orientation { Horizontal, Vertical };
enum class Edge { Unknown, Bottom, Top, Left, Right };

struct Metrics {
    bool valid = false;
    RECT rect{};
    UINT dpi = 96;
    // What the window looks like: taller than wide runs down a side.
    Orientation orientation = Orientation::Horizontal;
    // What Windows says, when the caller read it (ReadDockedEdge).
    Edge edge = Edge::Unknown;
    // Runs down a side while Windows does not say it docked there: another
    // mod is rotating a horizontal taskbar.
    bool rotated = false;
    // The extent the arranged group has to fit INTO: the taskbar's height when
    // it runs across the screen, its width when it runs down the side.
    double constrainedDip = 0.0;
//@part alongDip
    // The extent it can run ALONG.
    double alongDip = 0.0;
//@end
};

//@part FindTaskbarChild
// The direct child of `parent` with this name, searching at most `levels`
// generations. The taskbar's top is shallow and fixed:
//   XamlRoot.Content() Grid > TaskbarFrame#TaskbarFrame > Grid#RootGrid
inline FrameworkElement FindTaskbarChild(FrameworkElement const& parent,
                                         wchar_t const* name, int levels) {
    if (!parent || levels <= 0) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; ++i) {
        auto child =
            VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    for (int i = 0; i < count; ++i) {
        auto child =
            VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (auto found = FindTaskbarChild(child, name, levels - 1)) return found;
    }
    return nullptr;
}
//@end

//@part ReadDockedEdge
// Windows' own statement of the edge. UI thread only. `taskbarRoot` is the
// taskbar XamlRoot's Content(). Unknown on builds without the native position
// setting, or if the tree has changed shape.
//
// Read ONLY RootGrid's DockingStates. Per-element OrientationStates further
// down (task-button IconPanels) were observed stale after a move back to the
// bottom; RootGrid's state was right on every edge.
inline Edge ReadDockedEdge(FrameworkElement const& taskbarRoot) {
    auto frame = FindTaskbarChild(taskbarRoot, L"TaskbarFrame", 2);
    auto rootGrid = FindTaskbarChild(frame, L"RootGrid", 2);
    if (!rootGrid) return Edge::Unknown;
    for (auto const& group : VisualStateManager::GetVisualStateGroups(rootGrid)) {
        if (group.Name() != L"DockingStates") continue;
        auto state = group.CurrentState();
        if (!state) return Edge::Unknown;
        auto name = state.Name();
        if (name == L"DockedBottom") return Edge::Bottom;
        if (name == L"DockedTop") return Edge::Top;
        if (name == L"DockedLeft") return Edge::Left;
        if (name == L"DockedRight") return Edge::Right;
        return Edge::Unknown;
    }
    return Edge::Unknown;
}
//@end

// `docked` is ReadDockedEdge's answer when the caller has the taskbar's XAML,
// Unknown otherwise. Without it a window running down the side is assumed
// rotated — the safe answer on a build that cannot say otherwise.
inline Metrics GetMetrics(HWND taskbarWnd, Edge docked = Edge::Unknown) {
    Metrics metrics;
    if (!taskbarWnd || !GetWindowRect(taskbarWnd, &metrics.rect))
        return metrics;

    metrics.valid = true;
    metrics.dpi = GetDpiForWindow(taskbarWnd);
    if (!metrics.dpi) metrics.dpi = 96;

    double width = (double)(metrics.rect.right - metrics.rect.left);
    double height = (double)(metrics.rect.bottom - metrics.rect.top);
    double scale = 96.0 / (double)metrics.dpi;

    metrics.orientation =
        height > width ? Orientation::Vertical : Orientation::Horizontal;
    metrics.edge = docked;
    metrics.rotated = metrics.orientation == Orientation::Vertical &&
                      docked != Edge::Left && docked != Edge::Right;
    bool horizontal = metrics.orientation == Orientation::Horizontal;
    metrics.constrainedDip = (horizontal ? height : width) * scale;
//@part alongDip
    metrics.alongDip = (horizontal ? width : height) * scale;
//@end
    return metrics;
}

//@part CanArrange
// Whether this mod may arrange here: any edge Windows placed the taskbar on
// itself, never a taskbar another mod is rotating. Checked BEFORE touching
// anything, so a taskbar this does not describe is left exactly as found.
inline bool CanArrange(Metrics const& metrics) {
    return metrics.valid && !metrics.rotated;
}
//@end

//@part RunsDownSide
// True on a left or right taskbar, where the WIDTH limits how many items fit
// side by side: generated layouts ("auto") fill across it. A written
// arrangement and every nudge are screen-literal on every edge, and top
// behaves exactly like bottom.
inline bool RunsDownSide(Metrics const& metrics) {
    return metrics.orientation == Orientation::Vertical;
}
//@end

inline wchar_t const* OrientationName(Orientation orientation) {
    return orientation == Orientation::Vertical ? L"vertical" : L"horizontal";
}

//@part EdgeName
inline wchar_t const* EdgeName(Edge edge) {
    switch (edge) {
        case Edge::Bottom: return L"bottom";
        case Edge::Top: return L"top";
        case Edge::Left: return L"left";
        case Edge::Right: return L"right";
        default: return L"unknown";
    }
}
//@end

//@part StartEdgeWatch StopEdgeWatch EdgeWatch
// ---- Following a move -------------------------------------------------------
//
// MOVING THE TASKBAR IS A RE-LAYOUT, NOT A REBUILD. The same elements survive
// a move between edges and TrayUI::StartTaskbar never fires, so a mod's
// rebuild hook will not tell it anything changed. Two signals cover every
// move:
//
//   - TaskbarFrame's size, which changes between a horizontal and a side edge
//     and whenever the thickness does (Windows' small and default heights,
//     another mod's side width);
//   - RootGrid's DockingStates group, which changes on EVERY edge change,
//     including bottom <-> top and left <-> right, where the size does not.
//     Those moves still re-template parts of the taskbar (live-observed:
//     the OmniButton sat low after bottom -> top until a re-apply).
//
// The callback runs on the UI thread from inside a layout pass or a state
// change, and both signals usually fire for one move: schedule the re-apply
// (wake the retry), never re-arrange synchronously, and expect a repeat.
//
// The mod owns the EdgeWatch, and must StopEdgeWatch on the UI thread before
// unload: both delegates point into the mod's image.
struct EdgeWatch {
    winrt::weak_ref<FrameworkElement> frame;
    winrt::event_token token{};
    winrt::weak_ref<winrt::Windows::UI::Xaml::VisualStateGroup> docking;
    winrt::event_token dockingToken{};
    bool side = false;
    double thickness = 0.0;
    void (*onChange)() = nullptr;
};

inline void StopEdgeWatch(EdgeWatch& watch) {
    if (watch.token) {
        if (auto frame = watch.frame.get()) frame.SizeChanged(watch.token);
    }
    if (watch.dockingToken) {
        if (auto group = watch.docking.get())
            group.CurrentStateChanged(watch.dockingToken);
    }
    watch.frame = nullptr;
    watch.token = {};
    watch.docking = nullptr;
    watch.dockingToken = {};
}

// Idempotent: watching the same TaskbarFrame again is a no-op, and a rebuilt
// taskbar's new frame replaces the old subscriptions. UI thread only.
inline bool StartEdgeWatch(EdgeWatch& watch, FrameworkElement const& taskbarRoot,
                           void (*onChange)()) {
    auto frame = FindTaskbarChild(taskbarRoot, L"TaskbarFrame", 2);
    if (!frame) return false;
    if (watch.token && watch.frame.get() == frame) return true;
    StopEdgeWatch(watch);
    watch.frame = winrt::make_weak(frame);
    watch.side = frame.ActualHeight() > frame.ActualWidth();
    watch.thickness = watch.side ? frame.ActualWidth() : frame.ActualHeight();
    watch.onChange = onChange;
    EdgeWatch* target = &watch;
    watch.token = frame.SizeChanged(
        [target](winrt::Windows::Foundation::IInspectable const&,
                 winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args) {
            auto size = args.NewSize();
            bool side = size.Height > size.Width;
            double thickness = side ? size.Width : size.Height;
            // Content-sized themes change length as task buttons come and go.
            // Only orientation and thickness require a new arrangement.
            if (side == target->side &&
                std::abs(thickness - target->thickness) < 0.5)
                return;
            target->side = side;
            target->thickness = thickness;
            if (target->onChange) target->onChange();
        });

    // Absent on builds without the native position setting; the size watch
    // alone is then all there is, and all that is needed.
    if (auto rootGrid = FindTaskbarChild(frame, L"RootGrid", 2)) {
        for (auto const& group :
             VisualStateManager::GetVisualStateGroups(rootGrid)) {
            if (group.Name() != L"DockingStates") continue;
            watch.docking = winrt::make_weak(group);
            watch.dockingToken = group.CurrentStateChanged(
                [target](winrt::Windows::Foundation::IInspectable const&,
                         winrt::Windows::UI::Xaml::VisualStateChangedEventArgs
                             const&) {
                    if (target->onChange) target->onChange();
                });
            break;
        }
    }
    return true;
}
//@end
