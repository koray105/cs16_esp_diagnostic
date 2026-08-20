#pragma once

#include "../sdk/sdk.hpp"

namespace Math {
    extern Vec3  g_camPos;
    extern Vec3  g_camAngles;
    extern Vec3  g_camForward;
    extern Vec3  g_camRight;
    extern Vec3  g_camUp;
    extern float g_camFov;
    extern bool  g_camValid;
    extern bool  g_localOnGround;
    extern bool  g_camSpectator;
    extern int   g_camViewEntity;
    extern uint64_t g_refdefFrames;

    // Fast Precalculated Projection Cache
    struct ProjectionCache {
        int   screenW = 0;
        int   screenH = 0;
        float fov = 0.0f;
        float halfW = 0.0f;
        float halfH = 0.0f;
        float invTanHalfFovX = 0.0f;
        float invTanHalfFovY = 0.0f;
        bool  valid = false;
    };
    extern ProjectionCache g_proj;

    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;

    void UpdateProjection(int screenW, int screenH, float fov);
    bool WorldToScreen(const Vec3& target, Vec2& screen, int screenW, int screenH, float* outDist = nullptr, const char** outReason = nullptr);
    bool WorldToRadar(const Vec3& target, const Vec3& localPos, float localYawDeg,
                      float radarCenterX, float radarCenterY, float radarRadius,
                      float maxRangeUnits, Vec2& outRadarScreen, bool& outClamped);
    bool WorldToRadarFast(float dx, float dy, float cosYaw, float sinYaw, float scale,
                          float radarCenterX, float radarCenterY, float radarRadius,
                          Vec2& outRadarScreen, bool& outClamped);

    void AngleVectors(const Vec3& angles, Vec3& forward, Vec3& right, Vec3& up);
    void NormalizeAngles(float& pitch, float& yaw);
    const char* GetAspectRatioName(int w, int h);
    float GetEffectiveHorizontalFov(float baseFov, int w, int h);
    float GetUiScale(int h);
}


