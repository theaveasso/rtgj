# RTGJ — From-Scratch Vulkan Renderer — Design Spec

**Date:** 2026-05-17
**Status:** Approved design — ready for milestone-by-milestone build guides

---

## 1. Overview & Constraints

RTGJ is a from-scratch real-time renderer built as a **learning project**, in the
spirit of Handmade Hero: minimal dependencies, understand every layer, hand-write
all code. The arc runs from first triangle to a modern **GPU-driven PBR** pipeline.

**Locked decisions:**

| Decision | Choice | Reasoning |
|----------|--------|-----------|
| Graphics API | **Vulkan only** | GPU-driven rendering & PBR are Vulkan-shaped. No OpenGL, no swap layer, no RHI abstraction — that would be premature abstraction with one backend. |
| OS target | **Windows only** | One platform layer (Win32). Native Vulkan + no macOS translation politics. Platform seam kept clean so a Linux port stays possible, but not paid for now. |
| Dependencies | **Pragmatic Handmade** | Hand-write the Win32 layer + math library. Vendor single-header libs (`stb_image`, `cgltf`) and unavoidable tooling (Vulkan SDK, `volk`, `glslc`). Time goes to rendering, not reinventing PNG decoders. |
| Code authorship | **User writes 100% of code by hand** | The point is learning by building. This spec and all build guides provide concepts + how-to only — never implementation code. |
| Roadmap | **Open-ended milestone ladder** | Every milestone is a runnable, demoable renderer. Stop at any rung. |

**Goals:** learn the GPU pipeline deeply; reach metallic-roughness PBR (M5) and a
GPU-driven indirect-draw pipeline with compute culling (M9).

---

## 2. Project Structure

A handful of compiled translation units (Handmade Hero style) — fast full rebuilds,
small enough to hold in your head. Not 50 tiny files.

```
rtgj/
├─ build.bat            # one-command build (wraps CMake)
├─ CMakeLists.txt
├─ shaders/             # .vert / .frag / .comp → compiled to SPIR-V
├─ assets/              # glTF models, HDR envmaps, textures
├─ third_party/         # volk, stb_image.h, cgltf.h (vendored)
└─ src/
   ├─ main.cpp          # entry point; includes the platform unit
   ├─ platform_win32.cpp# Win32 window, input, timing, file I/O, VkSurface hook
   ├─ platform.hpp      # the platform↔app seam
   ├─ math.hpp          # hand-written vec / mat / quat
   ├─ renderer.hpp      # renderer module's public API
   ├─ renderer_vk.cpp   # the renderer — the bulk of the project
   └─ app.cpp           # scene setup, camera control, demo logic
```

**File responsibilities** (what each *is for* — contents are yours to write):

- `platform_win32.cpp` — owns the OS. Creates the window, pumps the message loop,
  reads keyboard/mouse, queries high-resolution time, reads files from disk, and
  exposes a hook to create a `VkSurfaceKHR` from the window's `HWND`.
- `platform.hpp` — the single OS-facing seam. Declares the platform API the rest of
  the app calls. Nothing above it includes `<windows.h>`.
- `math.hpp` — pure, deterministic vector/matrix/quaternion math. No dependencies.
- `renderer.hpp` — the renderer's public API. Declares the `Scene` struct and the
  `renderer_*` entry points. No Vulkan types leak through this header.
- `renderer_vk.cpp` — all Vulkan code. Device, swapchain, memory, pipelines, passes.
- `app.cpp` — builds a `Scene` each frame and hands it to the renderer. Never
  touches Vulkan.

**Build:** keep `CMakeLists.txt`; add a `build.bat` so iteration is one keystroke.
Shaders compile to SPIR-V as a CMake build step; runtime shader hot-reload arrives
at M3.

---

## 3. Module Layout & Data Flow

One direction of dependency, no cycles:

