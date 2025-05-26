#ifndef SOFTWARE_RENDERER_H
#define SOFTWARE_RENDERER_H

#include "stdlib.h"

#include "raylib.h"

//unsigned char g_SoftwareRendererFramebuffer[1200*750*4];
unsigned char* g_SoftwareRendererFramebuffer = NULL;
Texture2D g_SoftwareRendererTexture;

struct SoftwareRenderer {
    int width;
    int height;
    Font font;
    Image font_image;
    Color* font_pixels;
};


struct SoftwareRenderer software_renderer_init(int width, int height)
{
    g_SoftwareRendererFramebuffer = (unsigned char*)malloc(width * height * 4 * sizeof(unsigned char));

    Image image = {
        .data = g_SoftwareRendererFramebuffer,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    g_SoftwareRendererTexture = LoadTextureFromImage(image);


    struct SoftwareRenderer renderer;
    renderer.width = width;
    renderer.height = height;
    renderer.font = GetFontDefault();
    renderer.font_image = LoadImageFromTexture(renderer.font.texture);
    renderer.font_pixels = LoadImageColors(renderer.font_image);
    return renderer;
}


static inline void software_renderer_shutdown(struct SoftwareRenderer renderer)
{
    if (g_SoftwareRendererFramebuffer != NULL)
        free(g_SoftwareRendererFramebuffer);

    UnloadTexture(g_SoftwareRendererTexture); // Free GPU texture
    UnloadImage(renderer.font_image);         // Free CPU image copy
    UnloadImageColors(renderer.font_pixels);
}


static inline void software_renderer_resize(struct SoftwareRenderer* renderer, int new_width, int new_height)
{
    unsigned char* tmp = (unsigned char*)realloc(g_SoftwareRendererFramebuffer, new_width * new_height * 4 * sizeof(unsigned char));
    if (tmp != NULL)
    {
        renderer->width = new_width;
        renderer->height = new_height;
        g_SoftwareRendererFramebuffer = tmp;

        UnloadTexture(g_SoftwareRendererTexture);
        Image image = {
            .data = g_SoftwareRendererFramebuffer,
            .width = new_width,
            .height = new_height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };
        g_SoftwareRendererTexture = LoadTextureFromImage(image);
    }
}


static inline void software_renderer_begin_drawing()
{
    BeginDrawing();
}


void software_renderer_clear_background(const struct SoftwareRenderer* renderer, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < renderer->width*renderer->height*4; i += 4)
    {
        g_SoftwareRendererFramebuffer[i] = r;
        g_SoftwareRendererFramebuffer[i+1] = g;
        g_SoftwareRendererFramebuffer[i+2] = b;
        g_SoftwareRendererFramebuffer[i+3] = a;
    }
}


static inline void software_renderer_end_drawing()
{
    UpdateTexture(g_SoftwareRendererTexture, g_SoftwareRendererFramebuffer);
    DrawTexture(g_SoftwareRendererTexture, 0, 0, WHITE);
    EndDrawing();
}


static inline int software_renderer_abs(int x)
{
    return x < 0 ? -x : x;
}


static inline void software_renderer_put_pixel(const struct SoftwareRenderer* renderer, int x, int y, Color color)
{
    if (x < 0 || x >= renderer->width || y < 0 || y >= renderer->height) return;

    const int index = (y * renderer->width + x) * 4;
    g_SoftwareRendererFramebuffer[index + 0] = color.r;
    g_SoftwareRendererFramebuffer[index + 1] = color.g;
    g_SoftwareRendererFramebuffer[index + 2] = color.b;
    g_SoftwareRendererFramebuffer[index + 3] = color.a;
}


// Helper: blend a pixel with alpha blending into the framebuffer
void software_renderer_put_pixel_alpha(const struct SoftwareRenderer* renderer, int x, int y, Color color)
{
    if (x < 0 || x >= renderer->width || y < 0 || y >= renderer->height) return;

    const int index = (y * renderer->width + x) * 4;

    const unsigned char dstR = g_SoftwareRendererFramebuffer[index + 0];
    const unsigned char dstG = g_SoftwareRendererFramebuffer[index + 1];
    const unsigned char dstB = g_SoftwareRendererFramebuffer[index + 2];
    const unsigned char dstA = g_SoftwareRendererFramebuffer[index + 3];

    const float srcAlpha = color.a / 255.0f;
    const float invAlpha = 1.0f - srcAlpha;

    const unsigned char outR = (unsigned char)(color.r * srcAlpha + dstR * invAlpha);
    const unsigned char outG = (unsigned char)(color.g * srcAlpha + dstG * invAlpha);
    const unsigned char outB = (unsigned char)(color.b * srcAlpha + dstB * invAlpha);
    const unsigned char outA = (unsigned char)(color.a * srcAlpha + dstA * invAlpha);

    g_SoftwareRendererFramebuffer[index + 0] = outR;
    g_SoftwareRendererFramebuffer[index + 1] = outG;
    g_SoftwareRendererFramebuffer[index + 2] = outB;
    g_SoftwareRendererFramebuffer[index + 3] = outA;
}


