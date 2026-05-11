# Development Notes: Vertical OmniButton Mod

## Current Status

**Phase**: Initial skeleton implementation complete

The basic mod structure has been created with:
- Metadata and settings blocks
- C++ framework with WinRT XAML includes
- Helper functions for element traversal
- Core orientation manipulation logic
- Logging infrastructure

## Next Steps: Function Hooking Implementation

The current implementation is **incomplete** - it needs proper function hooking into `Taskbar.View.dll` to actually intercept and modify the OmniButton's StackPanel.

### Required Implementation Pattern

Based on the `taskbar-notification-icon-spacing.wh.cpp` mod, we need to:

1. **Hook into Taskbar.View.dll functions**
   - Use `Wh_SetFunctionHook()` with appropriate signatures
   - Target functions that create or initialize the OmniButton
   - Possible targets:
     - `SystemTray.OmniButton` constructor
     - `IconView::IconView()` constructor (if applicable)
     - System tray initialization routines

2. **Use symbol-based hooking**
   ```cpp
   WindhawkUtils::SYMBOL_HOOK hookName[] = {
       {
           {LR"(public: __cdecl OmniButton::OmniButton(void))"},
           &OmniButton_Constructor_Hook,
           &OmniButton_Constructor_Original
       }
   };
   ```

3. **Access XAML content correctly**
   - Hook needs to capture the actual XAML island content
   - Use proper thread marshaling with `RunFromWindowThread()`
   - Access element tree from within hooked functions

4. **Handle settings changes**
   - Implement proper reapplication when settings change
   - Store references to modified elements for cleanup

## Research Needed

### Find the Right Hook Target

Use one of these methods:

1. **UWPSpy inspection**:
   - Run UWPSpy while taskbar is visible
   - Navigate to the OmniButton element
   - Identify parent objects and initialization chain

2. **Debug symbols analysis**:
   - Extract function signatures from `Taskbar.View.pdb`
   - Look for OmniButton, ControlCenterButton, or SystemTray initialization
   - Match against patterns in Windows symbols

3. **Study existing mods**:
   - `taskbar-notification-icon-spacing.wh.cpp` - hooks IconView constructor
   - `taskbar-vertical.wh.cpp` - hooks various sizing/positioning functions
   - `taskbar-clock-customization.wh.cpp` - hooks ClockButton methods

### Key Questions to Answer

1. **When is the OmniButton StackPanel created?**
   - During explorer.exe startup?
   - On-demand when system tray initializes?
   - When individual icons register?

2. **What function sets the StackPanel orientation?**
   - Constructor parameter?
   - Property setter after construction?
   - Layout calculation function?

3. **How to persist changes across updates?**
   - Does the element get recreated on icon changes?
   - Need to re-hook on each recreation?
   - Store weak references to avoid leaks?

## Testing Strategy

### Phase 1: Verify Hook Works
- Add hooks with logging only
- Confirm functions are being called
- Log element access attempts

### Phase 2: Modify Properties
- Apply orientation change in hook
- Test with various icon combinations
- Check persistence across changes

### Phase 3: Settings Integration
- Test enable/disable toggle
- Verify spacing adjustments work
- Test unload/reload cycle

### Phase 4: Edge Cases
- Test with different Windows 11 builds (22H2, 23H2, 24H2)
- Test with other taskbar mods active
- Test rapid settings changes
- Test explorer.exe restart scenarios

## Known Technical Challenges

### 1. XAML Thread Access
- Must modify UI from correct thread
- Use `RunFromWindowThread()` pattern
- Handle async timing issues

### 2. Element Lifecycle
- StackPanel may be recreated dynamically
- Need to detect and reapply changes
- Avoid holding stale references

### 3. Windows Version Differences
- Different builds may have different structures
- May need conditional logic per version
- Test on multiple Windows 11 versions

### 4. Compatibility with Other Mods
- Don't interfere with Taskbar height mods
- Don't conflict with notification icon spacing
- Test combined scenarios

## Reference Code Patterns

