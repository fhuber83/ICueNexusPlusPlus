# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ICueNexusPlusPlus is a C++ wrapper around hidapi for controlling the Corsair iCUE Nexus — a 640×48 24-bit color USB touchscreen display. It provides a Linux-compatible alternative to Corsair's Windows-only iCUE software.

## Build

```bash
cmake -B build
cmake --build build
./build/clock
```

Requires: `hidapi-hidraw`, `SDL3`, `SDL3_ttf`. The build copies `src/demo/font.ttf` to the build directory automatically.

## Structure

- `src/lib/` — reusable library (`ICueNexus.h`, `ICueNexus.cpp`)
- `src/demos/clock/` — clock demo (`main.cpp`, `font.ttf`)

## Architecture

**src/lib/ICueNexus.h / ICueNexus.cpp** — the reusable library class. Wraps a raw `hid_device*` with RAII semantics. Key constants:
- Corsair Vendor ID `0x1b1c`, Product ID `0x1b8e`
- Screen: 640×48px, BGRA32, 122,880 bytes per frame
- Image transfer: chunked HID packets (8-byte header + up to 1016 bytes payload each)
- `SendReport<T...>()` is a variadic template that builds and sends HID feature reports

**src/demos/clock/main.cpp** — clock demo: a real-time clock rendered with SDL3_ttf, refreshed every 500ms, with SIGINT shutdown.

## C++ conventions

- Standard: C++26 (uses `std::format`, `std::chrono::zoned_time`)
- Error handling: throw `std::runtime_error` on HID failures
- RAII throughout — use custom deleters or smart pointers for SDL/HID resources
- Struct layout safety is validated with `static_assert` (`has_unique_object_representations_v`, `sizeof` checks)