```
   ┌─────────────┐   builds a Scene    ┌──────────────┐
   │   app.cpp   │ ──(meshes, camera,──▶│ renderer_vk  │
   │ (demo/scene)│    lights, handles) │  (Vulkan)    │
   └─────────────┘                     └──────┬───────┘
          │                                   │ VkSurface request
          │ window / input / time             ▼
          └────────────▶ ┌──────────────────────────┐
                         │   platform_win32.cpp     │
                         │  (Win32: window, input)  │
                         └──────────────────────────┘
```

**The key boundary:** even with one backend, `app.cpp` does **not** touch Vulkan. It
builds a plain `Scene` struct — camera, an array of mesh instances referencing
material/texture *handles*, and lights — then calls `renderer_render(scene)`. This is
not an abstraction layer; it is a clean module API. It lets the renderer internals
churn freely toward GPU-driven rendering without ever breaking `app.cpp`.

---

## 4. Key Technical Decisions

Two of these are *timing* decisions — Vulkan punishes both extremes — so the spec
names the exact milestone each thing arrives.

**4.1 GPU memory allocation.** Vulkan makes you sub-allocate; `vkAllocateMemory` is
capped at ~4096 total allocations, so one-alloc-per-resource does not scale.
- M2: dead-simple — one `VkDeviceMemory` per resource. Teaches memory types &
  requirements.
- M3–M4: write a small **block sub-allocator** (~200 lines): allocate large chunks,
  hand out offset ranges. Educational and core to *getting* Vulkan.
- Escape hatch: if it becomes a time sink, drop in **VMA** (`vk_mem_alloc.h`, single
  header). You will understand it because you wrote v1.

**4.2 Descriptors → bindless.** A timing decision.
- M1–M3: classic descriptor sets / layouts / pools — core Vulkan literacy, do not
  skip it.
- M4: switch to a **bindless global descriptor set** (`VK_EXT_descriptor_indexing`,
  core in Vulkan 1.2): one large set holding all textures and buffers as arrays;
  shaders index by integer handle. Retrofitting this later is painful, and
  **GPU-driven rendering requires it** — GPU-generated draws cannot rebind
  descriptors. M4 is late enough to know the basics, early enough to avoid a rewrite.

**4.3 Synchronization & passes — hardcode first, render graph when it hurts.**
- M0–M7: hand-written `VkImageMemoryBarrier`s and an explicit, hardcoded pass list.
  You learn the GPU pipeline this way. No speculative abstraction.
- M8: with depth-prepass + shadow + main + post, barrier bookkeeping becomes
  error-prone. *Then* build a lightweight **render graph**: passes declare resource
  reads/writes; the graph derives barriers and aliases transient images. Abstraction
  earned from real pain.

**4.4 Shaders & iteration.** GLSL → SPIR-V via `glslc` as a CMake step. Add **shader
hot-reload** at M3 (watch the file, recompile, rebuild the pipeline at runtime) — the
single most motivating Handmade-Hero-style feature for a renderer.

**4.5 Validation (non-negotiable).** Vulkan **validation layers always on in debug**.
**RenderDoc** for frame capture from M1 onward.

---

## 5. Milestone Roadmap

Every rung is a runnable, demoable renderer.

