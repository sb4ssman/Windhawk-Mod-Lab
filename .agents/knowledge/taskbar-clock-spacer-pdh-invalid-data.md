# Taskbar Clock Customization Spacer — PDH invalid-data flood

## Evidence captured 2026-07-18

During a Privacy Indicator Anchor reload, DbgView captured 279 runtime lines:

- 208 belonged to local mod `taskbar-clock-customizationt`.
- 143 were `PdhGetFormattedCounterValue error C0000BC6`.
- `0xC0000BC6` is `PDH_INVALID_DATA` (`PdhMsg.h`).
- The failure repeated for roughly 11 counters per one-second clock refresh.
- Privacy Indicator Anchor compiled and loaded successfully; this PDH output is
  unrelated to the privacy mod.

The current log site is
`taskbar-clock-customization-spacer/taskbar-clock-customization-spacer.wh.cpp`
in `QueryDataCollectionSession::QueryDataWithCount`, at the
`PdhGetFormattedCounterValue` failure branch (around line 2142 at capture time).

## Follow-up investigation

1. Log or otherwise identify the counter path associated with each failing
   handle; the current message reports only the status code.
2. Verify whether the counters require a second sample after
   `PdhCollectQueryData`, especially wildcard counters added after the previous
   collection.
3. Distinguish normal first-sample `PDH_INVALID_DATA` from counters that remain
   invalid indefinitely.
4. Remove, recreate, or temporarily suppress persistently invalid counters.
5. Rate-limit diagnostics to state transitions so one bad counter cannot emit
   an error every second forever.

Do this in a separate spacer-focused chat; no spacer source change was made
during the privacy investigation.
