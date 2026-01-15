# Project Research & Findings: Task Manager Tail

## Objective
Maintain a specific application (default: Task Manager) at the visual end (tail) of the Windows 11 Taskbar.

## Technical Approaches Explored

### 1. Polling (Timer-based)
*   **Method:** Run a loop every 50-100ms to check button positions.
*   **Verdict:** Rejected.
*   **Reason:** Wastes CPU cycles when idle; reaction time is tied to polling frequency, causing visual "lag" or "jumping" after the fact.

### 2. UI Automation (UIA) Events
*   **Method:** Register `IUIAutomationStructureChangedEventHandler` within `explorer.exe`.
*   **Verdict:** Rejected.
*   **Reason:** Running UIA event listeners inside the provider process (`explorer.exe`) caused deadlocks and UI freezes (the "seizing up" issue).

### 3. WinEventHook (Global Hooks) - **CHOSEN**
*   **Method:** Use `SetWinEventHook` with `EVENT_OBJECT_CREATE`, `DESTROY`, `SHOW`, `HIDE`.
*   **Verdict:** Successful.
*   **Reason:** Lightweight system mechanism. By filtering for `OBJID_WINDOW`, we only wake up when top-level windows change state.

## Implementation Details

*   **Detection:** We use `SetWinEventHook` to detect when *any* window is created or hidden.
*   **Debouncing:** A 300ms timer prevents the logic from running multiple times during a "burst" of events (e.g., opening a layout of apps).
*   **Identification:** We use UI Automation (UIA) to scan the specific XAML Island (`DesktopWindowXamlSource`) inside the Taskbar to find the button order.
*   **Action:** We use `ITaskbarList::DeleteTab` and `AddTab` to physically move the target window's button to the end.

## Capabilities & Limitations (v1.0)

*   **Targeting:** Supports **one** target application at a time.
    *   Configured via `Target Window Class` (e.g., `TaskManagerWindow`) and `Target Button Name`.
*   **Matching Logic:**
    *   **Class:** Exact match required (found via Spy++ or similar).
    *   **Name:** Partial substring match (e.g., "Bridge" matches "LLM Bridge"). Case-sensitive.
*   **Ordering:** Does not currently support sorting a list of multiple pinned apps. It simply ensures the *one* target app is moved to the very last position.

## Phase 3: Final Polish & Release Candidate (v1.0)

### 11. Final Architecture
- **Detection Strategy**: `SetWinEventHook` (Global) listening for `EVENT_OBJECT_CREATE` and `EVENT_OBJECT_HIDE`. This is lightweight and event-driven, avoiding polling loops.
- **Debouncing**: A 300ms timer is used to coalesce rapid events (e.g., restoring a session) into a single check.
- **Identification**: We use UI Automation (UIA) to scan the `DesktopWindowXamlSource` within the Taskbar. We look for children with the class `TaskListButtonAutomationPeer`.
- **Action**: We use `ITaskbarList::DeleteTab` followed by `AddTab` to move the target window. This leverages the OS's native behavior to append items to the end of the list.
- **Configuration**:
  - `targetName`: Partial string match for the button text (handles localization).
  - `targetClass`: Window class name (handles targeting specific apps like Notepad vs Task Manager).
- **Limitations**: Currently supports tracking a single application. Multi-app support would require more complex state management and sequential moves.