| #    | Milestone        | What works at the end                                              | New concepts |
|------|------------------|--------------------------------------------------------------------|--------------|
| M0   | Bring-up         | Win32 window + loop; Vulkan instance/device/swapchain; clear screen | Platform seam, Vulkan init, validation layers |
| M1   | First triangle   | Graphics pipeline, SPIR-V shaders, one draw call                   | Pipelines, render passes, shader compilation, RenderDoc |
| M2   | 3D + camera      | Vertex/index buffers, depth buffer, MVP, spinning cube             | `math.hpp`, depth testing, memory alloc v1 |
| M3   | Textures + light | Textured, simply-lit mesh; shader hot-reload                       | Descriptor sets, image layouts/barriers, samplers, block allocator |
| M4   | glTF assets      | Real multi-mesh/multi-material model; bindless descriptors         | cgltf, asset/handle system, descriptor indexing |
| M5 ⭐ | PBR              | Metallic-roughness BRDF, normal mapping, HDR + tonemapping         | The PBR lighting model |
| M6   | IBL              | Env cubemap → irradiance + prefiltered specular + BRDF LUT         | First compute shaders, cubemap rendering |
| M7   | Shadows          | Directional shadow map, then cascades                              | Shadow techniques, multi-view rendering |
| M8   | Render graph     | Depth-prepass + shadow + main + post, auto-managed barriers        | Frame-graph design, transient aliasing |
| M9 ⭐ | GPU-driven       | Scene in GPU buffers, compute culling, `vkCmdDrawIndexedIndirectCount` | The modern GPU-driven pipeline |
| M10+ | Open-ended       | Clustered lighting · deferred / visibility-buffer · SSAO · GI · ray tracing | Pick by curiosity |

A **debug overlay** (immediate-mode line/text draw + frame timings) is worth slipping
in around M3 — you will use it to diagnose every later milestone.

---

## 6. Per-Milestone Concept Guide

High-level concept map. Each milestone gets its own detailed build guide (concept →
why → how-to → pitfalls → done-criteria, **no code**) written when you reach it.

**M0 — Bring-up.** *Concept:* Vulkan is explicit — you construct the device stack
yourself. *How:* create a Win32 window and message loop; create `VkInstance` (with
validation layers in debug); enumerate physical devices and pick one with a graphics
+ present queue; create the logical device and swapchain; acquire an image, clear it,
present. *Pitfall:* swapchain recreation on window resize is mandatory, not optional.
*Done when:* a colored window, zero validation errors.

**M1 — First triangle.** *Concept:* a pipeline bakes nearly all GPU state into one
immutable object. *How:* write a trivial vertex+fragment shader, compile to SPIR-V;
create a render pass describing the swapchain attachment; build the graphics
pipeline; record a command buffer that begins the pass and issues one `vkCmdDraw`.
*Pitfall:* forgetting dynamic viewport/scissor, or a vertex output the fragment stage
does not consume. *Done when:* a triangle, and you can capture it in RenderDoc.

**M2 — 3D + camera.** *Concept:* geometry lives in GPU buffers; transforms move it.
*How:* upload vertex/index buffers; add a depth attachment; write `math.hpp` (mat4,
perspective, lookAt); pass an MVP via push constants; render a spinning cube.
*Pitfall:* Vulkan's clip space (Y down, depth 0–1) differs from OpenGL — your
projection matrix must account for it. *Done when:* a depth-correct spinning cube.

**M3 — Textures + light.** *Concept:* descriptors bind resources to shaders; images
have layouts that must be transitioned. *How:* load a texture with `stb_image`, copy
through a staging buffer, transition layouts with barriers; create a sampler and a
descriptor set; add simple Blinn-Phong lighting; build a block sub-allocator; add
shader hot-reload. *Pitfall:* wrong image layout or missing barrier before
sampling — validation will tell you. *Done when:* a textured, lit mesh; editing a
shader updates the image live.

**M4 — glTF assets.** *Concept:* a real asset has many meshes, materials, textures —
you need a handle system and bindless access. *How:* parse glTF with `cgltf`; build
an asset manager that returns opaque handles; create one bindless global descriptor
set with arrays of all textures; shaders index materials by integer. *Pitfall:*
glTF's coordinate/winding conventions; descriptor-indexing feature flags must be
enabled at device creation. *Done when:* a multi-material model renders correctly.

**M5 — PBR. ⭐** *Concept:* PBR models light with a physically-grounded BRDF instead
of ad-hoc terms. *How:* implement the Cook-Torrance metallic-roughness BRDF
(GGX distribution, Smith geometry, Fresnel-Schlick); sample base-color / metallic-
roughness / normal maps; do normal mapping in tangent space; render to an HDR target
and tonemap to the swapchain. *Pitfall:* color-space mistakes — base color is sRGB,
normal/metallic-roughness are linear. *Done when:* a model with correct PBR response
to a directional light.

