# Instrument dashboard visual prototype

Open `index.html` locally. No server, network access, telemetry collector, or
dependencies. Every reading is explicitly simulated. This is a desktop-console
design experiment, not a published website or a working system monitor.

Green, amber, and blue phosphor palettes; bars, history graphs, and gauges;
1x1, 2x1, 1x2, 2x2, 3x2, and 3x3 footprints. Arrange mode enables drag ordering,
keyboard-accessible move buttons, resizing, style selection, and removal.
Add instruments, freeze the readings, or restore the factory layout. Layout and
palette are saved in the browser's local storage when available.

Larger instruments reveal history and supporting readings. CPU is the dominant
3x3 instrument; GPU gets a broad gauge; network/storage use long histories.
References: the user's automotive instrument photos and https://tmog.org/.
Next stage: choose a native host and real metric providers; unavailable sensors
must become unavailable readings rather than plausible demonstration numbers.
