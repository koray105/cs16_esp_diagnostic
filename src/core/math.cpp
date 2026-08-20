#include "math.hpp"

namespace Math {
    Vec3     g_camPos         = {0, 0, 0};
    Vec3     g_camAngles      = {0, 0, 0};
    Vec3     g_camForward     = {0, 0, 0};
    Vec3     g_camRight       = {0, 0, 0};
    Vec3     g_camUp          = {0, 0, 0};
    float    g_camFov         = 90.0f;
    bool     g_camValid       = false;
    bool     g_localOnGround  = true;
    bool     g_camSpectator   = false;
    int      g_camViewEntity  = 0;
    uint64_t g_refdefFrames   = 0;

    ProjectionCache g_proj = {};

    void UpdateProjection(int screenW, int screenH, float fov) {
        if (screenW <= 0 || screenH <= 0) {
            g_proj.valid = false;
            return;
        }

        float currentFov = fov;
        if (currentFov <= 0.0f || currentFov > 170.0f) currentFov = 90.0f;

        // Check if cache already matches
        if (g_proj.valid && g_proj.screenW == screenW && g_proj.screenH == screenH && fabsf(g_proj.fov - currentFov) < 0.01f) {
            return;
        }

        g_proj.screenW = screenW;
        g_proj.screenH = screenH;
        g_proj.fov     = currentFov;
        g_proj.halfW   = (float)screenW * 0.5f;
        g_proj.halfH   = (float)screenH * 0.5f;

        float aspect = (float)screenW / (float)screenH;
        float halfFovRad = (currentFov * 0.5f) * DEG2RAD;
        float tanHalfFov = tanf(halfFovRad);

        // GoldSrc standard Hor+ projection scaling:
        // tan(fov_y / 2) = tan(base_fov / 2) * (3 / 4)
        // tan(fov_x / 2) = tan(fov_y / 2) * aspect_ratio
        float tanHalfFovY = tanHalfFov * 0.75f;
        float tanHalfFovX = tanHalfFovY * aspect;

        if (tanHalfFovX > 0.0001f && tanHalfFovY > 0.0001f) {
            g_proj.invTanHalfFovX = 1.0f / tanHalfFovX;
            g_proj.invTanHalfFovY = 1.0f / tanHalfFovY;
            g_proj.valid = true;
        } else {
            g_proj.invTanHalfFovX = 1.0f;
            g_proj.invTanHalfFovY = 1.0f;
            g_proj.valid = false;
        }
    }

    bool WorldToScreen(const Vec3& target, Vec2& screen, int screenW, int screenH, float* outDist, const char** outReason) {
        if (!g_camValid) {
            if (outReason) *outReason = "Camera invalid / Refdef not populated";
            return false;
        }
        if (!target.IsValid()) {
            if (outReason) *outReason = "Target vector contains NaN/Inf/Out-of-bounds";
            return false;
        }
        if (!g_camPos.IsValid()) {
            if (outReason) *outReason = "Camera position contains NaN/Inf";
            return false;
        }

        // Auto-update projection cache if viewport or FOV changed
        if (!g_proj.valid || g_proj.screenW != screenW || g_proj.screenH != screenH || fabsf(g_proj.fov - g_camFov) > 0.01f) {
            UpdateProjection(screenW, screenH, g_camFov);
            if (!g_proj.valid) {
                if (outReason) *outReason = "Invalid viewport or projection parameters";
                return false;
            }
        }

        float dx = target.x - g_camPos.x;
        float dy = target.y - g_camPos.y;
        float dz = target.z - g_camPos.z;

        float zDist = dx * g_camForward.x + dy * g_camForward.y + dz * g_camForward.z;
        if (outDist) *outDist = zDist;

        // Clip vertices behind camera or too close to near plane (zDist <= 1.0 unit)
        if (zDist <= 1.0f) {
            if (outReason) *outReason = "Behind camera or near plane clipped (zDist <= 1.0)";
            return false;
        }

        float xDist = dx * g_camRight.x + dy * g_camRight.y + dz * g_camRight.z;
        float yDist = dx * g_camUp.x    + dy * g_camUp.y    + dz * g_camUp.z;

        float invZ = 1.0f / zDist;

        // Fast zero-trig projection using precalculated inverse tangents
        float ndcX = (xDist * invZ) * g_proj.invTanHalfFovX;
        float ndcY = (yDist * invZ) * g_proj.invTanHalfFovY;

        screen.x = g_proj.halfW * (1.0f + ndcX);
        screen.y = g_proj.halfH * (1.0f - ndcY);

        if (outReason) *outReason = "Success";
        return true;
    }

