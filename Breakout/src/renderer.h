#ifndef RENDERER_H
#define RENDERER_H

#include "hardware_renderer.h"
#include "software_renderer.h"

enum RenderMode
{
    Software,
    Hardware
};

void renderer_init()
{
    // hardware renderer doesn't need additional setup
    software_renderer_init();
}

void renderer_begin_drawing(enum RenderMode mode)
{
    if (mode == Hardware)
        hardware_renderer_begin_drawing();
    else
        software_renderer_begin_drawing();
}

void renderer_clear_background(enum RenderMode mode, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (mode == Hardware)
        hardware_renderer_clear_background(r, g, b, a);
    else
        software_renderer_clear_background(r, g, b, a);
}

void renderer_end_drawing(enum RenderMode mode)
{
    if (mode == Hardware)
        hardware_renderer_end_drawing();
    else
        software_renderer_end_drawing();
}

void renderer_draw_rectangle_rec(enum RenderMode mode, Rectangle rec, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_rectangle_rec(rec, color);
    else
        software_renderer_draw_rectangle_rec(rec, color);
}

void renderer_draw_circle_v(enum RenderMode mode, Vector2 center, float radius, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_circle_v(center, radius, color);
    else
        software_renderer_draw_circle_v(center, radius, color);
}

void renderer_draw_triangle(enum RenderMode mode, Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_triangle(v1, v2, v3, color);
    else
        software_renderer_draw_triangle(v1, v2, v3, color);
}

void renderer_draw_line_v(enum RenderMode mode, Vector2 v1, Vector2 v2, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_line_v(v1, v2, color);
    else
        software_renderer_draw_line_v(v1, v2, color);
}

void renderer_draw_rectangle_lines_ex(enum RenderMode mode, Rectangle rec, float thickness, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_rectangle_lines_ex(rec, thickness, color);
    else
        software_renderer_draw_rectangle_lines_ex(rec, (int)thickness, color);
}

void renderer_draw_circle_lines_v(enum RenderMode mode, Vector2 center, float radius, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_circle_lines_v(center, radius, color);
    else
        software_renderer_draw_circle_lines_v(center, radius, color);
}

void renderer_draw_text(enum RenderMode mode, const char* text, int posX, int posY, int fontSize, Color color)
{
    if (mode == Hardware)
        hardware_renderer_draw_text(text, posX, posY, fontSize, color);
    else
        software_renderer_draw_text(text, posX, posY, fontSize, color);
}

#endif // RENDERER_H