**M6 — IBL.** *Concept:* ambient light comes from the environment, precomputed into
lookups. *How:* load an HDR equirectangular map, project to a cubemap; convolve an
irradiance map (diffuse) and a roughness-mipped prefiltered map (specular); generate
the split-sum BRDF LUT — all via compute shaders. *Pitfall:* cubemap face
orientation and mip count. *Done when:* the model is lit plausibly with no analytic
light.

**M7 — Shadows.** *Concept:* a light sees the scene from its own viewpoint; depth
from that view tells you what is occluded. *How:* render scene depth from the light
into a depth texture; sample it in the main pass with a depth comparison; add
cascades for a directional light. *Pitfall:* shadow acne and peter-panning — needs
bias tuning and front/back-face strategy. *Done when:* correct contact shadows.

**M8 — Render graph.** *Concept:* declaring resource usage lets barriers and
transient memory be derived automatically. *How:* model passes as nodes that declare
read/write resources; topologically order them; derive `VkImageMemoryBarrier`s from
producer→consumer edges; alias transient images that do not overlap in time.
*Pitfall:* over-engineering — keep it minimal, just enough to kill manual barrier
bugs. *Done when:* the existing passes run through the graph with no hand-written
barriers and identical output.

**M9 — GPU-driven. ⭐** *Concept:* the GPU decides what to draw — the CPU stops
issuing per-object draw calls. *How:* upload the whole scene (transforms, bounds,
mesh metadata) into GPU buffers; a compute shader does frustum (then occlusion)
culling and writes surviving draws into an indirect-args buffer; render with
`vkCmdDrawIndexedIndirectCount`. Requires the bindless setup from M4. *Pitfall:*
CPU↔GPU sync on the indirect buffer; getting the draw-count buffer right. *Done
when:* thousands of objects, culling done on the GPU, near-constant CPU cost.

**M10+ — Open-ended.** Clustered/tiled lighting for many lights; a deferred or
visibility-buffer pass; SSAO; real-time GI (screen-space, voxel, or `VK_KHR_ray_tracing`).
Each gets its own design pass when you choose it.

---

## 7. Testing & Validation Strategy

Renderers resist traditional unit testing — the strategy is layered by what is
actually testable.

| Layer | Covers | Tooling | From |
|-------|--------|---------|------|
| Validation layers | API misuse, sync errors, leaks | Vulkan validation, always on in debug | M0 |
| Math unit tests | `math.hpp` — pure & deterministic | assert-based `tests` build target | M2 |
| Frame debugging | "Why is it black?" | RenderDoc capture | M1 |
| Golden-image tests | Visual regressions | offscreen render vs saved reference, perceptual diff with tolerance | M5 (optional) |
| GPU profiling | Per-pass timing, bottlenecks | `VkQueryPool` timestamps → debug overlay; RenderDoc/Nsight | M8 |

**Principles:**
- The renderer's real test is the screen. A milestone is "done" when it renders
  correctly *and* a RenderDoc capture shows clean barriers and zero validation
  warnings — make that the per-milestone definition of done.
- Only write assert-based tests for deterministic code: math, asset parsing, the
  allocator free-list, the handle system. Shading output gets golden images, not
  asserts.
- Golden images need a fixed seed: a canonical camera + scene + frame rendered to an
  offscreen target. Do not add before M5 — nothing is stable enough to baseline.
- Treat any validation error like a compile error: fix before moving on. This single
  habit prevents most multi-hour Vulkan debugging horror stories.

---

## 8. Open Questions / Future

- Linux port: the platform seam stays clean for it, but it is explicitly out of scope
  until at least M5.
- A second backend (OpenGL/D3D12) is out of scope. If ever wanted, extract it from
  the finished Vulkan renderer — by then there is one real case to abstract from.
- M10+ direction is deliberately undecided; chosen by curiosity when M9 lands.
