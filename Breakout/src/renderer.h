#ifndef RENDERER_H
#define RENDERER_H

#include "hardware_renderer.h"
#include "software_renderer.h"

enum RenderMode
{
    Software,
    Hardware
};

struct Renderer {
    enum RenderMode mode;
    struct SoftwareRenderer software_renderer;
    // struct HardwareRenderer hardware_renderer; // Not needed just yet, but might be in the future
};

static inline struct Renderer renderer_init(int width, int height)
{
    struct Renderer renderer;
    hardware_renderer_init(width, height);
    renderer.mode = Hardware;
    renderer.software_renderer = software_renderer_init(width, height);
    return renderer;
}

static inline void renderer_shutdown(struct Renderer renderer)
{
    software_renderer_shutdown(renderer.software_renderer);
    hardware_renderer_shutdown();
}

static inline void renderer_swap_render_mode(struct Renderer* renderer)
{
    if (renderer->mode == Hardware)
        renderer->mode = Software;
    else
        renderer->mode = Hardware;
}

static inline void renderer_begin_drawing(const struct Renderer* renderer)
{
    if (renderer->mode == Hardware)
        hardware_renderer_begin_drawing();
    else
        software_renderer_begin_drawing();
}

static inline void renderer_clear_background(const struct Renderer* renderer, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (renderer->mode == Hardware)
        hardware_renderer_clear_background(r, g, b, a);
    else
        software_renderer_clear_background(&renderer->software_renderer, r, g, b, a);
}

static inline void renderer_end_drawing(const struct Renderer* renderer)
{
    if (renderer->mode == Hardware)
        hardware_renderer_end_drawing();
    else
        software_renderer_end_drawing();
}

static inline void renderer_draw_rectangle_rec(const struct Renderer* renderer, Rectangle rec, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_rectangle_rec(rec, color);
    else
        software_renderer_draw_rectangle_rec(&renderer->software_renderer, rec, color);
}

static inline void renderer_draw_circle_v(const struct Renderer* renderer, Vector2 center, float radius, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_circle_v(center, radius, color);
    else
        software_renderer_draw_circle_v(&renderer->software_renderer, center, radius, color);
}

static inline void renderer_draw_triangle(const struct Renderer* renderer, Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_triangle(v1, v2, v3, color);
    else
        software_renderer_draw_triangle(&renderer->software_renderer, v1, v2, v3, color);
}

static inline void renderer_draw_line_v(const struct Renderer* renderer, Vector2 v1, Vector2 v2, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_line_v(v1, v2, color);
    else
        software_renderer_draw_line_v(&renderer->software_renderer, v1, v2, color);
}

static inline void renderer_draw_rectangle_lines_ex(const struct Renderer* renderer, Rectangle rec, float thickness, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_rectangle_lines_ex(rec, thickness, color);
    else
        software_renderer_draw_rectangle_lines_ex(&renderer->software_renderer, rec, (int)thickness, color);
}

static inline void renderer_draw_circle_lines_v(const struct Renderer* renderer, Vector2 center, float radius, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_circle_lines_v(center, radius, color);
    else
        software_renderer_draw_circle_lines_v(&renderer->software_renderer, center, radius, color);
}

static inline void renderer_draw_text(const struct Renderer* renderer, const char* text, int posX, int posY, int fontSize, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_text(text, posX, posY, fontSize, color);
    else
        software_renderer_draw_text(&renderer->software_renderer, text, posX, posY, fontSize, color);
}

static inline void renderer_draw_rectangle(const struct Renderer* renderer, int x, int y, int width, int height, Color color)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_rectangle(x, y, width, height, color);
    else
        software_renderer_draw_rectangle(&renderer->software_renderer, x, y, width, height, color);
}

void renderer_draw_fps(const struct Renderer* renderer, int x, int y)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_fps(x, y);
    else
        software_renderer_draw_fps(&renderer->software_renderer, x, y);
}

static inline void renderer_draw_texture_ex(const struct Renderer* renderer, Texture2D texture, Image img, Vector2 pos, float rotation, float scale, Color tint)
{
    if (renderer->mode == Hardware)
        hardware_renderer_draw_texture_ex(texture, pos, rotation, scale, tint);
    else
        software_renderer_draw_texture_ex(&renderer->software_renderer, img, pos, scale, tint); // rotation not supported, doesn't matter for us rotation is always 0
}
#endif // RENDERER_H
