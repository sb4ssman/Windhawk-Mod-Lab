## Overview
This Windhawk mod automatically keeps Task Manager as the last item on the taskbar. It uses the `ITaskbarList` interface to programmatically reorder the button, which is efficient and does not hijack the user's mouse.

## Current Status
- **Final Version (v16.4)**: Uses the `ITaskbarList` interface. It is optimized to only run when Task Manager is open and includes a fix to prevent it from fighting with pinned items.
