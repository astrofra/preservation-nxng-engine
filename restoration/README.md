# Restoration Workspace

This folder contains the macOS-oriented restoration path:

- `compat/`: inferred compatibility shims for missing legacy modules (`IO`, `xPack`, image wrappers, input/resource stubs).
- `nxng_cli/`: headless command-line scene loader/viewer.

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Run

```bash
./build/restoration/nxng-cli --scene demos/vestige/scene.lws
```

Useful options:

```bash
./build/restoration/nxng-cli --scene demos/fra_tanks/scene_hnoize.lws --no-window
```
