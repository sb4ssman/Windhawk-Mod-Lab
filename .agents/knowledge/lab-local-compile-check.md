# Local Windhawk syntax check (no Windhawk build needed)

The installed Windhawk ships its clang toolchain; a mod can be syntax-checked
locally without submitting or building through the Windhawk UI:

```sh
"/c/Program Files/Windhawk/Compiler/bin/clang++.exe" \
  --target=x86_64-w64-windows-gnu -std=c++23 -fsyntax-only \
  -DUNICODE -D_UNICODE -DWH_MOD -DWH_EDITING \
  -DWH_MOD_ID=L\"<mod-id>\" -DWH_MOD_VERSION=L\"<ver>\" \
  -I"/c/Program Files/Windhawk/Compiler/include" \
  -include windhawk_api.h <mod-file>.wh.cpp
```

Notes (established 2026-07-19 on the Clock Spacer fix):

- `-DWH_MOD` is what exposes the `Wh_*` API declarations in `windhawk_api.h`;
  `-DWH_EDITING` selects the editing variant of the internal macros.
- `-DUNICODE -D_UNICODE` are required or `RegisterWindowMessage` etc. resolve
  to the ANSI variants and fail on `L""` strings.
- This is `-fsyntax-only`: it validates includes, WinRT projections, and types
  but does not link, so `@compilerOptions` libs are irrelevant here.
- First run takes ~1-2 minutes (WinRT headers); plan timeouts accordingly.
