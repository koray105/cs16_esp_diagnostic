---
name: goldsrc-reverse-engineering
description: Complete blueprint for safe GoldSrc (CS 1.6) internal hooks, zero-lag entity tracking, instant death gates, and dynamic zoom FOV matrix projections.
---

# GoldSrc Reverse-Engineering & Internal Diagnostics Guide

## 1. Crash-Proof Dispatch Table Hooking
- Avoid raw inline trampolines on small functions in client.dll due to relative branches (`jz`, `call`).
- Hook `hw.dll`'s `cldll_func_t` table via pointer swapping:
  - `V_CalcRefdef`: `hw.dll + 0x11FE36C`
  - `HUD_AddEntity`: `hw.dll + 0x11FE370`

## 2. Verified Engine Indices (`cl_enginefuncs_s`)
- Index 21 (`+0x54`): `pfnGetPlayerInfo`
- Index 51 (`+0xCC`): `pfnGetLocalPlayer` (returns `cl_entity_t*`)
- Index 53 (`+0xD4`): `pfnGetEntityByIndex` (returns `cl_entity_t*`)
- Team resolution: `client.dll::HUD_GetPlayerTeam` export (RVA `0x45350`) & `g_PlayerExtraInfo` (`client.dll + 0x12B2F4`, stride 104 bytes).

## 3. Zero-Lag Live Render Synchronization
- Direct read from `cl_entity_t + 0x02C0` during `wglSwapBuffers` for live interpolated position.
- Bone apex head tracking: `cl_entity_t + 0x02D8` (`attachment[0]`).
- Ducking detection: `cl_entity_t + 0x00A0` (`usehull == 1`).

## 4. 0ms Instant Death & Ghost Suppression
- Check `curstate.sequence >= 101` (GoldSrc death animations 101..125).
- Check `curstate.solid == 0` (`SOLID_NOT`).
- Check `extra_player_info_t.dead != 0` (`client.dll + 0x12B2F4 + index*104 + 0x44`).
- Check `ref_params_t->intermission != 0` for round transitions.

## 5. Optical Zoom / Scoped FOV Projection
- Read live integer FOV from `client.dll + 0x11D490` (Normal=90, AUG/SG552=55, AWP=40/10).
- Scale projection: `tanHalfFovY = tan(FOV * 0.5 * PI / 180) * 0.75`.
- Calculate NDC: `tanHalfFovX = tanHalfFovY * AspectRatio`.
