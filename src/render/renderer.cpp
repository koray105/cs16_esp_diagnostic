#include "renderer.hpp"
#include <cstdint>
#include <cstring>
#include <cmath>

typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);

namespace Render {
    static PFNGLUSEPROGRAMPROC g_fnUseProgram = nullptr;

    static const uint8_t s_font8x8[96][8] = {
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
        {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
        {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
        {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // 35 '#'
        {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // 36 '$'
        {0x00,0x66,0x6C,0x18,0x30,0x66,0x46,0x00}, // 37 '%'
        {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // 38 '&'
        {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, // 39 '''
        {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // 40 '('
        {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // 41 ')'
        {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
        {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // 43 '+'
        {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 ','
        {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // 45 '-'
        {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 '.'
        {0x02,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, // 47 '/'
        {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 48 '0'
        {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 49 '1'
        {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00}, // 50 '2'
        {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 51 '3'
        {0x0C,0x1C,0x34,0x64,0x7E,0x04,0x04,0x00}, // 52 '4'
        {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 53 '5'
        {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 54 '6'
        {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, // 55 '7'
        {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 56 '8'
        {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, // 57 '9'
        {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00}, // 58 ':'
        {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00}, // 59 ';'
        {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, // 60 '<'
        {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // 61 '='
        {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00}, // 62 '>'
        {0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00}, // 63 '?'
        {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00}, // 64 '@'
        {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, // 65 'A'
        {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // 66 'B'
        {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // 67 'C'
        {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // 68 'D'
        {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // 69 'E'
        {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // 70 'F'
        {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00}, // 71 'G'
        {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 72 'H'
        {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 73 'I'
        {0x0E,0x06,0x06,0x06,0x06,0x66,0x3C,0x00}, // 74 'J'
        {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // 75 'K'
        {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // 76 'L'
        {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // 77 'M'
        {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // 78 'N'
        {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 79 'O'
        {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // 80 'P'
        {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00}, // 81 'Q'
        {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, // 82 'R'
        {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // 83 'S'
        {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 84 'T'
        {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 85 'U'
        {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // 86 'V'
        {0x63,0x63,0x63,0x6B,0x7F,0x7F,0x36,0x00}, // 87 'W'
        {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // 88 'X'
        {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // 89 'Y'
        {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // 90 'Z'
        {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // 91 '['
        {0x40,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, // 92 '\'
        {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // 93 ']'
        {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // 94 '^'
        {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}, // 95 '_'
        {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
        {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // 97 'a'
        {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // 98 'b'
        {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00}, // 99 'c'
        {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // 100 'd'
        {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // 101 'e'
        {0x1C,0x30,0x78,0x30,0x30,0x30,0x30,0x00}, // 102 'f'
        {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, // 103 'g'
        {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // 104 'h'
        {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // 105 'i'
        {0x0C,0x00,0x0C,0x0C,0x0C,0x0C,0x6C,0x38}, // 106 'j'
        {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // 107 'k'
        {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 108 'l'
        {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, // 109 'm'
        {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // 110 'n'
        {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // 111 'o'
        {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // 112 'p'
        {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x07}, // 113 'q'
        {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // 114 'r'
        {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // 115 's'
        {0x18,0x18,0x7E,0x18,0x18,0x18,0x0C,0x00}, // 116 't'
        {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // 117 'u'
        {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // 118 'v'
        {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, // 119 'w'
        {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // 120 'x'
        {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, // 121 'y'
        {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // 122 'z'
        {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // 123 '{'
        {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 124 '|'
        {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // 125 '}'
        {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}, // 126 '~'
        {0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x00}  // 127 Box
    };

    static float s_cornerTR[7][2];
    static float s_cornerTL[7][2];
    static float s_cornerBL[7][2];
    static float s_cornerBR[7][2];

    static float s_sinCos6[7][2];
    static float s_sinCos8[9][2];
    static float s_sinCos14[15][2];
    static float s_sinCos16[17][2];
    static float s_sinCos28[29][2];
    static float s_sinCos36[37][2];
    static float s_sinCos48[49][2];
    static bool  s_tablesInit = false;

    static void InitTrigTables() {
        if (s_tablesInit) return;
        const int segs = 6;
        const float pi = 3.14159265358979323846f;
        for (int i = 0; i <= segs; i++) {
            float th = (float)i / (float)segs * (pi * 0.5f);
            s_cornerTR[i][0] = cosf(th);
            s_cornerTR[i][1] = sinf(th);

            float thTL = pi * 0.5f + th;
            s_cornerTL[i][0] = cosf(thTL);
            s_cornerTL[i][1] = sinf(thTL);

            float thBL = pi + th;
            s_cornerBL[i][0] = cosf(thBL);
            s_cornerBL[i][1] = sinf(thBL);

            float thBR = 1.5f * pi + th;
            s_cornerBR[i][0] = cosf(thBR);
            s_cornerBR[i][1] = sinf(thBR);
        }

        auto genCircle = [](float table[][2], int count) {
            for (int i = 0; i <= count; i++) {
                float th = (float)i / (float)count * 6.28318530718f;
                table[i][0] = cosf(th);
                table[i][1] = sinf(th);
            }
        };
        genCircle(s_sinCos6, 6);
        genCircle(s_sinCos8, 8);
        genCircle(s_sinCos14, 14);
        genCircle(s_sinCos16, 16);
        genCircle(s_sinCos28, 28);
        genCircle(s_sinCos36, 36);
        genCircle(s_sinCos48, 48);

        s_tablesInit = true;
    }

    void Init() {
        InitTrigTables();
        g_fnUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    }


    bool Begin2D(int& outW, int& outH, HDC hdc) {
        if (!wglGetCurrentContext()) {
            return false;
        }

        GLint vp[4] = {0};
        glGetIntegerv(GL_VIEWPORT, vp);
        outW = vp[2];
        outH = vp[3];
        if (outW <= 0 || outH <= 0) { outW = 800; outH = 600; }

        // Targeted OpenGL state push
        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_POLYGON_BIT);


        if (g_fnUseProgram) {
            g_fnUseProgram(0);
        }

        glViewport(0, 0, outW, outH);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, (GLdouble)outW, (GLdouble)outH, 0.0, -100.0, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_FOG);
        glDepthMask(GL_FALSE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return true;
    }


    void End2D() {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glPopAttrib();
    }

    void DrawBox(float x, float y, float w, float h, float r, float g, float b, float a, float lineWidth) {
        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
        glEnd();
    }

    void DrawCornerBox(float x, float y, float w, float h, float r, float g, float b, float a, float lineWidth) {
        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        float lineW = w * 0.25f;
        float lineH = h * 0.20f;

        glBegin(GL_LINES);
            // Top Left
            glVertex2f(x, y); glVertex2f(x + lineW, y);
            glVertex2f(x, y); glVertex2f(x, y + lineH);
            // Top Right
            glVertex2f(x + w, y); glVertex2f(x + w - lineW, y);
            glVertex2f(x + w, y); glVertex2f(x + w, y + lineH);
            // Bottom Left
            glVertex2f(x, y + h); glVertex2f(x + lineW, y + h);
            glVertex2f(x, y + h); glVertex2f(x, y + h - lineH);
            // Bottom Right
            glVertex2f(x + w, y + h); glVertex2f(x + w - lineW, y + h);
            glVertex2f(x + w, y + h); glVertex2f(x + w, y + h - lineH);
        glEnd();
    }

    void DrawModernCornerBox(float x, float y, float w, float h, float r, float g, float b, float a, float lineWidth) {
        float lineW = w * 0.28f;
        float lineH = h * 0.22f;

        // 1. Black Outer Shadow & Depth Pass
        glColor4f(0.0f, 0.0f, 0.0f, 0.95f);
        glLineWidth(lineWidth + 2.0f);
        glBegin(GL_LINES);
            glVertex2f(x, y); glVertex2f(x + lineW, y);
            glVertex2f(x, y); glVertex2f(x, y + lineH);
            glVertex2f(x + w, y); glVertex2f(x + w - lineW, y);
            glVertex2f(x + w, y); glVertex2f(x + w, y + lineH);
            glVertex2f(x, y + h); glVertex2f(x + lineW, y + h);
            glVertex2f(x, y + h); glVertex2f(x, y + h - lineH);
            glVertex2f(x + w, y + h); glVertex2f(x + w - lineW, y + h);
            glVertex2f(x + w, y + h); glVertex2f(x + w, y + h - lineH);
        glEnd();

        // 2. High-Contrast Colored Foreground Pass
        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        glBegin(GL_LINES);
            glVertex2f(x, y); glVertex2f(x + lineW, y);
            glVertex2f(x, y); glVertex2f(x, y + lineH);
            glVertex2f(x + w, y); glVertex2f(x + w - lineW, y);
            glVertex2f(x + w, y); glVertex2f(x + w, y + lineH);
            glVertex2f(x, y + h); glVertex2f(x + lineW, y + h);
            glVertex2f(x, y + h); glVertex2f(x, y + h - lineH);
            glVertex2f(x + w, y + h); glVertex2f(x + w - lineW, y + h);
            glVertex2f(x + w, y + h); glVertex2f(x + w, y + h - lineH);
        glEnd();
    }

    void DrawFilledRoundedBox(float x, float y, float w, float h, float radius, float r, float g, float b, float a) {
        if (radius < 1.0f) radius = 1.0f;
        if (radius * 2.0f > w) radius = w * 0.5f;
        if (radius * 2.0f > h) radius = h * 0.5f;

        glColor4f(r, g, b, a);
        // Central body
        glBegin(GL_QUADS);
            glVertex2f(x + radius, y);
            glVertex2f(x + w - radius, y);
            glVertex2f(x + w - radius, y + h);
            glVertex2f(x + radius, y + h);

            glVertex2f(x, y + radius);
            glVertex2f(x + radius, y + radius);
            glVertex2f(x + radius, y + h - radius);
            glVertex2f(x, y + h - radius);

            glVertex2f(x + w - radius, y + radius);
            glVertex2f(x + w, y + radius);
            glVertex2f(x + w, y + h - radius);
            glVertex2f(x + w - radius, y + h - radius);
        glEnd();

        // 4 Rounded Corners using precalculated quadrant tables
        const int segs = 6;
        // Top-Right
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + w - radius, y + radius);
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + w - radius + radius * s_cornerTR[i][0], y + radius - radius * s_cornerTR[i][1]);
        }
        glEnd();

        // Top-Left
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + radius, y + radius);
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + radius + radius * s_cornerTL[i][0], y + radius - radius * s_cornerTL[i][1]);
        }
        glEnd();

        // Bottom-Left
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + radius, y + h - radius);
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + radius + radius * s_cornerBL[i][0], y + h - radius - radius * s_cornerBL[i][1]);
        }
        glEnd();

        // Bottom-Right
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + w - radius, y + h - radius);
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + w - radius + radius * s_cornerBR[i][0], y + h - radius - radius * s_cornerBR[i][1]);
        }
        glEnd();
    }

    void DrawRoundedBox(float x, float y, float w, float h, float radius, float r, float g, float b, float a, float lineWidth) {
        if (radius < 1.0f) radius = 1.0f;
        if (radius * 2.0f > w) radius = w * 0.5f;
        if (radius * 2.0f > h) radius = h * 0.5f;

        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        const int segs = 6;

        glBegin(GL_LINE_LOOP);
        // Top-Right
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + w - radius + radius * s_cornerTR[i][0], y + radius - radius * s_cornerTR[i][1]);
        }
        // Top-Left
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + radius + radius * s_cornerTL[i][0], y + radius - radius * s_cornerTL[i][1]);
        }
        // Bottom-Left
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + radius + radius * s_cornerBL[i][0], y + h - radius - radius * s_cornerBL[i][1]);
        }
        // Bottom-Right
        for (int i = 0; i <= segs; i++) {
            glVertex2f(x + w - radius + radius * s_cornerBR[i][0], y + h - radius - radius * s_cornerBR[i][1]);
        }
        glEnd();
    }


    void DrawPillBadge(float x, float y, float w, float h, float r, float g, float b, float a, float borderR, float borderG, float borderB, float borderA) {
        float rad = h * 0.45f;
        // Background Dark Body
        DrawFilledRoundedBox(x, y, w, h, rad, r, g, b, a);
        // Subtle Colored Border
        if (borderA > 0.01f) {
            DrawRoundedBox(x, y, w, h, rad, borderR, borderG, borderB, borderA, 1.2f);
        }
    }

    void DrawOffscreenArrow(float cx, float cy, float angleRad, float radius, float size, float r, float g, float b, float a) {
        float tipX = cx + cosf(angleRad) * radius;
        float tipY = cy + sinf(angleRad) * radius;

        float leftAngle = angleRad + 2.55f;
        float rightAngle = angleRad - 2.55f;

        float leftX = tipX + cosf(leftAngle) * size;
        float leftY = tipY + sinf(leftAngle) * size;
        float rightX = tipX + cosf(rightAngle) * size;
        float rightY = tipY + sinf(rightAngle) * size;

        // Drop shadow pass
        glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
        glBegin(GL_TRIANGLES);
            glVertex2f(tipX + 1.0f, tipY + 1.0f);
            glVertex2f(leftX + 1.0f, leftY + 1.0f);
            glVertex2f(rightX + 1.0f, rightY + 1.0f);
        glEnd();

        // Filled colored arrowhead
        glColor4f(r, g, b, a);
        glBegin(GL_TRIANGLES);
            glVertex2f(tipX, tipY);
            glVertex2f(leftX, leftY);
            glVertex2f(rightX, rightY);
        glEnd();

        // Bright border outline
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        glLineWidth(1.4f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(tipX, tipY);
            glVertex2f(leftX, leftY);
            glVertex2f(rightX, rightY);
        glEnd();
    }

    void DrawRadarSweepLine(float cx, float cy, float radius, float sweepAngleRad, float r, float g, float b, float a) {
        float tipX = cx + cosf(sweepAngleRad) * (radius - 2.0f);
        float tipY = cy + sinf(sweepAngleRad) * (radius - 2.0f);

        // Fading trail sector
        const int tailSteps = 12;
        float tailSpan = 0.55f; // radians
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(r, g, b, a * 0.35f);
        glVertex2f(cx, cy);
        for (int i = 0; i <= tailSteps; i++) {
            float frac = (float)i / (float)tailSteps;
            float curA = sweepAngleRad - frac * tailSpan;
            float alphaStep = a * (1.0f - frac) * 0.35f;
            glColor4f(r, g, b, alphaStep);
            glVertex2f(cx + cosf(curA) * (radius - 2.0f), cy + sinf(curA) * (radius - 2.0f));
        }
        glEnd();

        // Crisp leading sweep line
        glColor4f(r, g, b, a);
        glLineWidth(1.8f);
        glBegin(GL_LINES);
            glVertex2f(cx, cy);
            glVertex2f(tipX, tipY);
        glEnd();
    }

    void DrawGlowLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float width) {
        // Soft outer glow pass
        glColor4f(r, g, b, a * 0.35f);
        glLineWidth(width + 2.5f);
        glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        glEnd();

        // Sharp core line pass
        glColor4f(r, g, b, a);
        glLineWidth(width);
        glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        glEnd();
    }

    void DrawFilledBox(float x, float y, float w, float h, float r, float g, float b, float a) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
        glEnd();
    }

    void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float width) {
        glColor4f(r, g, b, a);
        glLineWidth(width);
        glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        glEnd();
    }

    void DrawCircle(float cx, float cy, float radius, int segments, float r, float g, float b, float a, float width) {
        glColor4f(r, g, b, a);
        glLineWidth(width);

        const float (*table)[2] = nullptr;
        int count = segments;

        if (segments == 48) { table = s_sinCos48; count = 48; }
        else if (segments == 36) { table = s_sinCos36; count = 36; }
        else if (segments == 28) { table = s_sinCos28; count = 28; }
        else if (segments == 16) { table = s_sinCos16; count = 16; }
        else if (segments == 14) { table = s_sinCos14; count = 14; }
        else if (segments == 8)  { table = s_sinCos8;  count = 8;  }
        else if (segments == 6)  { table = s_sinCos6;  count = 6;  }

        glBegin(GL_LINE_LOOP);
        if (table) {
            for (int i = 0; i < count; i++) {
                glVertex2f(cx + radius * table[i][0], cy + radius * table[i][1]);
            }
        } else {
            if (segments < 8) segments = 16;
            for (int i = 0; i < segments; i++) {
                float theta = 6.28318530718f * (float)i / (float)segments;
                glVertex2f(cx + radius * cosf(theta), cy + radius * sinf(theta));
            }
        }
        glEnd();
    }

    void DrawFilledCircle(float cx, float cy, float radius, int segments, float r, float g, float b, float a) {
        glColor4f(r, g, b, a);

        const float (*table)[2] = nullptr;
        int count = segments;

        if (segments == 48) { table = s_sinCos48; count = 48; }
        else if (segments == 36) { table = s_sinCos36; count = 36; }
        else if (segments == 28) { table = s_sinCos28; count = 28; }
        else if (segments == 16) { table = s_sinCos16; count = 16; }
        else if (segments == 14) { table = s_sinCos14; count = 14; }
        else if (segments == 8)  { table = s_sinCos8;  count = 8;  }
        else if (segments == 6)  { table = s_sinCos6;  count = 6;  }

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        if (table) {
            for (int i = 0; i <= count; i++) {
                glVertex2f(cx + radius * table[i][0], cy + radius * table[i][1]);
            }
        } else {
            if (segments < 8) segments = 16;
            for (int i = 0; i <= segments; i++) {
                float theta = 6.28318530718f * (float)i / (float)segments;
                glVertex2f(cx + radius * cosf(theta), cy + radius * sinf(theta));
            }
        }
        glEnd();
    }


    void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float a, bool filled) {
        glColor4f(r, g, b, a);
        if (filled) {
            glBegin(GL_TRIANGLES);
                glVertex2f(x1, y1);
                glVertex2f(x2, y2);
                glVertex2f(x3, y3);
            glEnd();
        } else {
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(x1, y1);
                glVertex2f(x2, y2);
                glVertex2f(x3, y3);
            glEnd();
        }
    }

    void DrawGradientBox(float x, float y, float w, float h, float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2, bool vertical) {
        glBegin(GL_QUADS);
        if (vertical) {
            glColor4f(r1, g1, b1, a1);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glColor4f(r2, g2, b2, a2);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
        } else {
            glColor4f(r1, g1, b1, a1);
            glVertex2f(x, y);
            glColor4f(r2, g2, b2, a2);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glColor4f(r1, g1, b1, a1);
            glVertex2f(x, y + h);
        }
        glEnd();
    }

    void DrawProgressBar(float x, float y, float w, float h, float fraction, float r, float g, float b, float a) {
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;

        // Background track
        DrawFilledBox(x, y, w, h, 0.08f, 0.09f, 0.12f, 0.85f);
        DrawBox(x, y, w, h, 0.2f, 0.22f, 0.28f, 0.9f, 1.0f);

        // Filled bar
        if (fraction > 0.005f) {
            float fillW = (w - 2.0f) * fraction;
            DrawFilledBox(x + 1.0f, y + 1.0f, fillW, h - 2.0f, r, g, b, a);
        }
    }

    static inline void EmitGlyphQuads(float startX, float startY, const char* text, size_t len, float scale) {
        float curX = startX;
        float glyphW = 8.0f * scale;
        for (size_t i = 0; i < len; i++) {
            char c = text[i];
            if (c < 32 || c > 127) c = '?';
            int idx = (int)c - 32;
            const uint8_t* glyph = s_font8x8[idx];

            for (int row = 0; row < 8; row++) {
                uint8_t rowBits = glyph[row];
                if (!rowBits) continue;
                for (int col = 0; col < 8; col++) {
                    if (rowBits & (1 << (7 - col))) {
                        float px = curX + (float)col * scale;
                        float py = startY + (float)row * scale;
                        glVertex2f(px, py);
                        glVertex2f(px + scale, py);
                        glVertex2f(px + scale, py + scale);
                        glVertex2f(px, py + scale);
                    }
                }
            }
            curX += glyphW;
        }
    }

    void DrawString(float x, float y, float r, float g, float b, const char* text, float scale) {
        if (!text || text[0] == 0) return;
        size_t len = strlen(text);
        if (scale < 0.85f) scale = 0.90f;

        // 1. High-Contrast 4-Way Solid Black Outline (N, S, E, W + Diagonal for 100% readability)
        glColor4f(0.0f, 0.0f, 0.0f, 0.98f);
        glBegin(GL_QUADS);
        EmitGlyphQuads(x - 1.0f, y,        text, len, scale);
        EmitGlyphQuads(x + 1.0f, y,        text, len, scale);
        EmitGlyphQuads(x,        y - 1.0f, text, len, scale);
        EmitGlyphQuads(x,        y + 1.0f, text, len, scale);
        EmitGlyphQuads(x + 1.0f, y + 1.0f, text, len, scale);
        glEnd();

        // 2. Bright Foreground Color Pass
        glColor4f(r, g, b, 1.0f);
        glBegin(GL_QUADS);
        EmitGlyphQuads(x, y, text, len, scale);
        glEnd();
    }

    float GetTextWidth(const char* text, float scale) {
        if (!text) return 0.0f;
        if (scale < 0.85f) scale = 0.90f;
        return (float)strlen(text) * 8.0f * scale;
    }
}