void software_renderer_draw_text(const struct SoftwareRenderer* renderer, const char* text, int x, int y, int fontSize, Color color)
{
    int cursorX = x;
    const float scale = (float)fontSize / (float)renderer->font.baseSize;

    for (int i = 0; text[i] != '\0'; ++i)
    {
        const unsigned char c = (unsigned char)text[i];
        int glyphIndex = -1;

        for (int g = 0; g < renderer->font.glyphCount; g++)
        {
            if (renderer->font.glyphs[g].value == c)
            {
                glyphIndex = g;
                break;
            }
        }
        if (glyphIndex == -1) continue;

        const Rectangle glyphRec = renderer->font.recs[glyphIndex];
        const GlyphInfo glyph = renderer->font.glyphs[glyphIndex];

        // Loop over glyph pixels
        for (int py = 0; py < (int)glyphRec.height; py++)
        {
            for (int px = 0; px < (int)glyphRec.width; px++)
            {
                const int gx = (int)(glyphRec.x + px);
                const int gy = (int)(glyphRec.y + py);
                const Color texel = renderer->font_pixels[gy * renderer->font_image.width + gx];

                if (texel.a == 0) continue;

                // Scaled target position
                const int baseX = cursorX + (int)((px + glyph.offsetX) * scale);
                const int baseY = y + (int)((py + glyph.offsetY) * scale);

                // Scale the glyph pixel to a block of scale x scale
                int blockSize = (int)(scale + 0.5f);
                if (blockSize < 1) blockSize = 1;

                for (int dy = 0; dy < blockSize; dy++)
                {
                    for (int dx = 0; dx < blockSize; dx++)
                    {
                        software_renderer_put_pixel(renderer, baseX + dx, baseY + dy, color);
                    }
                }
            }
        }

        // Advance the cursor
        const float advance = (float)(glyph.advanceX ? glyph.advanceX : glyphRec.width + renderer->font.glyphPadding + 1);
        cursorX += (int)(advance * scale + 0.5f);
    }
}


void software_renderer_draw_rectangle_rec(const struct SoftwareRenderer* renderer, Rectangle rec, Color color)
{
    int startX = (int)rec.x;
    int startY = (int)rec.y;
    int endX = startX + (int)rec.width;
    int endY = startY + (int)rec.height;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > renderer->width) endX = renderer->width;
    if (endY > renderer->height) endY = renderer->height;

    for (int y = startY; y < endY; y++)
    {
        for (int x = startX; x < endX; x++)
        {
            int index = (y * renderer->width + x) * 4;
            g_SoftwareRendererFramebuffer[index + 0] = color.r;
            g_SoftwareRendererFramebuffer[index + 1] = color.g;
            g_SoftwareRendererFramebuffer[index + 2] = color.b;
            g_SoftwareRendererFramebuffer[index + 3] = color.a;
        }
    }
}


void software_renderer_draw_fps(const struct SoftwareRenderer* renderer, int x, int y)
{
    Color color = LIME;                         // Good FPS
    const int fps = GetFPS();

    if ((fps < 30) && (fps >= 15)) color = ORANGE;  // Warning FPS
    else if (fps < 15) color = RED;             // Low FPS

    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "%d FPS", fps);

    software_renderer_draw_text(renderer, fpsText, x, y, 20, color);
}


void software_renderer_draw_rectangle(const struct SoftwareRenderer* renderer, int x, int y, int width, int height, Color color)
{
    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            const int px = x + col;
            const int py = y + row;

            // Bounds check
            if (px >= 0 && px < renderer->width && py >= 0 && py < renderer->height)
            {
                const int index = (py * renderer->width + px) * 4; // 4 bytes per pixel (RGBA)

                g_SoftwareRendererFramebuffer[index + 0] = color.r;
                g_SoftwareRendererFramebuffer[index + 1] = color.g;
                g_SoftwareRendererFramebuffer[index + 2] = color.b;
                g_SoftwareRendererFramebuffer[index + 3] = 255; // Full opacity
            }
        }
    }
}


