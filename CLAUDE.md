# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Working Mode — Read This First

**The user writes 100% of the code by hand.** `rtgj` is a from-scratch Vulkan
renderer built Handmade Hero style for the purpose of *learning rendering*. Code
written by Claude defeats that purpose.

Claude's role: provide **concepts** (the "what" and "why") and **how-to guidance**
(the approach, which Vulkan objects/calls are involved, the order of operations,
common pitfalls, what to verify). Do **not** produce implementation code blocks,
file edits to source, or scaffolding — even when asked to "implement" something.
Exception: if the user explicitly asks for a code sample, that overrides this.

Specs and plans live in `docs/superpowers/` and are written as conceptual guides
(no code) by design.

## Build

CMake with presets. **Ninja Multi-Config** generator, x64. Configure and build
must run inside an MSVC environment (x64 Native Tools prompt, or
`Launch-VsDevShell.ps1 -Arch amd64`) — Ninja finds `cl.exe` from that environment.

```sh
cmake --preset windows-x64                          # configure (also fetches deps)
cmake --build build --preset windows-x64-debug      # debug build
cmake --build build --preset windows-x64-release    # release build
```

Output: `build/Debug/RTGJ.exe` (or `Release/`). The configure step also exports
`build/compile_commands.json` for clangd. `build.bat` is intended as a one-command
wrapper but is currently empty.

Dependencies are pulled at configure time via CMake `FetchContent` (see
`cmake/`): **volk** (Vulkan function loader) and **spdlog** (logging). There is
no `vcpkg`/manual dependency step. The LunarG **Vulkan SDK** must be installed
with `VULKAN_SDK` set — volk needs its headers, and validation layers / `glslc`
are used later.

If CMake configure behaves oddly, delete `build/` (stale cache).

## Architecture

A thin, hand-written platform seam below all rendering code.

- `src/main.cpp` — entry point; owns the frame loop; ties platform + renderer.
- `src/platform/` — the OS-facing seam. `platform.hpp` is the **only** place
  native OS types cross the boundary, and they cross as opaque `void*`
  (`platform_get_instance_handle` / `platform_get_window_handle`). Nothing above
  this layer includes `<windows.h>`. `platform_win32.cpp` is the Win32
  implementation (window class, message pump, handles).
- `src/renderer/` — all Vulkan code lives in `renderer_vk.cpp` behind
  `renderer.hpp`. The public renderer API exposes no Vulkan types.
- `src/core/` — engine infrastructure (`log.*` for spdlog-based logging).

Key design decisions baked into the milestone plan:

- **Message pumping is separate from rendering.** `platform_pump_messages()`
  drains the OS queue; it is never called from inside renderer code. `main.cpp`
  drives the loop: pump, then render, until `platform_should_quit()`.
- **volk loads Vulkan**, not the static loader. `VK_NO_PROTOTYPES` is defined
  compile-wide; never link `vulkan-1.lib`. Order: `volkInitialize()` →
  `volkLoadInstance()` after instance → `volkLoadDevice()` after device.
- **Vulkan 1.3 dynamic rendering** (`vkCmdBeginRendering`) — no `VkRenderPass`
  or `VkFramebuffer` objects.
- `WIN32_LEAN_AND_MEAN` is defined compile-wide.

When adding a `.cpp` to the build, add it to the `add_executable` list in
`CMakeLists.txt` manually (no glob).

## Current Status

Milestone **M0 — Bring-Up** (see `docs/superpowers/plans/2026-05-17-m0-bring-up.md`):
a Win32 window that brings up the full Vulkan device stack and clears the screen
each frame, validation-clean. The Win32 window + message loop are done; the
Vulkan instance/device/swapchain/render-loop tasks are next. `renderer.hpp`,
`renderer_vk.cpp`, and `core/log.*` are still essentially empty stubs.

`tmp/` holds throwaway prototype versions of the platform layer and is
git-ignored — not part of the build.
