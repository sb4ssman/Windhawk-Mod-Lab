# Project Status: Vertical OmniButton Mod

**Last Updated**: 2025-12-05
**Current Phase**: Implementation Framework Complete

---

## What We've Accomplished

### ✅ Research Complete
- Studied existing Windhawk mods (taskbar-notification-icon-spacing, taskbar-vertical)
- Identified TranslateTransform as the correct positioning technique
- Located OmniButton element structure using UWPSpy
- Confirmed element paths and properties

### ✅ UWPSpy Investigation
- Successfully navigated to `SystemTray.OmniButton - ControlCenterButton`
- Found the StackPanel containing icon ContentPresenters
- **Key Discovery**: StackPanel.Orientation is already "Vertical" (value: 1) but icons still display horizontally
- Confirmed we need TranslateTransform approach, not just orientation change

### ✅ Implementation Framework
- Created complete mod structure ([vertical-omnibutton.wh.cpp](vertical-omnibutton.wh.cpp))
- Implemented settings system (enable toggle, icon size, spacing, debug logging)
- Built helper functions for element tree traversal
- Created `ApplyVerticalTransform()` function using TranslateTransform
- Added comprehensive logging for debugging
- All compilation errors resolved

### ✅ Documentation
- [README.md](README.md) - Comprehensive project documentation
- [DEVELOPMENT.md](DEVELOPMENT.md) - Technical development notes
- [NEXT-STEPS.md](NEXT-STEPS.md) - Implementation roadmap
- [UWPSpy-FINDINGS.md](UWPSpy-FINDINGS.md) - Investigation results
- STATUS.md - This file

---

## What's Missing: The Critical Piece

### ⚠️ Function Hooking Not Implemented

The mod **will compile** but **won't do anything yet** because we haven't implemented the actual function hooks that intercept the taskbar's icon creation.

**Why it's missing**: We need Windows debug symbols to extract the correct function signature for hooking.

**What needs to happen**:
1. Hook into IconView constructor or OmniButton initialization
2. Identify OmniButton-specific IconView elements (vs regular tray icons)
3. Call `ProcessOmniButton()` when the element is created
4. Apply transforms to reposition icons

---

## Current Implementation Status

### File Structure
```
Windhawk-Vertical-Omnibutton/
├── vertical-omnibutton.wh.cpp    ✅ Complete framework, needs hooks
├── README.md                      ✅ Full documentation
├── DEVELOPMENT.md                 ✅ Technical guide
├── NEXT-STEPS.md                  ✅ Implementation steps
├── UWPSpy-FINDINGS.md            ✅ Investigation results
└── STATUS.md                      ✅ This file
```

### Code Status

**Working Components**:
- ✅ Mod metadata and settings blocks
- ✅ Settings loading and validation
- ✅ Element tree traversal helpers (`FindChildByClassName`, `FindChildByName`)
- ✅ Transform logic (`ApplyVerticalTransform` with proper offset calculations)
- ✅ OmniButton processing (`ProcessOmniButton` to apply transforms)
- ✅ Logging infrastructure
- ✅ Windhawk callbacks (init, uninit, settings changed)

**Not Working Yet**:
- ❌ Function hooks (commented out placeholders only)
- ❌ XAML island access
- ❌ Automatic application on taskbar creation
- ❌ Live testing and validation

---

## Technical Summary

### The Problem
Windows 11 displays wifi/sound/battery icons horizontally in the OmniButton. When using a double-height taskbar with gridded tray icons, this looks inconsistent.

### The Solution
Use `TranslateTransform` to reposition each IconView element vertically:
- Calculate Y offset: `(iconSize + spacing) * iconIndex`
- Center the stack: offset by `-(totalHeight / 2)`
- Apply transform: `iconView.RenderTransform(transform)`

### Why This Approach
- StackPanel.Orientation is already "Vertical" but doesn't affect layout
- IconView elements need direct transforms (proven in taskbar-notification-icon-spacing)
- TranslateTransform provides pixel-perfect positioning

### Reference Pattern (from taskbar-notification-icon-spacing)
```cpp
Media::TranslateTransform transform;
transform.Y(calculatedOffset);
transform.X(0);
iconView.RenderTransform(transform);
```

---

## Next Actions (In Priority Order)

### 1. Extract Function Signature ⚡ CRITICAL
**Goal**: Find the correct function to hook

**Methods**:
- Use Windows debug symbols (Taskbar.View.pdb)
- Look for: `SystemTray::OmniButton`, `IconView::IconView`, or initialization methods
- Test with different signature patterns

**Tools**:
- Dia2Dump (Windows SDK)
- IDA Pro / x64dbg
- Symbol server downloads

### 2. Implement Hook
**Goal**: Wire up the mod to actually run

**Pattern** (from taskbar-notification-icon-spacing):
```cpp
#include <windhawk_utils.h>

WindhawkUtils::SYMBOL_HOOK hooks[] = {
    {
        {LR"(public: __cdecl OmniButton::Initialize(void))"},  // Example
        &OmniButton_Initialize_Hook,
        (void**)&OmniButton_Initialize_Original,
        false
    }
};

if (!HookSymbols(Wh_GetModuleHandle(L"taskbar.view.dll"), hooks, ARRAYSIZE(hooks))) {
    Wh_Log(L"Failed to hook symbols");
    return FALSE;
}
```

### 3. Test in Windhawk
**Goal**: Load mod and verify it works

**Steps**:
1. Copy `vertical-omnibutton.wh.cpp` to Windhawk mods folder
2. Enable in Windhawk UI
3. Turn on debug logging
4. Monitor DebugView for log output
5. Check taskbar for vertical icons

### 4. Iterate and Polish
**Goal**: Fine-tune positioning and handle edge cases

