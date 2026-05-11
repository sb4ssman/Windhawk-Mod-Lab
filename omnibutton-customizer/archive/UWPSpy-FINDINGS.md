# UWPSpy Investigation Findings

## Date: 2025-12-05

## What We Discovered

Using UWPSpy, we successfully located the OmniButton structure in the Windows 11 taskbar.

### Element Path Confirmed

```
SystemTray.OmniButton - ControlCenterButton
  └─ Windows.UI.Xaml.Controls.Grid
     └─ Windows.UI.Xaml.Controls.ContentPresenter
        └─ Windows.UI.Xaml.Controls.ItemsPresenter
           └─ Windows.UI.Xaml.Controls.StackPanel ← TARGET ELEMENT
              ├─ Properties:
              │  - Class: Windows.UI.Xaml.Controls.StackPanel
              │  - Orientation: 1 (Vertical - surprisingly!)
              │  - ItemsHost: 1
              │
              └─ Children: ContentPresenter elements
                 └─ Each contains SystemTray.IconView
                    (wifi, sound, battery icons)
```

### Key Findings

1. **StackPanel Orientation is Already Vertical (1)**
   - UWPSpy shows `Orientation: 1` (Vertical)
   - BUT the taskbar displays icons horizontally
   - This suggests the StackPanel orientation property is NOT controlling the visual layout

2. **Icons Are Still Horizontal in Taskbar**
   - Despite StackPanel being "Vertical", icons appear side-by-side
   - Each icon is wrapped in a ContentPresenter
   - IconView elements likely have their own positioning logic

3. **Individual IconView Elements**
   - SystemTray.IconView class for each icon
   - Nested in: ContentPresenter > SystemTray.IconView > Grid > Grid
   - Each IconView might be using absolute positioning or transforms

### Implications for Implementation

Since changing StackPanel.Orientation alone won't work (it's already Vertical!), we need to:

1. **Use TranslateTransform** (like taskbar-notification-icon-spacing mod)
   - Apply Y-axis transforms to each IconView
   - Calculate offsets based on icon size + spacing
   - Center the stack vertically

2. **Target IconView Elements Directly**
   - Hook into IconView constructor or initialization
   - Apply transforms when icons are created
   - Similar pattern to taskbar-notification-icon-spacing.wh.cpp

3. **Handle Multiple Icons**
   - Typically 3 icons: wifi, sound, battery
   - Need to enumerate ContentPresenter children
   - Apply increasing Y offsets (index * (iconSize + spacing))

## Technical Approach

### Based on taskbar-notification-icon-spacing.wh.cpp Pattern

The working approach uses:

```cpp
// Calculate vertical offset for each icon
double itemHeight = iconSize + spacing;
double yOffset = itemHeight * iconIndex;

// Center the stack
int iconCount = 3;
double totalHeight = itemHeight * (iconCount - 1);
yOffset -= totalHeight / 2;

// Apply transform
Media::TranslateTransform transform;
transform.Y(yOffset);
transform.X(0);
iconView.RenderTransform(transform);
```

### Required Hooks

We need to hook into one of these:
1. **IconView constructor** - When icons are created
2. **OmniButton initialization** - When the button is set up
3. **StackPanel layout methods** - When positioning happens

### Reference from taskbar-notification-icon-spacing

That mod hooks `IconView::IconView` constructor and applies transforms there:

```cpp
WindhawkUtils::SYMBOL_HOOK hooks[] = {
    {
        {LR"(public: __cdecl IconView::IconView(void))"},
        &IconView_IconView_Hook,
        (void**)&IconView_IconView_Original,
        false
    }
};
```

## Next Steps

### 1. Extract Function Signatures

Need to find the signature for:
- `SystemTray::OmniButton` constructor
- `IconView::IconView` constructor (if different from notification icons)
- StackPanel layout methods

### 2. Implement Hook

Follow the pattern from taskbar-notification-icon-spacing:
- Hook the constructor
- Identify which IconView belongs to OmniButton (vs regular tray)
- Apply TranslateTransform with calculated offsets

### 3. Test and Iterate

- Load mod in Windhawk
- Enable debug logging
- Watch for hook calls in DebugView
- Adjust offsets if needed

## Why StackPanel.Orientation Alone Won't Work

The UWPSpy finding that Orientation is already "1" (Vertical) but icons display horizontally suggests:

1. **IconView has internal layout logic** that overrides StackPanel
2. **Transforms or absolute positioning** are applied elsewhere
3. **Windows might be using a different layout mechanism** for these specific icons

This is why we must use **TranslateTransform** to forcibly reposition the icons, rather than relying on StackPanel.Orientation.

## Current Taskbar Configuration

User has a **double-height taskbar** setup using Windhawk mods:
- Taskbar height increased for two rows
- Notification area icons arranged in a grid
- OmniButton icons (wifi/sound/battery) currently horizontal
- Goal: Make OmniButton icons vertical to match the grid aesthetic

## Resources Used

- **UWPSpy**: For live XAML inspection
- **taskbar-notification-icon-spacing.wh.cpp**: Reference for TranslateTransform technique
- **taskbar-vertical.wh.cpp**: Reference for orientation changes
- **Windows 11 Taskbar Styling Guide**: For element paths

## Confidence Level

**HIGH** - The TranslateTransform approach is proven to work in taskbar-notification-icon-spacing for similar icon repositioning. The main challenge is hooking into the right function to access the OmniButton's IconView elements specifically.

---

**Updated**: 2025-12-05
**Status**: Implementation framework complete, awaiting function hook integration
