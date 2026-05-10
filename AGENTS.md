# AGENTS.md

## Project Intent
- `ICueNexusPlusPlus` is a Linux C++ wrapper around `hidapi` for Corsair iCUE Nexus (640x48 BGRA display), with reusable device logic in `src/lib/` and executable demos in `src/demos/`.
- Keep library/demo separation: reusable transport and protocol code stays in `src/lib/ICueNexus.*`; UI/demo behavior stays under `src/demos/*`.

## Architecture You Need First
- Core boundary: `ICueNexus` class (`src/lib/ICueNexus.h`) owns `hid_device*` and manages lifecycle with RAII (`hid_init` in constructor, `hid_exit` in destructor in `src/lib/ICueNexus.cpp`).
- Data flow for frame updates:
  1. Demo renders pixels (SDL surface in `src/demos/clock/main.cpp`).
  2. Calls `ICueNexus::ShowImage(...)`.
  3. `ShowImage` chunks a full 122,880-byte frame into HID feature reports (8-byte header + up to 1016-byte payload) before device commit.
- Protocol safety is enforced with compile-time checks in `ShowImage` (packet layout `static_assert`s in `src/lib/ICueNexus.cpp`): preserve these when editing report structs.
- `SendReport<T...>()` in `src/lib/ICueNexus.cpp` is the generic feature-report builder used by initialization and transfer paths.

## Build / Run Workflow
- Requires: `hidapi-hidraw`, `SDL3`, `SDL3_ttf`.
- Standard workflow:
  - `cmake -B build`
  - `cmake --build build`
  - `./build/clock`
- Root config (`CMakeLists.txt`) sets `CMAKE_CXX_STANDARD 26`; use C++26 features consistently (e.g., `std::format`, chrono zoned time usage).
- Targets are added from `src/` CMake: library plus demos (`clock`, `basic`).

## Project-Specific Coding Patterns
- Error model: HID failures throw `std::runtime_error` (do not silently ignore write/read failures).
- RAII is expected for native resources:
  - HID handle ownership in `ICueNexus`.
  - SDL resources wrapped with custom deleters in `src/demos/clock/main.cpp`.
- `clock` demo runtime loop pattern to preserve when modifying:
  - Poll SDL events, render text, send full frame, sleep ~500ms, exit cleanly via SIGINT handler.

## Integration Points
- USB device constants are project-critical and centralized in `src/lib/ICueNexus.h`:
  - Vendor ID `0x1b1c`, Product ID `0x1b8e`.
- External communication path is one-way frame streaming over HID feature reports; there is no separate service/process boundary.
- `clock` demo depends on font asset copy behavior from CMake (`src/demos/clock/font.ttf` copied into build output). Keep this asset flow working when changing build logic.

## When Making Changes
- If you touch transfer logic in `ShowImage`, verify payload sizing math and packet count behavior against 640x48 BGRA assumptions.
- If you add demos, mirror existing `src/demos/*/CMakeLists.txt` structure and link against `ICueNexus` plus required SDL components.
- If you change API signatures in `src/lib/ICueNexus.h`, update both demos (`src/demos/basic/main.cpp`, `src/demos/clock/main.cpp`) in the same change.

