# RTGJ — Logger Module — Design Spec

**Date:** 2026-05-18
**Status:** Approved design — ready for an implementation plan
**Reference:** `docs/superpowers/specs/2026-05-17-vulkan-renderer-design.md` (§2 structure),
`docs/superpowers/plans/2026-05-17-m0-bring-up.md` (M0 — the logger precedes Task 3)

> **How to use this spec:** You write 100% of the code by hand. This document gives
> the concepts, the decisions and their reasoning, and ordered how-to guidance — which
> spdlog objects/calls are involved and in what order. It contains no implementation
> code, by design, per the project's hand-write-everything constraint.

---

## 1. Purpose & Scope

A small logging module under `src/core/`, built on **spdlog** (already vendored at
v1.17.0 via `cmake/spdlog.cmake` and linked as the compiled `spdlog::spdlog` target).

**Why now.** The logger is built before the Vulkan instance work (M0 Task 3) because
the Vulkan validation-layer **debug-messenger callback** needs a destination for its
output. A logger in place first means every later mistake is reported through one
consistent channel. It also gives `platform_win32.cpp` somewhere better than
`stderr` to report failures.

**In scope:** a macro-based logging API, console + rotating-file output, per-build
compile-time level stripping, and an init/shutdown lifecycle.

**Out of scope (YAGNI):** `RTGJ_ASSERT` / `RTGJ_LOG_FATAL` (log-then-debug-break);
per-subsystem named loggers; the Visual Studio `msvc_sink`. These are noted as
possible future additions, not built now.

---

## 2. Architecture & The Seam

The logger is a `core` module with one public header. The seam works like the
platform layer, with one honest caveat about macros.

- **Call sites** (`main.cpp`, `platform_win32.cpp`, `renderer_vk.cpp`, …) only ever
  `#include "core/log.hpp"` and only ever type `RTGJ_LOG_*`. No call site includes
  a spdlog header directly.
- `log.hpp` itself **does** transitively include `<spdlog/spdlog.h>`. This is
  unavoidable: the `RTGJ_LOG_*` macros forward to spdlog's macros, and a macro must
  see spdlog's declarations where it expands. This is the accepted cost of the macro
  approach.
- `log.cpp` is the **only** `.cpp` that includes spdlog directly, and it owns all
  configuration (sinks, pattern, levels, flush policy, lifecycle).

**What the seam still buys**, despite the transitive include: every call site speaks
only `RTGJ_LOG_*`, and swapping spdlog for another backend later means editing only
`log.hpp` + `log.cpp` — no call site changes.

**Why a macro can't be hidden the way `void*` hides `HWND`.** A function call
resolves at link time, so the platform layer keeps Win32 types out of its header
entirely. A macro expands *at the call site*, so its dependencies must be visible
there. This is why the logger seam is "softer" than the platform seam, and why
consistent naming (`RTGJ_LOG_*` everywhere) does the real isolation work.

---

## 3. Files & Responsibilities

Both files already exist as empty stubs and are already compiled by `CMakeLists.txt`.
No new files and no new CMake targets are introduced.

| File | Responsibility |
|------|----------------|
| `src/core/log.hpp` | Public API. Includes `<spdlog/spdlog.h>`; declares `log_init()` / `log_shutdown()`; defines the six `RTGJ_LOG_*` macros. |
| `src/core/log.cpp` | The only spdlog configuration site. Implements `log_init` / `log_shutdown`: builds sinks, assembles the logger, sets pattern, level, and flush policy. |

---

## 4. The Macro API

Six macros, thin forwarders over spdlog's own macros (`SPDLOG_TRACE`, `SPDLOG_DEBUG`,
`SPDLOG_INFO`, `SPDLOG_WARN`, `SPDLOG_ERROR`, `SPDLOG_CRITICAL`):

| Macro | Level | Typical use |
|-------|-------|-------------|
| `RTGJ_LOG_TRACE`    | trace    | Hot-path, per-frame / per-draw detail. |
| `RTGJ_LOG_DEBUG`    | debug    | Development diagnostics. |
| `RTGJ_LOG_INFO`     | info     | Lifecycle milestones (init done, device chosen). |
| `RTGJ_LOG_WARN`     | warn     | Recoverable oddities. |
| `RTGJ_LOG_ERROR`    | error    | Failures (e.g. `CreateWindowEx` failed). |
| `RTGJ_LOG_CRITICAL` | critical | Unrecoverable conditions. |

**Why forward to spdlog's macros** rather than to `spdlog::info(...)` functions:
spdlog's `SPDLOG_*` macros already (a) capture `__FILE__` / `__LINE__` / function
automatically, and (b) honour `SPDLOG_ACTIVE_LEVEL` for compile-time stripping. Both
come for free. The `RTGJ_` wrapper adds consistent project naming, a single swap
point, and a future home for added behaviour.

---

## 5. Two Level Controls (Do Not Confuse Them)

The logger has two independent level mechanisms:

1. **`SPDLOG_ACTIVE_LEVEL` — compile-time.** A preprocessor value. Any `SPDLOG_*`
   macro (and therefore any `RTGJ_LOG_*` macro) for a level below it expands to
   **nothing** — the argument expressions are not even compiled. This is what makes
   `RTGJ_LOG_TRACE` cost literally zero in a Release build.