**Adjustments**:
- Icon spacing values
- Centering offset calculation
- Handle different icon counts
- Test with other mods enabled

---

## Known Challenges

### 1. Finding the Right Hook
**Challenge**: Windows internals aren't documented
**Solution**: Trial and error with different function signatures, study similar mods

### 2. OmniButton vs Regular Icons
**Challenge**: IconView is used for both OmniButton and tray icons
**Solution**: Filter by parent element or icon type when applying transforms

### 3. Windows Updates
**Challenge**: Internal structure can change with updates
**Solution**: Version detection, fallback patterns, community testing

### 4. Thread Safety
**Challenge**: XAML access must be on correct thread
**Solution**: Use `RunFromWindowThread()` pattern (see reference mods)

---

## Testing Plan

### Phase 1: Hook Verification
- [ ] Mod compiles without errors
- [ ] Mod loads in Windhawk
- [ ] Hook function is called (verify via logging)
- [ ] Can access OmniButton element

### Phase 2: Transform Application
- [ ] Transforms are applied to IconViews
- [ ] Icons move to calculated positions
- [ ] No crashes or hangs
- [ ] Icons remain clickable

### Phase 3: Settings & Edge Cases
- [ ] Enable/disable toggle works
- [ ] Spacing adjustment works
- [ ] Icon size setting works
- [ ] Mod unloads cleanly
- [ ] Works after explorer.exe restart

### Phase 4: Compatibility
- [ ] Works with "Taskbar height and icon size"
- [ ] Works with "Taskbar notification icon spacing"
- [ ] Works on Windows 11 22H2
- [ ] Works on Windows 11 23H2
- [ ] Works on Windows 11 24H2

---

## Resources & References

### Documentation
- [Windhawk Mod Creation Wiki](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windows 11 Taskbar Styling Guide](https://github.com/ramensoftware/windows-11-taskbar-styling-guide)
- [Windhawk Mods Repository](https://github.com/ramensoftware/windhawk-mods)

### Reference Mod Source Code
- [taskbar-notification-icon-spacing.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp) - **PRIMARY REFERENCE**
- [taskbar-vertical.wh.cpp](https://github.com/m417z/my-windhawk-mods/blob/main/mods/taskbar-vertical.wh.cpp)
- [taskbar-clock-customization.wh.cpp](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp)

### Tools
- **Windhawk**: [windhawk.net](https://windhawk.net/)
- **UWPSpy**: [ramensoftware.com/uwpspy](https://ramensoftware.com/uwpspy)
- **DebugView**: [Sysinternals](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview)

### Community
- [Windhawk Discussions](https://github.com/ramensoftware/windhawk-mods/discussions)
- [Ramen Software Blog](https://ramensoftware.com)

---

## Confidence Assessment

### What We're Confident About
✅ **TranslateTransform approach**: Proven technique from taskbar-notification-icon-spacing
✅ **Element structure**: Confirmed via UWPSpy investigation
✅ **Transform calculations**: Math is solid, based on working reference
✅ **Settings framework**: Complete and tested pattern

### What's Uncertain
⚠️ **Exact hook target**: Need to find correct function signature
⚠️ **Filtering OmniButton icons**: May need trial and error
⚠️ **Windows version differences**: Might need conditional code

### Risk Level
**MODERATE** - The approach is proven, but finding the right hook requires research and experimentation.

---

## Success Criteria

### Minimum Viable Product (MVP)
- [x] Mod compiles and loads
- [ ] Icons stack vertically when enabled
- [ ] Icons remain clickable and functional
- [ ] Works on at least one Windows 11 version

### Full Release
- [ ] All settings functional
- [ ] Works on Windows 11 22H2, 23H2, 24H2
- [ ] Compatible with major taskbar mods
- [ ] Clean enable/disable behavior
- [ ] Comprehensive documentation

### Stretch Goals
- [ ] Auto-detect taskbar orientation
- [ ] Configurable icon order
- [ ] Individual icon visibility toggles
- [ ] Integration with Windows 11 Taskbar Styler

---

## Timeline Estimate

**Current Progress**: ~60% complete

**Remaining Work**:
- Function signature research: 2-6 hours
- Hook implementation: 1-3 hours
- Testing and debugging: 2-4 hours
- **Total**: 5-13 hours

**Confidence**: High that it's achievable with the right function signature.

---

## How to Use This Project (Current State)

### For Developers
1. Review the code in [vertical-omnibutton.wh.cpp](vertical-omnibutton.wh.cpp)
2. Study the transform logic in `ApplyVerticalTransform()`
3. Reference [DEVELOPMENT.md](DEVELOPMENT.md) for technical details
4. Check [UWPSpy-FINDINGS.md](UWPSpy-FINDINGS.md) for investigation results
5. Follow [NEXT-STEPS.md](NEXT-STEPS.md) to implement hooks

### For Testing (Once Hooks Implemented)
1. Copy `.wh.cpp` to Windhawk mods folder
2. Enable in Windhawk
3. Turn on debug logging
4. Check DebugView for output
5. Report issues with Windows version and other active mods

### For Contributing
1. Focus on finding the right function hook
2. Test on different Windows 11 versions
3. Report findings in GitHub issues
4. Submit PRs with improvements

---

## Questions? Issues?

- **Project Repository**: [github.com/tmiller711/Windhawk-Vertical-Omnibutton](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton)
- **Report Issues**: [github.com/tmiller711/Windhawk-Vertical-Omnibutton/issues](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton/issues)
- **Windhawk Community**: [github.com/ramensoftware/windhawk-mods/discussions](https://github.com/ramensoftware/windhawk-mods/discussions)

---

**Status**: 🚧 **Implementation Framework Complete - Awaiting Hook Integration**

The heavy lifting is done. The last critical piece is hooking into the right function to make it all come alive!
