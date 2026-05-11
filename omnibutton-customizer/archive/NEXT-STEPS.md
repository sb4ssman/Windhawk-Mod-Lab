# Next Steps: Implementing the Vertical OmniButton Mod

## Quick Start Summary

You now have a complete skeleton implementation of the Windhawk mod for vertically arranging system tray icons. Here's what to do next.

## What You Have

### Files Created
1. **[vertical-omnibutton.wh.cpp](vertical-omnibutton.wh.cpp)** - Main mod file (skeleton implementation)
2. **[README.md](README.md)** - Comprehensive project documentation
3. **[DEVELOPMENT.md](DEVELOPMENT.md)** - Detailed technical development notes
4. **NEXT-STEPS.md** - This file (action plan)

### Current Status
- Metadata blocks defined with proper mod info
- Settings framework implemented (enable toggle, spacing, debug logging)
- Helper functions for XAML element traversal written
- Core orientation logic skeleton created
- Logging infrastructure in place

### What's Missing
The mod will not work yet because it lacks the critical function hooking implementation that actually intercepts the taskbar's XAML creation.

## Immediate Next Actions

### Option 1: Research Hook Targets (Recommended First Step)

**Goal**: Identify which function(s) to hook in `Taskbar.View.dll`

**Steps**:
1. Download and run [UWPSpy](https://www.microsoft.com/store/productId/9N1W692FV4S1)
2. Open the taskbar in UWPSpy
3. Navigate to `SystemTray.OmniButton#ControlCenterButton`
4. Examine the element tree to find the StackPanel
5. Right-click elements to view their properties and parent objects
6. Note any class names, method names, or creation patterns

**What to Look For**:
- Constructor functions (e.g., `OmniButton::OmniButton()`)
- Initialization methods (e.g., `InitializeComponent()`)
- Layout functions (e.g., `Measure()`, `Arrange()`)
- Property setters (e.g., `SetOrientation()`)

### Option 2: Extract Debug Symbols

**Goal**: Get function signatures from Windows debug symbols

**Steps**:
1. Download Windows SDK symbols for your Windows 11 build
2. Extract `Taskbar.View.pdb` symbols
3. Search for functions containing:
   - "OmniButton"
   - "ControlCenter"
   - "SystemTray"
   - "StackPanel"
4. Identify functions that create or configure these elements

**Tools**:
- Dia2Dump (Windows SDK tool)
- IDA Pro (commercial, but has free version)
- x64dbg with symbol loading

### Option 3: Study Reference Implementation

**Goal**: Understand how similar mods hook and modify elements

**Focus on these mods**:
1. **taskbar-notification-icon-spacing.wh.cpp** - Similar tray manipulation
2. **taskbar-vertical.wh.cpp** - Orientation changes
3. **taskbar-clock-customization.wh.cpp** - Element property modification

**Key patterns to extract**:
```cpp
// Symbol hook declaration
WindhawkUtils::SYMBOL_HOOK hooks[] = {
    {
        {LR"(signature string here)"},
        &Hook_Function,
        (void**)&Original_Function,
        false
    }
};

// Hook installation
HookSymbols(Wh_GetModuleHandle(L"taskbar.view.dll"),
            hooks, ARRAYSIZE(hooks));
```

## Implementation Roadmap

### Phase 1: Basic Hook Implementation
**Estimated Time**: 2-4 hours

1. Add symbol hook infrastructure to the mod
2. Hook into a known function (e.g., `IconView::IconView`)
3. Add logging to confirm hook is called
4. Verify compilation and loading in Windhawk

**Success Criteria**: Seeing log messages when taskbar initializes

### Phase 2: Element Access
**Estimated Time**: 2-4 hours

1. From hooked function, access XAML content
2. Navigate to OmniButton element
3. Find StackPanel within element tree
4. Log StackPanel properties to confirm access

**Success Criteria**: Logging shows StackPanel orientation value

### Phase 3: Orientation Modification
**Estimated Time**: 1-2 hours

1. Apply vertical orientation in hook
2. Set spacing value from settings
3. Test visual change in taskbar
4. Handle edge cases (null checks, thread safety)

**Success Criteria**: Icons actually stack vertically

### Phase 4: Polish & Testing
**Estimated Time**: 2-4 hours

1. Implement settings hot-reload
2. Add proper cleanup on unload
3. Test on different Windows 11 versions
4. Test with other taskbar mods
5. Document any issues or limitations

**Success Criteria**: Mod works reliably across scenarios

## Testing Checklist

### Basic Functionality
- [ ] Mod compiles without errors
- [ ] Mod loads in Windhawk
- [ ] Logging appears in DebugView
- [ ] Icons change to vertical arrangement
- [ ] Icons remain clickable
- [ ] Settings toggle works

### Edge Cases
- [ ] Behavior when enabling/disabling mod
- [ ] Behavior during explorer.exe restart
- [ ] Behavior with no system tray icons
- [ ] Behavior with all system tray icons
- [ ] Spacing adjustment from 0-16 pixels
- [ ] Unload/reload cycles

### Compatibility
- [ ] Works on Windows 11 22H2
- [ ] Works on Windows 11 23H2
- [ ] Works on Windows 11 24H2
- [ ] Works with "Taskbar height and icon size"
- [ ] Works with "Taskbar notification icon spacing"
- [ ] Works with "Vertical Taskbar for Windows 11"

### Performance
- [ ] No noticeable CPU usage
- [ ] No memory leaks over time
- [ ] No explorer.exe crashes
- [ ] Fast settings response time

## Common Pitfalls to Avoid

### 1. Cross-Thread XAML Access
**Problem**: Modifying XAML from wrong thread crashes explorer.exe

**Solution**: Use `RunFromWindowThread()` pattern:
```cpp
RunFromWindowThread(hWnd, [element]() {
    // Modify XAML properties here
    stackPanel.Orientation(Vertical);
});
```

### 2. Stale Element References
**Problem**: Keeping references to XAML elements that get destroyed

**Solution**: Re-query elements each time, don't cache long-term

### 3. Hook Signature Mismatch
**Problem**: Symbol string doesn't match actual function

**Solution**: Test with multiple signature variations, check Windows build

### 4. Missing Error Handling
**Problem**: Exceptions crash explorer.exe

**Solution**: Wrap all XAML operations in try-catch blocks:
```cpp
try {
    // XAML operations
} catch (const winrt::hresult_error& ex) {
    Wh_Log(L"Error: 0x%08X", ex.code());
} catch (...) {
    Wh_Log(L"Unknown error");
}
```

## Resources for Next Steps

### Essential Tools
- **UWPSpy**: [Microsoft Store](https://www.microsoft.com/store/productId/9N1W692FV4S1)
- **DebugView**: [Sysinternals](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview)
- **Windhawk**: [windhawk.net](https://windhawk.net/)

### Documentation
- [Windhawk Mod Creation](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windows 11 Taskbar Styling Guide](https://github.com/ramensoftware/windows-11-taskbar-styling-guide)
- [C++/WinRT Docs](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)

### Reference Mods Source Code
- [taskbar-notification-icon-spacing.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp)
- [taskbar-vertical.wh.cpp](https://github.com/m417z/my-windhawk-mods/blob/main/mods/taskbar-vertical.wh.cpp)
- [taskbar-clock-customization.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp)

### Community Support
- [Windhawk Discussions](https://github.com/ramensoftware/windhawk-mods/discussions)
- [Ramen Software Blog](https://ramensoftware.com)

## Decision Points

### Approach A: Hook Constructor
**Pros**: Catch element creation early, clean modification point

**Cons**: May not have access to fully initialized element tree

**Best for**: If you can identify OmniButton or StackPanel constructor

### Approach B: Hook Layout Function
**Pros**: Element tree fully built, reliable access to properties

**Cons**: Called frequently (performance concern), may need filtering

**Best for**: If constructor hooking doesn't work

### Approach C: Hook Update Function
**Pros**: Catches dynamic changes, can re-apply on icon updates

**Cons**: Most complex, highest overhead

**Best for**: If icons change dynamically and need re-orientation

## Success Metrics

### Minimum Viable Product (MVP)
- Icons stack vertically
- Basic enable/disable toggle
- Works on at least one Windows 11 version

### Full Release Quality
- All settings functional
- Works on all Windows 11 22H2+ versions
- Proper error handling and logging
- Compatible with major taskbar mods
- Clean unload/reload behavior
- Documented edge cases

### Bonus Features (Future)
- Horizontal/vertical auto-detection based on taskbar position
- Per-icon visibility toggles
- Icon reordering options
- Custom icon sizes per-icon
- Integration with Windows 11 Taskbar Styler

## Getting Help

### If You're Stuck

1. **Enable debug logging** and review DebugView output
2. **Check Windhawk logs** for compilation or loading errors
3. **Compare with reference mods** for similar patterns
4. **Test incrementally** - add one feature at a time
5. **Ask the community** in Windhawk discussions

### Where to Ask

- [Windhawk Mods Discussions](https://github.com/ramensoftware/windhawk-mods/discussions)
- [Project Issues](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton/issues)

### What to Include
- Windows 11 version and build number
- Windhawk version
- Debug log output (with `debugLogging: true`)
- Description of expected vs actual behavior
- Other active taskbar mods

## Timeline Estimate

**Conservative**: 8-16 hours total development time
- Research: 2-4 hours
- Implementation: 4-8 hours
- Testing: 2-4 hours

**Optimistic**: 4-8 hours (if you find the right hook quickly)

**Realistic for first-time mod developer**: 12-20 hours

## Final Notes

This is a **well-scoped project** with:
- Clear objective (vertical orientation)
- Good reference implementations (similar mods exist)
- Well-defined target (specific XAML element)
- Manageable complexity (single property change)

The hardest part is finding the right hook target. Once you have that, the rest should fall into place quickly.

Good luck, and feel free to iterate on the implementation as you learn more about the taskbar's internal structure!

---

**Last Updated**: 2025-12-05
**Status**: Ready for implementation
**Next Action**: Run UWPSpy to identify hook targets (see Option 1 above)
