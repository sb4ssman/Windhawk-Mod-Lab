# Task Manager Tail

A Windhawk mod that automatically keeps **Task Manager** (or any other specified application) at the tail end of your Windows 11 taskbar.

## Features

*   **Event Driven:** Uses lightweight system hooks (`SetWinEventHook`) to detect window changes instantly without polling.
*   **Zero CPU Usage:** Sleeps completely when no windows are opening or closing.
*   **Configurable:**
    *   **Target Application:** Default is Task Manager, but can be configured for Notepad, Calculator, etc.
    *   **Localization Support:** Works with non-English Windows versions via partial name matching.
    *   **Timing Control:** Adjustable delays and debounce timers to suit your preference.

## Installation

1.  Install Windhawk.
2.  Create a new mod in Windhawk.
3.  Copy the code from `task-manager-tail.wh.cpp`.
4.  Paste it into the editor and click **Compile and Enable**.

## Configuration

You can customize the behavior in the Windhawk mod settings:

*   **Target Button Name:** The text on the taskbar button (e.g., "Task Manager"). Supports partial matching.
*   **Target Window Class:** The internal class name (e.g., `TaskManagerWindow`). Use tools like Spy++ to find this for other apps.
*   **Move Delay:** Time (ms) to wait before moving the button (visual smoothing).
*   **Event Debounce Time:** Time (ms) to wait for window events to settle before checking.

## How It Works

This mod runs as a background tool and listens for global window creation/destruction events. When the taskbar layout changes, it uses UI Automation to scan the taskbar buttons. If the target application is found but is not at the end of the list, it uses the `ITaskbarList` interface to move it to the tail.

## Supported Systems

*   **Windows 11** (Relies on `DesktopWindowXamlSource` structure in the taskbar).

## License

MIT