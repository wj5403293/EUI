# EUI

EUI is a lightweight, efficiency-focused UI toolkit for OpenGL + GLFW.
The core is header-only (`include/EUI.h`) and can be integrated with custom render backends.

This repository contains:
- `index.html`: UI style and interaction prototype.
- C++ implementation: header-only core + CMake build + GLFW/OpenGL demo.

## Current Status (v0.1)

Implemented:
- Header-only core: `include/EUI.h`
- Theme system: light/dark mode + primary color
- Basic layout: vertical flow + row layout
- Core widgets:
  - `label`
  - `button`
  - `tab`
  - `slider_float`
  - `progress`
  - `begin_dropdown` / `end_dropdown`
- Draw command queue (filled rect / outline / text)
- GLFW + OpenGL demo app: `examples/basic_demo.cpp`
- Header compile check target: `eui_header_check`

## Project Layout

```text
EUI/
├─ include/
│  └─ EUI.h
├─ examples/
│  └─ basic_demo.cpp
├─ tests/
│  └─ header_check.cpp
├─ CMakeLists.txt
├─ index.html
└─ readme.md
```

## Build

### 1) Build core only (no GLFW required)

```bash
cmake -S . -B build -DEUI_BUILD_EXAMPLES=OFF
cmake --build build -j
```

### 2) Build demo with auto Git dependency for GLFW

```bash
cmake -S . -B build -DEUI_BUILD_EXAMPLES=ON
cmake --build build -j
```

Behavior:
- CMake first tries local GLFW package (`find_package(glfw3)`).
- If GLFW is missing and `EUI_FETCH_GLFW_FROM_GIT=ON` (default), CMake uses
  `FetchContent` to clone GLFW from GitHub automatically.
- OpenGL is linked automatically:
  - `OpenGL::GL` when available
  - `opengl32` fallback on Windows
  - `OpenGL` framework fallback on macOS

Important CMake options:

```bash
-DEUI_BUILD_EXAMPLES=ON|OFF
-DEUI_FETCH_GLFW_FROM_GIT=ON|OFF
-DEUI_GLFW_GIT_TAG=3.4
```

If your environment blocks network/Git access, disable auto fetch:

```bash
cmake -S . -B build -DEUI_BUILD_EXAMPLES=ON -DEUI_FETCH_GLFW_FROM_GIT=OFF
```

Then install GLFW locally and rerun CMake.

## Minimal Usage

```cpp
#include "EUI.h"

eui::Context ui;
eui::InputState input{};

ui.begin_frame(1280.0f, 720.0f, input);
ui.begin_panel("Demo", 20.0f, 20.0f, 600.0f);

float value = 0.5f;
ui.button("Run");
ui.slider_float("Value", value, 0.0f, 1.0f);
ui.progress("Loading", 0.42f);

ui.end_panel();
const auto& commands = ui.end_frame();
```

`commands` can be rendered by your own OpenGL/Vulkan/DirectX backend.

## Roadmap

1. Input focus and keyboard navigation
2. Text input, checkbox, and window drag widgets
3. More style controls (radius, shadow, animation)
4. Install/export support (`install()` and package manager flow)
5. More tests and CI
