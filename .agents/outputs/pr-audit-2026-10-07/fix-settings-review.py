from pathlib import Path

p = Path('privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp')
s = p.read_text(encoding='utf-8')
s = s.replace('return ParseColorToken(setting.get() ? setting.get() : L"", out);', 'return ParseColorToken(setting.get(), out);')
s = s.replace('    g_cameraHardwareDetectionEnabled.store(\n        s.cameraHardwareDetection);', '}\n\n// Publish worker toggles only after the matching UI settings are accepted.\nstatic void PublishWorkerSettings(ModSettings const& s) {\n    g_cameraHardwareDetectionEnabled.store(\n        s.cameraHardwareDetection);')
s = s.replace('LoadSettings(g_settings);', 'LoadSettings(g_settings);\n    PublishWorkerSettings(g_settings);')
s = s.replace('Wh_Log(L"[Init] Privacy Anchor v2.0");', 'Wh_Log(L"[Init] Privacy Anchor v" WH_MOD_VERSION);')
s = s.replace('    Grid::SetColumn(group, 0);\n', '    Grid::SetRow(group, 0);\n    Grid::SetRowSpan(group, std::max(1, static_cast<int>(rootGrid.RowDefinitions().Size())));\n    Grid::SetColumn(group, 0);\n')
s = s.replace('                if (g_lease)\n                    g_lease->Refresh(iconView, UIElement::VisibilityProperty());', '''                auto local = iconView.ReadLocalValue(UIElement::VisibilityProperty());
                bool localMatches = local == DependencyProperty::UnsetValue() ||
                    winrt::unbox_value<Visibility>(local) == iconView.Visibility();
                if (g_lease && localMatches)
                    g_lease->Refresh(iconView, UIElement::VisibilityProperty());''')
s = s.replace('// hooks are live. The state worker reads only the atomics LoadSettings\n    // stores directly.', '// hooks are live. Worker toggles are published with the accepted copy.')
s = s.replace('           g_cameraHardwareDetectionEnabled.load() ? 1 : 0,\n           next.glowEnabled', '           next.cameraHardwareDetection ? 1 : 0,\n           next.glowEnabled')
s = s.replace('    RequestStateRefresh(RefreshAll | RefreshMonitorSetup);\n\n    struct SettingsDispatch', '    struct SettingsDispatch')
s = s.replace('            g_settings = *d->next;', '            g_settings = *d->next;\n            PublishWorkerSettings(g_settings);\n            RequestStateRefresh(RefreshAll | RefreshMonitorSetup);')
s = s.replace('            g_settings = next;', '            g_settings = next;\n            PublishWorkerSettings(g_settings);\n            RequestStateRefresh(RefreshAll | RefreshMonitorSetup);')
p.write_text(s, encoding='utf-8')

p = Path('taskbar-folder-menus/taskbar-folder-menus.wh.cpp')
s = p.read_text(encoding='utf-8')
s = s.replace('return value.get() ? std::wstring(value.get()) : std::wstring{};', 'return std::wstring(value.get());')
s = s.replace('auto fits the taskbar height. Use 1 | 2', 'auto fits the taskbar height, or width on a side taskbar. Use 1 | 2')
s = s.replace('Compare those five cheap reads', 'Compare these cheap geometry/count reads')
needle = 'static void LoadSettings() {'
s = s.replace(needle, '''// Worker inputs are owned snapshots, independent of UI settings lifetime.
static std::mutex g_folderIconsMutex;  // exit-time-safe: heap-only
static std::vector<FolderEntry> g_iconEntries;  // exit-time-safe: heap-only
static int g_iconWidth = 22;
static int g_iconHeight = 22;

''' + needle)
s = s.replace('    g_settings.shineEffect = sio::LoadBool(L"Surface.ShineEffect");', '''    g_settings.shineEffect = sio::LoadBool(L"Surface.ShineEffect");
    std::lock_guard lock(g_folderIconsMutex);
    g_iconEntries = g_settings.folders;
    g_iconWidth = g_settings.buttonWidth;
    g_iconHeight = g_settings.buttonHeight;''')
s = s.replace('static std::mutex g_folderIconsMutex;  // exit-time-safe: heap-only\n\nstatic int FolderIconSize()', 'static int FolderIconSize()')
s = s.replace('        int size = FolderIconSize();\n        for (auto const& entry : g_settings.folders) {', '''        std::vector<FolderEntry> entries;
        int width, height;
        {
            std::lock_guard lock(g_folderIconsMutex);
            entries = g_iconEntries;
            width = g_iconWidth;
            height = g_iconHeight;
        }
        HWND taskbar = FindCurrentProcessTaskbarWnd();
        UINT dpi = taskbar ? GetDpiForWindow(taskbar) : 96;
        int size = std::clamp(MulDiv(std::max(1, std::min(width, height) - 4),
                                     dpi ? dpi : 96, 96), 8, 256);
        for (auto const& entry : entries) {''')
p.write_text(s, encoding='utf-8')
