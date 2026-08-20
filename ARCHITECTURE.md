# V.I.I.B.E CS 1.6 Modular Architecture & Diagnostic Guide

## 1. Directory Structure

```
cs16_esp_diagnostic/
├── .gitignore                          # Excludes compiled binaries, build/ outputs, logs
├── build_internal.bat                  # Automated compiler script (MinGW32)
├── injector.cpp                        # Dedicated remote thread injector
├── dllmain.cpp                         # DLL entry point (delegates to Framework)
├── ARCHITECTURE.md                     # Modular layer specification
├── README.md                           # Main documentation
└── src/
    ├── sdk/                            # GoldSrc Engine Types & Structs
    │   └── sdk.hpp                     # ref_params_t, studiohdr_t, PlayerData, MenuState
    ├── core/                           # Core Framework, Input, Math & Telemetry
    │   ├── framework.hpp / framework.cpp # Engine/DLL lifecycle, VEH crash handler & main loop
    │   ├── input.hpp / input.cpp       # Key tracking & input abstractions
    │   ├── math.hpp / math.cpp         # WorldToScreen, WorldToRadar, Vector math
    │   └── logger.hpp / logger.cpp     # File logger & live diagnostic snapshot dumper
    ├── render/                         # OpenGL Render Abstraction Layer
    │   └── renderer.hpp / renderer.cpp # Ortho 2D setup, primitive batching, bitmap font
    ├── engine/                         # Memory Safety & GoldSrc Interface Sub-Modules
    │   ├── memory.hpp / memory.cpp     # Safe memory reading & pointer validation
    │   ├── resolver.hpp / resolver.cpp # Engine function binding & raytracing
    │   ├── studio.hpp / studio.cpp     # Studio model header parsing & hitbox cache
    │   ├── player.hpp / player.cpp     # Player reading, skeleton generation & team resolution
    │   ├── entity.hpp / entity.cpp     # World entity tracking, dropped weapons, C4 & grenades
    │   └── engine.hpp / engine.cpp     # Unified Engine facade header
    ├── hooks/                          # Engine & Graphics Hooks
    │   └── hooks.hpp / hooks.cpp       # wglSwapBuffers hook, hw.dll dispatch table slots
    └── features/                       # Modular Feature Implementations
        ├── esp.hpp / esp.cpp           # 2D Boxes, Health bars, Weapon/Distance info, Chams
        ├── aimbot.hpp / aimbot.cpp     # Target calculation, RCS, smoothing, triggerbot
        ├── radar.hpp / radar.cpp       # 2D tactical rotating radar
        ├── misc.hpp / misc.cpp         # Auto bunnyhop, C4 & Grenade tracker
        ├── config.hpp / config.cpp     # INI parser & state synchronization
        └── menu.hpp / menu.cpp         # Multi-tab interactive GUI & telemetry HUD
```

---

## 2. Module Responsibilities

### `src/sdk/` (Software Development Kit)
- **`sdk.hpp`**: Contains exact GoldSrc structures (`ref_params_t`, `studiohdr_t`, `mstudiobone_t`, `mstudiohitbox_t`, `hud_player_info_t`), internal player representations (`PlayerData`, `WorldEntityData`), and UI state structures (`MenuState`, `PanelState`).

### `src/core/` (Core Framework & Telemetry)
- **`framework.hpp / .cpp`**: Manages DLL injection lifecycle, initializes logging and configuration, sets up Vectored Exception Handler (VEH) crash trap, resolves engine hooks, and manages the main worker loop.
- **`input.hpp / .cpp`**: Tracks async key states and toggle hits (`KeyTracker`).
- **`math.hpp / .cpp`**: Implements 3D-to-2D projection (`WorldToScreen`) factoring in dynamic FOV and aspect ratios. Implements rotational 2D coordinate transformation for radar (`WorldToRadar`).
- **`logger.hpp / .cpp`**: Handles file logging with thread safety and outputs periodic runtime diagnostic telemetry reports.

### `src/render/` (OpenGL Drawing Engine)
- **`renderer.hpp / .cpp`**: Manages orthographic 2D projection matrix setup (`Begin2D` / `End2D`), rendering state backup/restore, batch geometry primitives (boxes, outlines, circles, filled gradients), and pixel-perfect built-in 8x8 font rasterizer.

### `src/engine/` (GoldSrc Engine Sub-Modules)
- **`memory.hpp / .cpp`**: Provides memory safety primitives (`SafeReadBytes`, `IsReadableFast`, `SafeReadString`).
- **`resolver.hpp / .cpp`**: Resolves `gEngfuncs` function pointers, `hw.dll` offsets, and ray-tracing visibility check (`IsTargetVisible`).
- **`studio.hpp / .cpp`**: Parses GoldSrc studio headers, retrieves hitbox bounds, and maintains zero-allocation model caching.
- **`player.hpp / .cpp`**: Reads player state from memory, reconstructs 3D skeleton joints, resolves player teams, and calculates distance metrics.
- **`entity.hpp / .cpp`**: Tracks world items (C4, dropped weapons, grenades) and parses weapon display names.
- **`engine.hpp / .cpp`**: Acts as a unified facade for backward compatibility across all modules.

### `src/hooks/` (Interception Layer)
- **`hooks.hpp / .cpp`**: 
  - **`wglSwapBuffers`**: Inline 5-byte hook on OpenGL swap buffers for frame-synchronized rendering.
  - **`V_CalcRefdef`**: Hooks the `hw.dll` dispatch slot to capture camera origin, angles, and dynamic zoom FOV.
  - **`HUD_AddEntity`**: Hooks the entity dispatch slot to monitor visible render entities and world items (C4, grenades).

### `src/features/` (Functional Subsystems)
- **`esp.hpp / .cpp`**: In-game overlays including 2D bounding boxes, corner boxes, player health bars, distance tags, weapon names, and bone markers.
- **`aimbot.hpp / .cpp`**: Calculates view angle deltas, applies smoothing algorithms, and adjusts angles during camera calculation frames.
- **`radar.hpp / .cpp`**: Renders rotating 2D radar overlay with orientation aligned to local player yaw.
- **`misc.hpp / .cpp`**: Implements bunnyhop assistance and tracks active dropped/planted C4 and airborne grenades.
- **`config.hpp / .cpp`**: Serializes and deserializes cheat settings to `viibe_config.ini`.
- **`menu.hpp / .cpp`**: Renders modular draggable tab panels (Aimbot, Visuals, Radar, Misc, Themes, Config) with interactive controls.