void software_renderer_draw_circle_v(const struct SoftwareRenderer* renderer, Vector2 center, float radius, Color color)
{
    const int cx = (int)center.x;
    const int cy = (int)center.y;
    const int r = (int)radius;
    const int rSquared = r * r;

    // Bounding box of the circle
    int minX = cx - r;
    int maxX = cx + r;
    int minY = cy - r;
    int maxY = cy + r;

    // Clamp to screen bounds
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX > renderer->width) maxX = renderer->width;
    if (maxY > renderer->height) maxY = renderer->height;

    for (int y = minY; y < maxY; y++)
    {
        for (int x = minX; x < maxX; x++)
        {
            const int dx = x - cx;
            const int dy = y - cy;

            if (dx*dx + dy*dy <= rSquared)
            {
                const int index = (y * renderer->width + x) * 4;
                g_SoftwareRendererFramebuffer[index + 0] = color.r;
                g_SoftwareRendererFramebuffer[index + 1] = color.g;
                g_SoftwareRendererFramebuffer[index + 2] = color.b;
                g_SoftwareRendererFramebuffer[index + 3] = color.a;
            }
        }
    }
}


// Helper function to swap two Vector2s
static inline void software_renderer_swap_vec2(Vector2 *a, Vector2 *b)
{
    const Vector2 temp = *a;
    *a = *b;
    *b = temp;
}


void software_renderer_draw_texture_ex(const struct SoftwareRenderer* renderer, Image img, Vector2 pos, float scale, Color tint)
{
    const Color* const pixels = (Color*)img.data;

    const int scaledWidth = (int)(img.width * scale);
    const int scaledHeight = (int)(img.height * scale);

    for (int y = 0; y < scaledHeight; ++y)
    {
        for (int x = 0; x < scaledWidth; ++x)
        {
            // Source image coordinates (nearest neighbor)
            const int srcX = (int)(x / scale);
            const int srcY = (int)(y / scale);

            if (srcX < 0 || srcX >= img.width || srcY < 0 || srcY >= img.height)
                continue;

            const Color texel = pixels[srcY * img.width + srcX];

            if (texel.a < 50) // a little workaround because volume off isn't entirely transparent
                continue; // Skip transparent pixels

            // Apply tint
            const Color out = {
                .r = (unsigned char)(texel.r * tint.r / 255),
                .g = (unsigned char)(texel.g * tint.g / 255),
                .b = (unsigned char)(texel.b * tint.b / 255),
                .a = 255
            };

            const int dstX = (int)pos.x + x;
            const int dstY = (int)pos.y + y;

            // Bounds check
            if (dstX >= 0 && dstX < renderer->width && dstY >= 0 && dstY < renderer->height)
            {
                int index = (dstY * renderer->width + dstX) * 4;
                g_SoftwareRendererFramebuffer[index + 0] = out.r;
                g_SoftwareRendererFramebuffer[index + 1] = out.g;
                g_SoftwareRendererFramebuffer[index + 2] = out.b;
                g_SoftwareRendererFramebuffer[index + 3] = out.a;
            }
        }
    }
}


// Helper function to draw a horizontal line between two x values at a given y
static inline void software_renderer_draw_horizontal_line(const struct SoftwareRenderer* renderer, int y, int x0, int x1, Color color)
{
    if (y < 0 || y >= renderer->height) return;

    if (x0 > x1)
    {
        const int temp = x0;
        x0 = x1;
        x1 = temp;
    }

    if (x0 < 0) x0 = 0;
    if (x1 > renderer->width) x1 = renderer->width;

    for (int x = x0; x < x1; x++)
    {
        software_renderer_put_pixel_alpha(renderer, x, y, color);
    }
}