### Pattern 1: Symbol Hook Setup (from taskbar-notification-icon-spacing)
```cpp
#include <regex>

namespace {
    std::wregex g_filterRegex;

    struct {
        int notificationIconWidth;
        int iconSpacing;
    } g_settings;

    // Hook target
    using IconView_IconView_t = void(WINAPI*)(void* pThis, void* param1);
    IconView_IconView_t IconView_IconView_Original;

    void WINAPI IconView_IconView_Hook(void* pThis, void* param1) {
        Wh_Log(L"IconView::IconView called");
        IconView_IconView_Original(pThis, param1);

        // Modification logic here
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L"Init");

    LoadSettings();

    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(public: __cdecl IconView::IconView(void))"},
            &IconView_IconView_Hook,
            (void**)&IconView_IconView_Original,
            false
        }
    };

    return HookSymbols(Wh_GetModuleHandle(L"taskbar.view.dll"),
                       hooks, ARRAYSIZE(hooks));
}
```

### Pattern 2: Element Tree Traversal (from taskbar-vertical)
```cpp
FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    int childrenCount = Media::VisualTreeHelper::GetChildrenCount(element);

    for (int i = 0; i < childrenCount; i++) {
        auto child = Media::VisualTreeHelper::GetChild(element, i)
            .try_as<FrameworkElement>();
        if (!child) continue;

        if (child.Name() == name) {
            return child;
        }

        auto foundChild = FindChildByName(child, name);
        if (foundChild) return foundChild;
    }

    return nullptr;
}
```

### Pattern 3: Orientation Modification (from taskbar-vertical)
```cpp
auto stackPanel = element.try_as<StackPanel>();
if (stackPanel) {
    stackPanel.Orientation(
        g_unloading ? Controls::Orientation::Horizontal
                    : Controls::Orientation::Vertical
    );
}
```

## Development Environment

### Tools Needed
1. **Windhawk** - Latest version (1.4+)
2. **Visual Studio 2022** - For debugging (optional)
3. **UWPSpy** - For XAML tree inspection
4. **DebugView** - For OutputDebugString logging
5. **Windows SDK** - For symbols and headers

### Build Process
Windhawk compiles mods automatically:
- Edit `.wh.cpp` file
- Save in Windhawk's mod directory
- Windhawk recompiles and reloads
- Test changes immediately

### Debugging Tips
1. Enable debug logging in settings
2. Use OutputDebugStringW() extensively
3. Watch DebugView for log output
4. Test with Task Manager > Restart Explorer
5. Check Windhawk logs for compilation errors

## Resources

### Documentation
- [Windhawk Mod Creation Wiki](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windows 11 Taskbar Styling Guide](https://github.com/ramensoftware/windows-11-taskbar-styling-guide)
- [C++/WinRT Documentation](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)

### Example Mods
- [taskbar-notification-icon-spacing.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp)
- [taskbar-vertical.wh.cpp](https://github.com/m417z/my-windhawk-mods/blob/main/mods/taskbar-vertical.wh.cpp)
- [taskbar-clock-customization.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp)

### Community
- [Windhawk Mods Discussion](https://github.com/ramensoftware/windhawk-mods/discussions)
- [Ramen Software Blog](https://ramensoftware.com)

## Progress Checklist

- [x] Project structure created
- [x] Metadata blocks defined
- [x] Settings framework implemented
- [x] Helper functions written
- [x] Core logic skeleton created
- [ ] Function hook targets identified (NEEDED)
- [ ] Symbol hooks implemented (NEEDED)
- [ ] XAML content access working (NEEDED)
- [ ] Orientation modification functional (NEEDED)
- [ ] Spacing adjustment working
- [ ] Settings hot-reload working
- [ ] Unload/cleanup working
- [ ] Tested on Windows 11 22H2
- [ ] Tested on Windows 11 23H2
- [ ] Tested on Windows 11 24H2
- [ ] Tested with other taskbar mods
- [ ] Documentation finalized

## Next Immediate Action

**PRIMARY TASK**: Use UWPSpy or debug symbols to identify the exact function(s) that:
1. Create the OmniButton's StackPanel
2. Set its Orientation property
3. Can be hooked to intercept and modify the orientation

Once identified, implement the symbol hook using the patterns above.
