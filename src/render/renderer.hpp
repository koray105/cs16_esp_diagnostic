#pragma once

#include <windows.h>
#include <gl/gl.h>

namespace Render {
    void Init();
    bool Begin2D(int& outW, int& outH, HDC hdc = nullptr);
    void End2D();


    void DrawBox(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f, float lineWidth = 1.5f);
    void DrawFilledBox(float x, float y, float w, float h, float r, float g, float b, float a);
    void DrawCornerBox(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f, float lineWidth = 1.5f);
    void DrawModernCornerBox(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f, float lineWidth = 2.0f);
    void DrawRoundedBox(float x, float y, float w, float h, float radius, float r, float g, float b, float a = 1.0f, float lineWidth = 1.5f);
    void DrawFilledRoundedBox(float x, float y, float w, float h, float radius, float r, float g, float b, float a = 1.0f);
    void DrawPillBadge(float x, float y, float w, float h, float r, float g, float b, float a, float borderR, float borderG, float borderB, float borderA);
    void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a = 1.0f, float width = 1.5f);
    void DrawGlowLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a = 1.0f, float width = 1.5f);
    void DrawCircle(float cx, float cy, float radius, int segments, float r, float g, float b, float a = 1.0f, float width = 1.5f);
    void DrawFilledCircle(float cx, float cy, float radius, int segments, float r, float g, float b, float a = 1.0f);
    void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float a = 1.0f, bool filled = true);
    void DrawGradientBox(float x, float y, float w, float h, float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2, bool vertical = true);
    void DrawProgressBar(float x, float y, float w, float h, float fraction, float r, float g, float b, float a = 1.0f);
    void DrawOffscreenArrow(float cx, float cy, float angleRad, float radius, float size, float r, float g, float b, float a);
    void DrawRadarSweepLine(float cx, float cy, float radius, float sweepAngleRad, float r, float g, float b, float a);
    void DrawString(float x, float y, float r, float g, float b, const char* text, float scale = 1.0f);
    float GetTextWidth(const char* text, float scale = 1.0f);
}