    bool WorldToRadarFast(float dx, float dy, float cosYaw, float sinYaw, float scale,
                          float radarCenterX, float radarCenterY, float radarRadius,
                          Vec2& outRadarScreen, bool& outClamped) {
        float forwardDist = dx * cosYaw + dy * sinYaw;
        float rightDist   = dx * sinYaw - dy * cosYaw;

        float radarX = rightDist * scale;
        float radarY = -forwardDist * scale;

        float dist2DSq = radarX * radarX + radarY * radarY;
        float maxRadius = radarRadius - 4.0f;
        float maxRadiusSq = maxRadius * maxRadius;

        outClamped = false;
        if (dist2DSq > maxRadiusSq) {
            outClamped = true;
            float dist2D = sqrtf(dist2DSq);
            if (dist2D > 0.001f) {
                float invDist = maxRadius / dist2D;
                radarX *= invDist;
                radarY *= invDist;
            }
        }

        outRadarScreen.x = radarCenterX + radarX;
        outRadarScreen.y = radarCenterY + radarY;
        return true;
    }

    bool WorldToRadar(const Vec3& target, const Vec3& localPos, float localYawDeg,
                      float radarCenterX, float radarCenterY, float radarRadius,
                      float maxRangeUnits, Vec2& outRadarScreen, bool& outClamped) {
        if (maxRangeUnits <= 10.0f || radarRadius <= 5.0f) return false;
        if (!target.IsValid() || !localPos.IsValid()) return false;

        float dx = target.x - localPos.x;
        float dy = target.y - localPos.y;

        float yawRad = localYawDeg * DEG2RAD;
        float cosYaw = cosf(yawRad);
        float sinYaw = sinf(yawRad);
        float scale  = radarRadius / maxRangeUnits;

        return WorldToRadarFast(dx, dy, cosYaw, sinYaw, scale, radarCenterX, radarCenterY, radarRadius, outRadarScreen, outClamped);
    }

    void AngleVectors(const Vec3& angles, Vec3& forward, Vec3& right, Vec3& up) {
        float angleY = angles.y * DEG2RAD;
        float sy = sinf(angleY), cy = cosf(angleY);
        float angleX = angles.x * DEG2RAD;
        float sp = sinf(angleX), cp = cosf(angleX);
        float angleZ = angles.z * DEG2RAD;
        float sr = sinf(angleZ), cr = cosf(angleZ);

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;

        right.x = (-sr * sp * cy + cr * sy);
        right.y = (-sr * sp * sy - cr * cy);
        right.z = -sr * cp;

        up.x = (cr * sp * cy + sr * sy);
        up.y = (cr * sp * sy - sr * cy);
        up.z = cr * cp;
    }

    void NormalizeAngles(float& pitch, float& yaw) {
        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        yaw = fmodf(yaw + 180.0f, 360.0f);
        if (yaw < 0.0f) yaw += 360.0f;
        yaw -= 180.0f;
    }

    const char* GetAspectRatioName(int w, int h) {
        if (w <= 0 || h <= 0) return "Unknown";
        float aspect = (float)w / (float)h;
        if (fabsf(aspect - (16.0f / 9.0f)) < 0.04f) return "16:9 Wide";
        if (fabsf(aspect - (16.0f / 10.0f)) < 0.04f) return "16:10 Wide";
        if (fabsf(aspect - (4.0f / 3.0f)) < 0.04f) return "4:3 Standard";
        if (fabsf(aspect - (5.0f / 4.0f)) < 0.04f) return "5:4 Classic";
        if (fabsf(aspect - (21.0f / 9.0f)) < 0.08f) return "21:9 Ultrawide";
        if (fabsf(aspect - (32.0f / 9.0f)) < 0.08f) return "32:9 Superwide";
        return "Custom Widescreen";
    }

    float GetEffectiveHorizontalFov(float baseFov, int w, int h) {
        if (w <= 0 || h <= 0) return baseFov;
        float aspect = (float)w / (float)h;
        float halfFovRad = (baseFov * 0.5f) * DEG2RAD;
        float tanHalfFovY = tanf(halfFovRad) * 0.75f;
        float tanHalfFovX = tanHalfFovY * aspect;
        return 2.0f * atanf(tanHalfFovX) * RAD2DEG;
    }

    float GetUiScale(int h) {
        if (h <= 0) return 1.0f;
        float scale = (float)h / 720.0f;
        if (scale < 1.0f) return 1.0f;
        if (scale > 2.5f) return 2.5f;
        return scale;
    }
}


