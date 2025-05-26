#ifndef HARDWARE_RENDERER_H
#define HARDWARE_RENDERER_H

#include "raylib.h"

void hardware_renderer_begin_drawing()
{
    BeginDrawing();
}

void hardware_renderer_clear_background(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    ClearBackground((Color){ r, g, b, a });
}

void hardware_renderer_end_drawing()
{
    EndDrawing();
}

void hardware_renderer_draw_rectangle_rec(Rectangle rec, Color color)
{
    DrawRectangleRec(rec, color);
}

void hardware_renderer_draw_circle_v(Vector2 center, float radius, Color color)
{
    DrawCircleV(center, radius, color);
}

void hardware_renderer_draw_triangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    DrawTriangle(v1, v2, v3, color);
}

#endif // HARDWARE_RENDERER_H
