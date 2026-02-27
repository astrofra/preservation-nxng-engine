# nXng Preservation Audit and Restoration Plan

Date: 2026-02-27  
Repository: `preservation-nxng-engine`

## Scope Checked

- Source tree: `src/` (83 files, 52 `.cpp`, 28 `.h`)
- Documentation: `doc/` (15 files) + top-level `README.md` and `release-notes.md`
- Demos/assets: `demos/` (70 files across `fra_tanks` and `vestige`)
- Legacy release archives: `releases/*.zip`

## Executive Summary

This repository is a partial preservation snapshot, not a buildable project yet.

Main blockers to "build then run":

1. No build system/project files are present (`.dsp`, `.vcproj`, `CMakeLists.txt`, etc.).
2. Multiple required code dependencies are missing (`IO/*`, `xPack/*`, `cInput/*`, image codec headers).
3. App shell pieces are missing (resource headers/RC assets, and no `main`/`WinMain` entrypoint in the snapshot).
4. Some source files are empty placeholders.
5. Code and data contain hardcoded legacy Windows paths.

## Status Update (2026-02-27)

Phase 1 is completed: preserved `nxng.exe` runs on Windows.

Phase 2 and Alternate Path kickoff are now started in this repository with a new staged restoration workspace under `restoration/`.

- Added a CMake bring-up path that builds:
  - `nxng_compat` (inferred compatibility layer for missing modules)
  - `nxng-cli` (macOS-oriented CLI scene loader/viewer path)
- Added inferred implementations for missing symbol groups used by legacy code:
  - `IO` (`qFileIO`, `cqString`, `MCIO`, `BitConvertIO`)
  - `xPack` (`xLink`)
  - image wrappers (`IMG`, `generic`, `JPEG`, `TARGA` stub)
  - `cInput` stub
- Started A2/A3/A4:
  - A2: runtime path no longer depends on Win32 dialogs or `.rc` resources in the CLI flow
  - A3: OpenGL backend path implemented for macOS via GLFW
  - A4: first modernization fixes merged (`sizeof(Type)`, include-path cleanup, local `resource.h` shim)

