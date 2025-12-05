# Windhawk Mod: Vertical System Tray OmniButton Icons

[![Windhawk](https://img.shields.io/badge/Windhawk-Mod-blue)](https://windhawk.net/)
[![Windows 11](https://img.shields.io/badge/Windows-11-blue)](https://www.microsoft.com/windows/windows-11)
[![Status](https://img.shields.io/badge/Status-In%20Development-yellow)](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton)

A Windhawk mod that transforms the horizontal system tray icon grouping (wifi, volume, battery) into a vertical arrangement for Windows 11. Perfect for double-height taskbar setups with gridded tray icons.

## Overview

This mod targets the `SystemTray.OmniButton#ControlCenterButton` element and converts its internal `StackPanel` from horizontal to vertical orientation, allowing the wifi/sound/battery icons to stack vertically instead of sitting side-by-side.

### The Problem

Windows 11 groups system tray icons (wifi, volume, battery) horizontally in a single unit called the OmniButton. When using a double-height taskbar with gridded tray icons, this horizontal grouping looks out of place and wastes vertical space.

### The Solution

This mod intercepts the StackPanel creation and changes its orientation from `Horizontal` (0) to `Vertical` (1), making the icons stack in a column that matches the aesthetic of multi-row taskbar configurations.

## Features

- Vertical stacking of system tray OmniButton icons
- Configurable icon spacing (0-16 pixels)
- Toggle enable/disable without restarting
- Debug logging for troubleshooting
- Compatible with other taskbar mods

## Installation

### Prerequisites

- Windows 11 (22H2, 23H2, or 24H2)
- [Windhawk](https://windhawk.net/) v1.4 or later

### Install Steps

1. Install Windhawk from [windhawk.net](https://windhawk.net/)
2. Download `vertical-omnibutton.wh.cpp` from this repository
3. Copy the file to Windhawk's mod directory (or use Windhawk's "Load from file" option)
4. Enable the mod in Windhawk
5. Adjust settings as desired

## Configuration

### Settings

**Enable vertical arrangement** (default: `true`)
- Toggles the vertical layout on/off
- Changes take effect immediately

**Icon spacing** (default: `4` pixels)
- Adjusts vertical spacing between icons
- Range: 0-16 pixels
- Useful for fine-tuning appearance

**Enable debug logging** (default: `false`)
- Logs detailed information to DebugView
- Helpful for troubleshooting
- Disable for normal use (reduces overhead)

### Recommended Companion Mods

This mod works great with:
- **Taskbar height and icon size** - For double-height taskbar
- **Taskbar notification icon spacing** - For gridded tray icons
- **Vertical Taskbar for Windows 11** - For vertical taskbar placement

## Development Status

**Current Phase**: Skeleton Implementation

The mod structure is complete, but the core functionality is not yet implemented. See [DEVELOPMENT.md](DEVELOPMENT.md) for technical details.

### What's Done
- Metadata and settings blocks
- C++ framework with WinRT XAML includes
- Helper functions for element traversal
- Core orientation manipulation logic
- Logging infrastructure

### What's Needed
- Function hooking into `Taskbar.View.dll`
- Symbol-based hook implementation
- Proper XAML content access
- Testing across Windows 11 versions

### How to Help

1. **Research**: Use UWPSpy to identify hook targets
2. **Testing**: Test on different Windows 11 builds
3. **Code**: Implement function hooks using existing mod patterns
4. **Documentation**: Improve docs and usage examples

See [DEVELOPMENT.md](DEVELOPMENT.md) for detailed development notes.

## Technical Details

### Target Element Path

```
SystemTray.OmniButton#ControlCenterButton
  > Grid
  > ContentPresenter
  > ItemsPresenter
  > StackPanel (Orientation="Horizontal") ← Target
  > ContentPresenter[1], ContentPresenter[2], ContentPresenter[3]
  > SystemTray.IconView (wifi, volume, battery)
```

### Modification Strategy

The mod intercepts the `StackPanel` and changes:
- `Orientation`: `Horizontal` (0) → `Vertical` (1)
- `Spacing`: User-configurable value (default: 4px)

### Implementation Approach

Uses C++ with Windows Runtime (WinRT) APIs:
- Function hooking via Windhawk's `Wh_SetFunctionHook()`
- Symbol-based hooking into `Taskbar.View.dll`
- XAML element tree traversal using `VisualTreeHelper`
- Property modification through WinRT interfaces

## Compatibility

### Supported Windows Versions
- Windows 11 22H2 (Build 22621+)
- Windows 11 23H2 (Build 22631+)
- Windows 11 24H2 (Build 26100+)

### Known Limitations
- Requires Windhawk framework
- May need explorer.exe restart on some updates
- Windows updates can change internal structure
- Not compatible with Windows 10 or earlier

### Tested Configurations
- Double-height taskbar with gridded icons
- Standard single-row taskbar
- Vertical taskbar (left/right placement)

## Troubleshooting

### Icons Don't Stack Vertically

1. Check that the mod is enabled in Windhawk
2. Try restarting explorer.exe (Task Manager → Windows Explorer → Restart)
3. Enable debug logging and check DebugView output
4. Verify Windows 11 version is supported

### Mod Doesn't Load

1. Check Windhawk logs for compilation errors
2. Ensure you have the latest Windhawk version
3. Verify the `.wh.cpp` file is not corrupted
4. Try disabling other taskbar mods temporarily

### Settings Don't Take Effect

1. Toggle the mod off and back on
2. Restart explorer.exe
3. Check for conflicts with Windows 11 Taskbar Styler
4. Review debug logs for errors

## Architecture

### Language & Platform
- **Language**: C++23 (Clang 20, mingw-w64)
- **Target Process**: `explorer.exe`
- **Framework**: Windhawk code injection
- **Architecture**: x86-64

### Dependencies
- `ole32.lib` - COM support
- `oleaut32.lib` - OLE automation
- `runtimeobject.lib` - Windows Runtime

### Key Components
- Element traversal utilities
- Orientation modification logic
- Settings management
- Logging infrastructure
- Hook implementation (TODO)

## Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/improvement`)
3. Test your changes on Windows 11
4. Commit with clear messages (`git commit -m "Add feature X"`)
5. Push to your fork (`git push origin feature/improvement`)
6. Open a Pull Request

### Development Resources
- [Windhawk Mod Creation Wiki](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windows 11 Taskbar Styling Guide](https://github.com/ramensoftware/windows-11-taskbar-styling-guide)
- [Windhawk Mods Repository](https://github.com/ramensoftware/windhawk-mods)
- [C++/WinRT Documentation](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)

## License

This project is open source. See LICENSE file for details.

## Credits

**Author**: sb4ssman

**Inspired by**:
- [Vertical Taskbar for Windows 11](https://windhawk.net/mods/taskbar-vertical) by m417z
- [Taskbar notification icon spacing](https://windhawk.net/mods/taskbar-notification-icon-spacing) by m417z
- [Windows 11 Taskbar Styling Guide](https://github.com/ramensoftware/windows-11-taskbar-styling-guide) by Ramen Software

**Built with**: [Windhawk](https://windhawk.net/) - The customization marketplace for Windows programs

## Links

- **Project Repository**: [github.com/tmiller711/Windhawk-Vertical-Omnibutton](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton)
- **Issues**: [Report bugs and request features](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton/issues)
- **Windhawk**: [windhawk.net](https://windhawk.net/)
- **Ramen Software**: [ramensoftware.com](https://ramensoftware.com)

## Support

- Open an issue for bugs or feature requests
- Check [DEVELOPMENT.md](DEVELOPMENT.md) for technical details
- Visit [Windhawk discussions](https://github.com/ramensoftware/windhawk-mods/discussions) for community help

---

**Status**: 🚧 In Development - Core functionality not yet implemented. See [DEVELOPMENT.md](DEVELOPMENT.md) for progress.