// Main triangle drawing function
void software_renderer_draw_triangle(const struct SoftwareRenderer* renderer, Vector2 v0, Vector2 v1, Vector2 v2, Color color)
{
    // Sort vertices by y (v0.y <= v1.y <= v2.y)
    if (v0.y > v1.y) software_renderer_swap_vec2(&v0, &v1);
    if (v1.y > v2.y) software_renderer_swap_vec2(&v1, &v2);
    if (v0.y > v1.y) software_renderer_swap_vec2(&v0, &v1);

    // Convert to integers for pixel rasterization

    // Handle flat-bottom triangle
    if ((int)v1.y == (int)v0.y)
    {
        // Sort by x
        if (v0.x > v1.x) software_renderer_swap_vec2(&v0, &v1);

        const float inv_slope_left  = (v2.x - v0.x) / (v2.y - v0.y);
        const float inv_slope_right = (v2.x - v1.x) / (v2.y - v1.y);

        float curx_left = v0.x;
        float curx_right = v1.x;

        for (int y = (int)v0.y; y <= (int)v2.y; y++)
        {
            software_renderer_draw_horizontal_line(renderer, y, (int)curx_left, (int)curx_right, color);
            curx_left += inv_slope_left;
            curx_right += inv_slope_right;
        }
    }
    // Handle flat-top triangle
    else if ((int)v1.y == (int)v2.y)
    {
        // Sort by x
        if (v1.x > v2.x) software_renderer_swap_vec2(&v1, &v2);

        const float inv_slope_left  = (v1.x - v0.x) / (v1.y - v0.y);
        const float inv_slope_right = (v2.x - v0.x) / (v2.y - v0.y);

        float curx_left = v0.x;
        float curx_right = v0.x;

        for (int y = (int)v0.y; y <= (int)v1.y; y++)
        {
            software_renderer_draw_horizontal_line(renderer, y, (int)curx_left, (int)curx_right, color);
            curx_left += inv_slope_left;
            curx_right += inv_slope_right;
        }
    }
    // General case: split the triangle into two flat ones
    else
    {
        // Find the split point
        const float t = (v1.y - v0.y) / (v2.y - v0.y);
        const Vector2 vi = {
            v0.x + t * (v2.x - v0.x),
            v1.y
        };

        software_renderer_draw_triangle(renderer, v0, v1, vi, color);
        software_renderer_draw_triangle(renderer, v1, vi, v2, color);
    }
}


void software_renderer_draw_line_v(const struct SoftwareRenderer* renderer, Vector2 v1, Vector2 v2, Color color)
{
    int x0 = (int)v1.x;
    int y0 = (int)v1.y;
    const int x1 = (int)v2.x;
    const int y1 = (int)v2.y;

    const int dx = software_renderer_abs(x1 - x0);
    const int dy = software_renderer_abs(y1 - y0);

    const int sx = (x0 < x1) ? 1 : -1;
    const int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        software_renderer_put_pixel(renderer, x0, y0, color);  // Use your alpha blending pixel function

        if (x0 == x1 && y0 == y1) break;

        const int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void software_renderer_draw_rectangle_lines_ex(const struct SoftwareRenderer* renderer, Rectangle rec, int thickness, Color color)
{
    const int x = (int)rec.x;
    const int y = (int)rec.y;
    const int w = (int)rec.width;
    const int h = (int)rec.height;

    for (int i = 0; i < thickness; i++)
    {
        // Top line
        software_renderer_draw_line_v(renderer, (Vector2) { x + i, y + i }, (Vector2) { x + w - 1 - i, y + i }, color);
        // Bottom line
        software_renderer_draw_line_v(renderer, (Vector2) { x + i, y + h - 1 - i }, (Vector2) { x + w - 1 - i, y + h - 1 - i }, color);
        // Left line
        software_renderer_draw_line_v(renderer, (Vector2) { x + i, y + i }, (Vector2) { x + i, y + h - 1 - i }, color);
        // Right line
        software_renderer_draw_line_v(renderer, (Vector2) { x + w - 1 - i, y + i }, (Vector2) { x + w - 1 - i, y + h - 1 - i }, color);
    }
}


void software_renderer_draw_circle_lines_v(const struct SoftwareRenderer* renderer, Vector2 center, float radius, Color color)
{
    const int cx = (int)center.x;
    const int cy = (int)center.y;
    const int r = (int)radius;

    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        software_renderer_put_pixel(renderer, cx + x, cy + y, color);
        software_renderer_put_pixel(renderer, cx + y, cy + x, color);
        software_renderer_put_pixel(renderer, cx - y, cy + x, color);
        software_renderer_put_pixel(renderer, cx - x, cy + y, color);
        software_renderer_put_pixel(renderer, cx - x, cy - y, color);
        software_renderer_put_pixel(renderer, cx - y, cy - x, color);
        software_renderer_put_pixel(renderer, cx + y, cy - x, color);
        software_renderer_put_pixel(renderer, cx + x, cy - y, color);

        y += 1;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

#endif // SOFTWARE_RENDERER_H