2. **Runtime level (`set_level`) — runtime.** Filters, at runtime, among whatever
   survived compilation.

**Per-build configuration (approved):**

| Build   | `SPDLOG_ACTIVE_LEVEL`  | Compiled-in levels        | Runtime level |
|---------|------------------------|---------------------------|---------------|
| Debug   | `SPDLOG_LEVEL_TRACE`   | trace → critical (all)    | `trace`       |
| Release | `SPDLOG_LEVEL_INFO`    | info → critical           | `info`        |

So Release strips `trace` and `debug` entirely; `info` and above remain.

---

## 6. spdlog Configuration (inside `log_init`)

- **Two sinks, both `_mt` variants:**
  - `stdout_color_sink_mt` — colored console output.
  - `rotating_file_sink_mt` — file at `logs/rtgj.log`, **5 MB per file, 3 files**.
  - `_mt` (multi-threaded) is chosen now because the renderer gains worker threads in
    later milestones; choosing `_st` would force a change then.
- **One logger** holding both sinks, registered as spdlog's **default logger** so the
  `SPDLOG_*` macros (which target the default logger) work without passing a logger
  handle.
- **Pattern:** timestamp, colored level, and `source-file:line` (spdlog pattern flags
  `%s:%#`), then the message. The source location only populates because logging
  goes through the `SPDLOG_*` macros.
- **Runtime level:** set in `log_init` per the table in §5.
- **Flush policy:** `flush_on(warn)` — a crash still leaves warnings and errors on
  disk.

---

## 7. Lifecycle & Wiring

- `log_init()` is called **first thing in `main()`**, before `platform_init()`, so a
  platform failure can be logged.
- `log_shutdown()` is called **last**, after `platform_shutdown()`; it calls spdlog's
  `shutdown()` to flush and release sinks.
- After the logger is up, the `std::fprintf(stderr, …)` calls currently in
  `platform_win32.cpp` (`RegisterClassEx` / `CreateWindowEx` failure reporting) are
  replaced with `RTGJ_LOG_ERROR`.
- Downstream consumer: M0 Task 3's Vulkan debug-messenger callback logs validation
  output through `RTGJ_LOG_*`.

---

## 8. Build Wiring (CMake)

One edit to `CMakeLists.txt`:

- Add `SPDLOG_ACTIVE_LEVEL` via `target_compile_definitions` (it must be a compile
  definition so it is defined before any include). Its value must vary per build
  config — use a generator expression on `$<CONFIG:Debug>` selecting
  `SPDLOG_LEVEL_TRACE` versus `SPDLOG_LEVEL_INFO`. A generator expression is required
  because the Visual Studio generator is multi-config.

No other CMake change is needed. `SPDLOG_COMPILED_LIB` is already handled — linking
the compiled `spdlog::spdlog` target propagates it, so fmt template-instantiation
cost is avoided.

One edit to `.gitignore`: add `logs/`.

---

## 9. Suggested Implementation Order

1. `CMakeLists.txt` — add the `SPDLOG_ACTIVE_LEVEL` generator-expression compile
   definition.
2. `log.hpp` — include `<spdlog/spdlog.h>`; declare `log_init` / `log_shutdown`;
   define the six `RTGJ_LOG_*` macros.
3. `log.cpp` — implement `log_init` (sinks → logger → register as default → pattern →
   runtime level → flush policy) and `log_shutdown`.
4. `main.cpp` — call `log_init` first and `log_shutdown` last; add a smoke-test
   `RTGJ_LOG_INFO`.
5. Build and run — confirm colored console output **and** that `logs/rtgj.log`
   appears.
6. `platform_win32.cpp` — replace the `fprintf(stderr, …)` calls with
   `RTGJ_LOG_ERROR`.
7. `.gitignore` — add `logs/`.

---

## 10. Definition of Done

- `RTGJ.exe` prints a colored startup `INFO` line to the console.
- `logs/rtgj.log` is created and contains the same line.
- A Release build compiles `RTGJ_LOG_TRACE` / `RTGJ_LOG_DEBUG` to nothing (verified
  by inspection or by confirming no trace/debug output appears).
- `platform_win32.cpp` no longer calls `fprintf`; failures go through `RTGJ_LOG_ERROR`.
- No spdlog header is `#include`d by any `.cpp` other than `log.cpp`.

---

## 11. Pitfalls

- **`SPDLOG_ACTIVE_LEVEL` defined too late.** It must be a compile definition. If it
  is `#define`d in a header *after* spdlog is included, stripping silently does not
  apply. The CMake compile definition avoids this entirely.
- **`_st` vs `_mt` sinks.** Use `_mt`; mixing single-threaded sinks with future
  worker-thread logging is a data race.
- **Forgetting to register the logger as default.** The `SPDLOG_*` macros target the
  default logger; if it is not registered, output goes to spdlog's stock default
  logger instead of your two-sink one.
- **Source location blank.** It populates only via the `SPDLOG_*` macros, not via
  `spdlog::info(...)` function calls — another reason the macro API is the only API.
- **Logging before `log_init`.** Any `RTGJ_LOG_*` before `log_init` runs hits
  spdlog's stock default logger, not the configured one. Keep `log_init` first.
