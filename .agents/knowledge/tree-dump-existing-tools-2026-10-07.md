# Tree Dump overlap and naming reassessment

User paused implementation/publication to reassess existing tools and floated
S-WML-Tool / Sbassman-Windhawk-Mod-Lab-Tool branding. No rename authorized yet.

Verified current primary sources on Oct 7:

- UWPSpy: https://github.com/m417z/UWPSpy
  MainDlg.cpp implements DumpElementRecursive and context-menu actions Copy
  subtree, Copy subtree with properties, and 10-second delayed versions.
  Earlier characterization as only a live one-element inspector was incomplete.
  https://github.com/m417z/UWPSpy/blob/main/UWPSpy/MainDlg.cpp
- lvt: https://github.com/asklar/lvt
  CLI exports native visual trees (including Windows XAML) to JSON/XML files,
  supports subtree/depth selection and live change watching. Significant overlap;
  taskbar compatibility and matching property detail have not been live tested.
- XamlTreeDump: https://www.nuget.org/packages/XamlTreeDump
  UWP library for text/JSON dumps and semantic comparisons within test apps;
  not a ready-made external taskbar collector.
- taskbar-multi-tray includes bounded debug tree logging:
  https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-multi-tray.wh.cpp

Conclusion: tree export is not novel. Lab utility may still offer a convenient
Windhawk-integrated, taskbar-specific snapshot workflow (settled changes, layout
and visual-state facts, optional text), but do not claim no existing tools do
this. Recommend a descriptive catalog name with optional Sbassman branding;
avoid Tool and Windhawk in the display-name prefix to address reviewer concerns.
Keep tool-mods/ lab organization independent of public identity.