Verified from this repository on macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j 8
./build/restoration/nxng-cli --scene demos/vestige/scene.lws --no-window
./build/restoration/nxng-cli --scene demos/fra_tanks/scene_hnoize.lws --no-window
./build/restoration/nxng-cli --scene demos/fra_tanks/scene_volume_spot_combo.lws --no-window
```

Observed loader results:

- `vestige/scene.lws`: 1 object, 4257 vertices, 6130 triangles
- `fra_tanks/scene_hnoize.lws`: 6 objects, 413 vertices, 644 triangles
- `fra_tanks/scene_volume_spot_combo.lws`: 36 objects, 12178 vertices, 8770 triangles

## Concrete Findings

### 1) Missing build metadata

- No build scripts or project files were found.
- A direct compile probe fails immediately:
  - `clang++ ... src/Nx_core.cpp` -> missing `nXng/nX_HeaderList.h` include mapping.
  - With include remap, compile then fails on missing Windows/DirectX headers (`dinput.h`).

### 2) Missing external code dependencies (hard blockers)

Quoted includes referenced by source but absent from this repository:

- `IO/qFileIO.h`
- `IO/cqString.h`
- `IO/MCIO.h`
- `IO/mcIO.h`
- `IO/BitConvertIO.h`
- `IO\BitConvertIO.h`
- `IO\CqString.h`
- `xPack/xPack.h`
- `cInput/cInput.h`
- `IMG.h`
- `TARGA.h`
- `JPEG.h`
- `generic.h`
- `d:\devellop\nX_S3DE\resource.h`

System/SDK dependencies (legacy Windows):

- `windows.h`, `windowsx.h`, `winuser.h`, `commctrl.h`
- `ddraw.h`, `d3d.h`, `dinput.h`

### 3) Missing app bootstrap/UI resources

- No `main` or `WinMain` symbol exists in `src/`.
- GUI and screen code require resource IDs/macros from missing `resource.h`.
- No `.rc` file or resource source set is present.

### 4) Incomplete/empty source units

Zero-length files found:

- `src/g_nX.cpp`
- `src/g_nX.h`
- `src/nX_Radiosity.cpp`
- `src/nX_SparkFaery[0].cpp`

Near-empty placeholders:

- `src/nX_Gizmo.cpp` (header banner only)
- `src/nX_MDPlug.h` (header banner only)

These indicate missing or dropped parts of the original codebase.

### 5) Legacy path and portability issues

Hardcoded absolute paths in engine code:

- `d:\devellop\nx_pics\flare.jpg`
- `d:\devellop\nx_pics\spark.jpg`
- `d:\devellop\nx_pics\defaultevmap.jpg`

Hardcoded resource include path:

- `d:\devellop\nX_S3DE\resource.h`

Demo scene and playlist paths are absolute Windows paths (`C:\...`, `S:\...`), although object lookup can succeed by basename in the bundled demo folders.

Include case/path conventions are also legacy-Windows-centric (for example `nXng/cMath.h` vs tracked file `Cmath.h`), which will break on case-sensitive filesystems unless normalized.

### 6) Source compatibility issues with modern compilers

At least some code relies on old compiler tolerance (example: `sizeof Type` form without parentheses), which fails on modern Clang/GCC and must be modernized.

### 7) Documentation/demo integrity notes

- `doc/nxng/index.htm` references missing files: `faq.htm`, `misc.htm`, `nxng.zip`.
- Demo asset check:
  - LWS object references resolve by basename to local `.lwo` files.
  - Embedded texture names found in `.lwo` assets resolve by basename to local images.
- MOA files:
  - Present for most scenes; `demos/fra_tanks/scene_volume_spot.lws` has no sibling `.moa` (not fatal; engine can recompute MOA).

## Effort Estimate

### A) "Run preserved binary" effort (fastest)

Target: run historical `nxng.exe` with bundled demos, without rebuilding source.

- Estimated effort: 0.5 to 1.5 days
- Main tasks:
  - Prepare Windows environment (or Wine/VM with old DirectX compatibility).
  - Use `releases/nxng.zip` (`nxng.exe`, `Bass.dll`, `nX.KLX`).
  - Place demos, adjust playlist/paths as needed.
  - Provide `nXng.ini` (`[NEWTEK]` base path).

### B) "Full source rebuild and run" effort (restoration)

Target: build executable from source and run demos.

- Estimated effort: 3 to 8 weeks (single engineer), depending on how much missing code can be recovered externally.
- Main cost drivers:
  - Reconstructing missing IO/xPack/image/input modules.
  - Recreating resource/GUI bootstrap (`WinMain`, `.rc`, dialog/control IDs).
  - Legacy DirectX 7 API/toolchain setup and modernization fixes.
  - Runtime validation of loader behavior for LWS/LWO/LWOB content.

### C) "Alternate macOS headless viewer" effort (inference + port)

Target: infer missing pieces from usage, remove GUI/Win32 dependencies, and ship a macOS command-line tool that can load and display a Lightwave scene.

- Estimated effort: 6 to 14 weeks (single engineer), with uncertainty driven by missing logic in external modules.
- Main cost drivers:
  - Inference/reimplementation quality for missing symbols (`IO`, `xPack`, image stack).
  - Refactoring to isolate engine core from DirectX 7 and Win32.
  - Building a new renderer backend for OpenGL or Metal on macOS.
  - Behavioral mismatches introduced by guessed implementations.

## Recommended Restoration Plan

### Phase 1: Baseline runnable preservation (binary)

1. Build a reproducible run environment (Windows VM preferred for determinism).
2. Launch `nxng.exe` from release package with `Bass.dll`.
3. Validate `fra_tanks` and `vestige` scene playback.
4. Capture known-good runtime settings (resolution, renderer mode, MOA options).

Exit criteria: demos play from preserved binary.

### Phase 2: Source recovery inventory

1. Recover or re-implement missing modules:
   - `IO` library (`qFileIO`, `qString`, `MCIO`, `BitConvertIO`)
   - `xPack` (`xLinker`)
   - `cInput`
   - image pipeline (`IMG`, `GENERIC`, `JPEG`, `TARGA`)
2. Restore UI resources (`resource.h` + `.rc`) and app entrypoint.
3. Replace hardcoded asset paths with config-relative paths.

Exit criteria: project compiles to object files with no missing includes/symbol roots.

### Phase 3: Build system + toolchain pinning

1. Create reproducible build definition (CMake or frozen VS project).
2. Support two modes:
   - legacy-accurate build (Windows + DX7 headers)
   - modernization build (shimmed APIs, optional)
3. Patch compiler-compat issues (`sizeof(Type)`, strict casts, etc.).

Exit criteria: executable links.

### Phase 4: Runtime correctness and demo validation

1. Validate LWS loader, object/texture resolution, MOA generation/loading.
2. Verify both software and D3D render paths where possible.
3. Compare output behavior against release binary and archived video references.

Exit criteria: source-built binary runs both demo sets reliably.

## Alternate Path: macOS CLI Tool

This section describes the alternate strategy requested for producing a command-line scene viewer on macOS.

### Goal

- Deliver `nxng-cli` that loads at least one `.lws` scene and displays it in a window.
- Remove all `.rc`/GUI dependencies from the runtime path.
- Keep project buildable on macOS with a modern compiler.

### Phase A1: Infer and implement missing symbols

1. Build a missing-symbol inventory from compiler/linker failures.
2. Reconstruct minimal interfaces from call-site usage for:
   - `qString`
   - `qFileIO`
   - `MCIO`
   - `BitConvertIO`
   - `xLinker`
   - image wrappers (`IMG`, `GENERIC`, `JPEG`, `TARGA`)
3. Implement behavior-first versions sufficient for scene/object/texture load.
4. Mark every inferred implementation with a validation note (known/assumed/unknown behavior).

Exit criteria: source compiles far enough to reach renderer and main-loop integration points.

Current status:

- Started and partially completed in `restoration/compat`.
- Inferred implementations now compile and link for the CLI restoration path.
- Remaining: verify behavioral parity of inferred logic vs historical runtime.

Effort, issues, solution:

- Effort spent: ~2 to 3 engineer-days (initial bring-up).
- Main issues:
  - Missing private dependency code not present in snapshot.
  - Case/filename collisions from legacy include conventions.
  - Unknown edge-case behavior in original helper libraries.
- Implemented solution:
  - Added `nxng_compat` static library with minimal behavior-first replacements.
  - Kept interfaces aligned with legacy call sites.
  - Isolated inferred code so it can be swapped with recovered originals later.

### Phase A2: Remove GUI and `.rc` dependency

1. Drop or bypass `nX_GUI`, `nX_Console` dialog code, and resource-only paths.
2. Remove `resource.h` include usage from non-essential modules.
3. Replace GUI controls with CLI arguments:
   - scene path
   - full screen/windowed
   - renderer backend
   - MOA rebuild toggle
4. Provide text logging to stdout/stderr.

Exit criteria: engine can initialize and run without any Win32 dialog/resource assets.

Current status:

- Started for alternate CLI path and functionally achieved for `nxng-cli`.
- Legacy GUI modules still exist in the preserved source tree, but are bypassed by restoration target.

Effort, issues, solution:

- Effort spent: ~0.5 to 1 engineer-day.
- Main issues:
  - Multiple direct includes of missing legacy `resource.h` path.
  - Initialization logic originally coupled to Win32 UI flow.
- Implemented solution:
  - Added local `resource.h` shim and normalized includes.
  - Introduced CLI entrypoint (`--scene`, `--no-window`) with stdout/stderr logging.
  - Removed `.rc` dependency from the restoration runtime path.

### Phase A3: Port DX7 backend to macOS renderer

1. Introduce renderer abstraction boundary:
   - frame begin/end
   - texture upload/bind
   - draw primitive
   - render-state translation
2. Implement backend option 1: OpenGL (faster bring-up path on preserved fixed-function semantics).
3. Implement backend option 2: Metal (native long-term path; optional second step).
4. Keep software raster path available as fallback where possible.

Exit criteria: one backend renders visible geometry from loaded LWS/LWO scene on macOS.

Current status:

- Started with OpenGL bring-up path (GLFW + OpenGL).
- Metal backend not started yet.
- Current renderer draws triangulated LWOB geometry (wireframe path).

Effort, issues, solution:

- Effort spent: ~1 to 2 engineer-days.
- Main issues:
  - Original rendering path tightly bound to DX7-era assumptions.
  - No existing cross-platform render abstraction in snapshot.
- Implemented solution:
  - Added standalone CLI renderer module in `restoration/nxng_cli`.
  - Implemented LWS object discovery + LWOB parsing (`FORM/LWOB`, `PNTS`, `POLS`) and polygon triangulation.
  - Implemented OpenGL draw loop suitable for macOS build/run validation.

### Phase A4: Legacy modernization pass

1. Fix syntax/portability issues (`sizeof(Type)`, case-sensitive include paths, unsafe casts).
2. Replace legacy C-string patterns where high-risk (`strcpy`, unchecked `wsprintf`) with bounded variants.
3. Add compile flags and warnings baseline for Clang on macOS.
4. Normalize path handling (`/` and `\\`) and remove hardcoded absolute developer paths.

Exit criteria: clean, repeatable macOS build with warnings triaged.

Current status:

- Started with targeted unblockers only.
- Not yet a full modernization sweep.

Effort, issues, solution:

- Effort spent: ~0.5 engineer-day.
- Main issues:
  - Modern compilers reject legacy syntax forms and path conventions.
  - Windows-only include style breaks portability.
- Implemented solution:
  - Fixed `sizeof(Type)` portability issues in `src/Cmath.cpp`.
  - Normalized several legacy include paths (`\\` to `/`, local `resource.h` include).
  - Added compatibility headers in `restoration/compat/include/nXng` to absorb case/path differences.

### Phase A5: Deliver `nxng-cli` minimum viable viewer

1. Add entrypoint executable:
   - `nxng-cli --scene demos/vestige/scene.lws`
2. Provide basic camera controls and frame loop.
3. Verify at least one reference scene displays and advances in realtime.
4. Document known unsupported features (example: advanced lightmaps/effectors if still incomplete).

Exit criteria: command-line tool loads and displays a Lightwave scene on macOS.

### Risks specific to this alternate path

- Inferred implementations may diverge from original behavior and cause subtle replay differences.
- DX7 fixed-function assumptions may require non-trivial translation in modern APIs.
- Missing code in empty placeholder files may block full feature parity.
- Scope can expand quickly; strict MVP definition is required.

### Suggested MVP boundary for first delivery

- Include:
  - LWS/LWO load
  - texture decode for demo assets
  - camera + geometry rendering
  - one working backend on macOS
- Exclude in first milestone:
  - full GUI/playlists
  - full lightmap baking pipeline
  - less-used effectors with missing source behavior

## Practical Next Actions

1. Expand A1 coverage by moving remaining compile/link failures from legacy engine sources into `nxng_compat` or recovered modules.
2. Upgrade A3 renderer from wireframe MVP to textured triangles and basic lights, then decide whether Metal is required in phase scope.
3. Continue A4 modernization with a controlled pass on string APIs, include casing, and warning cleanup under Clang.
4. Add regression scenes and a scripted smoke test (`--no-window`) to keep CLI loading stable during recovery work.
