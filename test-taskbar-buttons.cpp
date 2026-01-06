// Simple C++ test program to enumerate taskbar buttons
// Compile with: g++ -o test-taskbar-buttons.exe test-taskbar-buttons.cpp -lgdi32 -luser32 -lcomctl32
// Or use MSVC: cl test-taskbar-buttons.cpp /link user32.lib comctl32.lib

#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <vector>

struct ButtonInfo {
    int index;
    HWND hwnd;
    wchar_t text[256];
};

// Find the taskbar toolbar window
HWND FindTaskbarToolbar() {
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hTaskbar) {
        std::wcout << L"ERROR: Could not find taskbar window" << std::endl;
        return NULL;
    }
    
    std::wcout << L"Found taskbar window: 0x" << std::hex << (ULONG_PTR)hTaskbar << std::dec << std::endl;
    
    // Find toolbar windows
    HWND hToolbar = NULL;
    int toolbarIndex = 0;
    
    while ((hToolbar = FindWindowExW(hTaskbar, hToolbar, L"ToolbarWindow32", NULL)) != NULL) {
        int buttonCount = (int)SendMessageW(hToolbar, TB_BUTTONCOUNT, 0, 0);
        std::wcout << L"Toolbar #" << toolbarIndex << L": handle=0x" << std::hex << (ULONG_PTR)hToolbar 
                   << std::dec << L", buttons=" << buttonCount << std::endl;
        
        if (buttonCount > 0) {
            std::wcout << L"  -> This appears to be the main taskbar toolbar" << std::endl;
            return hToolbar;
        }
        
        toolbarIndex++;
    }
    
    return NULL;
}

// Enumerate taskbar buttons
std::vector<ButtonInfo> EnumerateTaskbarButtons(HWND hToolbar) {
    std::vector<ButtonInfo> buttons;
    
    if (!hToolbar) {
        return buttons;
    }
    
    int buttonCount = (int)SendMessageW(hToolbar, TB_BUTTONCOUNT, 0, 0);
    std::wcout << L"Enumerating " << buttonCount << L" buttons..." << std::endl;
    
    for (int i = 0; i < buttonCount; i++) {
        TBBUTTON button = {0};
        if (SendMessageW(hToolbar, TB_GETBUTTON, i, (LPARAM)&button)) {
            ButtonInfo info = {0};
            info.index = i;
            
            // Get button text
            wchar_t text[256] = {0};
            TCHAR buffer[256] = {0};
            if (button.iString != -1) {
                SendMessageW(hToolbar, TB_GETBUTTONTEXT, i, (LPARAM)buffer);
                wcscpy_s(info.text, buffer);
            }
            
            // TODO: Get window handle associated with button
            // This might require accessing internal taskbar structures
            
            buttons.push_back(info);
            
            std::wcout << L"  Button #" << i << L": \"" << info.text << L"\"" << std::endl;
        }
    }
    
    return buttons;
}

int main() {
    std::wcout << L"=== Taskbar Button Enumeration Test ===" << std::endl;
    std::wcout << std::endl;
    
    HWND hToolbar = FindTaskbarToolbar();
    if (!hToolbar) {
        std::wcout << L"ERROR: Could not find taskbar toolbar" << std::endl;
        return 1;
    }
    
    std::wcout << std::endl;
    std::vector<ButtonInfo> buttons = EnumerateTaskbarButtons(hToolbar);
    
    std::wcout << std::endl;
    std::wcout << L"Found " << buttons.size() << L" buttons total" << std::endl;
    
    // Check if Task Manager is in the list
    bool foundTaskMgr = false;
    for (const auto& button : buttons) {
        if (wcsstr(button.text, L"Task Manager") != NULL || 
            wcsstr(button.text, L"Taskmgr") != NULL) {
            foundTaskMgr = true;
            std::wcout << L"Task Manager found at index " << button.index << std::endl;
        }
    }
    
    if (!foundTaskMgr) {
        std::wcout << L"Task Manager not found in taskbar buttons." << std::endl;
        std::wcout << L"Please open Task Manager and run this test again." << std::endl;
    }
    
    return 0;
}
