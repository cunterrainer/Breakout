#ifndef HARDWARE_RENDERER_H
#define HARDWARE_RENDERER_H

#include "raylib.h"

#include "images.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

void hardware_renderer_init(int width, int height)
{
    InitWindow(width, height, "Breakout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetExitKey(KEY_NULL);

    Image icon = LoadImageFromMemory(".png", sg_Icon_image, ARRAY_SIZE(sg_Icon_image));
    SetWindowIcon(icon);
    UnloadImage(icon);
}

static inline void hardware_renderer_shutdown()
{
    TerminateWindow();
}

static inline void hardware_renderer_begin_drawing()
{
    BeginDrawing();
}

static inline void hardware_renderer_clear_background(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    ClearBackground((Color){ r, g, b, a });
}

static inline void hardware_renderer_end_drawing()
{
    EndDrawing();
}

static inline void hardware_renderer_draw_rectangle_rec(Rectangle rec, Color color)
{
    DrawRectangleRec(rec, color);
}

static inline void hardware_renderer_draw_circle_v(Vector2 center, float radius, Color color)
{
    DrawCircleV(center, radius, color);
}

static inline void hardware_renderer_draw_triangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    DrawTriangle(v1, v2, v3, color);
}

static inline void hardware_renderer_draw_line_v(Vector2 v1, Vector2 v2, Color color)
{
    DrawLineV(v1, v2, color);
}

static inline void hardware_renderer_draw_rectangle_lines_ex(Rectangle rec, float thickness, Color color)
{
    DrawRectangleLinesEx(rec, thickness, color);
}

static inline void hardware_renderer_draw_circle_lines_v(Vector2 center, float radius, Color color)
{
    DrawCircleLinesV(center, radius, color);
}

static inline void hardware_renderer_draw_text(const char* text, int posX, int posY, int fontSize, Color color)
{
    DrawText(text, posX, posY, fontSize, color);
}

static inline void hardware_renderer_draw_rectangle(int x, int y, int width, int height, Color color)
{
    DrawRectangle(x, y, width, height, color);
}

static inline void hardware_renderer_draw_fps(int x, int y)
{
    DrawFPS(x, y);
}

static inline void hardware_renderer_draw_texture_ex(Texture2D texture, Vector2 pos, float rotation, float scale, Color tint)
{
    DrawTextureEx(texture, pos, rotation, scale, tint);
}

#endif // HARDWARE_RENDERER_H
