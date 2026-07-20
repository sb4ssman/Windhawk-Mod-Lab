# Task Manager Tail 1.1

This mod ensures that **Task Manager** always stays at the tail end of your taskbar on **Windows 10 and Windows 11**.

When you open or close other applications, this mod detects the change and automatically
moves the Task Manager button to the tail end of the list.

## Features
- **Event Driven:** Uses lightweight hooks to detect window changes instantly.
- **Zero Polling:** Does not waste CPU cycles checking the taskbar constantly.
- **Configurable:** Supports non-English languages and other target applications.
- **Cross-Platform:** Works on both Windows 10 and Windows 11.

## Platform Notes

**Windows 11:** Full functionality. Responds to all window open/close events.

**Windows 10:** The target moves to the tail when new applications are opened.
Due to Windows 10 taskbar limitations, closing apps or manual dragging may not
immediately trigger a reposition - the target will return to the tail on the
next app open event. This decision avoids polling, or alternatively parsing 
many possible (continuous) user interaction event noise.
