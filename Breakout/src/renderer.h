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

#endif // RENDERER_H
