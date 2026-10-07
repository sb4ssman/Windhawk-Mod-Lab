# Runtime choices for Windhawk 2.0

Prepared for the 2.0 settings UI; **not adopted by the family yet**. The current
Windows 10 experiment still installs on Windhawk 1.7.3, using ordinary settings
with separate, explicitly labelled Windows 10 and Windows 11 position lists.
Do not require an alpha upgrade to use it.

Reference: [official settings documentation](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod#dynamicselect).

## OS-specific position dropdown

Use [dynamic-setting-options.h](../dynamic-setting-options.h) to publish the
complete union of owned option values, marking unsupported ones unavailable.
Call after reliable OS/taskbar detection during initialization. The labels
live in local mod storage under `::wh_select_option::<path>::<value>`; they
are not settings and do not overwrite the user's selected position.

```cpp
namespace dynamic_options = windhawk_mod_templates::dynamic_setting_options;
dynamic_options::Option options[] = {
    {L"beforeIcons", L"Before hidden-icons chevron", classicTaskbar},
    {L"afterClock", L"After clock", true},
    {L"afterNotifications", L"Between notifications and Show Desktop", classicTaskbar},
    {L"leftOfStart", L"Left of Start", !classicTaskbar},
    // Include EVERY value the publisher has ever owned. Keep retired values
    // here as unavailable so labels from older runs don't remain visible.
};
if (!dynamic_options::Publish(L"Placement.Position", options))
    Wh_Log(L"Could not publish position choices");
```

This example is a subset, not VD Switcher's complete position list. Do not
embed it into that mod until all values are included and its settings/readmes
are reconciled. The helper is a standalone template now; convert it to an
assembled component when the first real mod adopts it.

For a runtime-only dropdown with a 1.7 fallback:

```yaml
- Placement:
  - Position: beforeIcons
    $name: Position
    $description: On Windhawk 1.7, enter a documented supported token.
    #! $dynamicSelect: true
  $name: Placement
```

Windhawk 1.7 treats the annotation as a comment and shows a text field.
Windhawk 2.0 shows the published dropdown. **Do not keep an unmarked static
`$options` list containing unsupported values:** static choices are merged
with runtime choices, so runtime publishing cannot remove them. Marking an
`$options` fallback is also possible alongside `$dynamicSelect`, but those
static entries are still shown by 2.0; use only choices valid on every target.

The currently selected value remains visible even when unsupported. A mod
must independently validate/clamp its effective placement and explain any
fallback. This mechanism cannot silently migrate the saved setting or make an
unsupported saved value vanish. Older values must remain parseable as needed.

## Conditional controls

`#! $showIf: {FeatureEnabled: true}` or `#! $hideIf: {Mode: none}` operates on
another **user setting**, not an OS variable or local storage value. Do not
invent a hidden `IsWindows10` setting or pretend the mod can set it. Automatic
OS-specific visibility of entire groups needs a separate supported mechanism;
runtime dropdown options alone solve the position-choice problem.

Before adoption on the Windows 11 machine: verify the then-current official
release/schema, install/test the new UI, check both OS lists and stale saved
values, verify that the marked settings still validate on 1.7.3, assemble the
helper into the adopting mod, and run the normal lab checks. Leave family
publishing dependent on the human live test of each exact source build.
