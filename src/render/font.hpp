#ifndef RENDER_FONT_HPP
#define RENDER_FONT_HPP

#include <windows.h>
#include <gl/gl.h>

namespace Render {
    void DrawString(float x, float y, float r, float g, float b, const char* text, float scale = 1.0f);
    float GetTextWidth(const char* text, float scale = 1.0f);
}

#endif // RENDER_FONT_HPP
