#ifndef SOFTWARE_RENDERER_H
#define SOFTWARE_RENDERER_H

#include "string.h"

#include "raylib.h"

static int g_Width = 1200;
static int g_Height = 750;
static unsigned char g_SoftwareRendererFramebuffer[1200*750*4];
static Texture2D g_SoftwareRendererTexture;
static Font g_Font;
static Image g_FontImage;
static Color* g_FontPixels;

void software_renderer_init() // TODO: CLEANUP
{
    Image image = {
        .data = g_SoftwareRendererFramebuffer,
        .width = 1200,
        .height = 750,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    g_SoftwareRendererTexture = LoadTextureFromImage(image);
    g_Font = GetFontDefault();
    g_FontImage = LoadImageFromTexture(g_Font.texture);
    g_FontPixels = LoadImageColors(g_FontImage);
}


void software_renderer_begin_drawing()
{
    BeginDrawing();
}


void software_renderer_clear_background(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int i = 0; i < g_Width*g_Height*4; i += 4)
    {
        g_SoftwareRendererFramebuffer[i] = r;
        g_SoftwareRendererFramebuffer[i+1] = g;
        g_SoftwareRendererFramebuffer[i+2] = b;
        g_SoftwareRendererFramebuffer[i+3] = a;
    }
}


void software_renderer_end_drawing()
{
    UpdateTexture(g_SoftwareRendererTexture, g_SoftwareRendererFramebuffer);
    DrawTexture(g_SoftwareRendererTexture, 0, 0, WHITE);
    EndDrawing();
}


inline int software_renderer_abs(int x)
{
    return x < 0 ? -x : x;
}


inline void software_renderer_put_pixel(int x, int y, Color color)
{
    if (x < 0 || x >= g_Width || y < 0 || y >= g_Height) return;

    int index = (y * g_Width + x) * 4;
    g_SoftwareRendererFramebuffer[index + 0] = color.r;
    g_SoftwareRendererFramebuffer[index + 1] = color.g;
    g_SoftwareRendererFramebuffer[index + 2] = color.b;
    g_SoftwareRendererFramebuffer[index + 3] = color.a;
}


void software_renderer_draw_text(const char* text, int x, int y, int fontSize, Color color)
{
    int scale = fontSize / g_Font.baseSize;
    int cursorX = x;
    for (int i = 0; text[i] != '\0'; ++i) {
        char c = text[i];
        int codepoint = (unsigned char)c;

        // Find glyph index
        int glyphIndex = -1;
        for (int g = 0; g < g_Font.glyphCount; g++) {
            if (g_Font.glyphs[g].value == codepoint) {
                glyphIndex = g;
                break;
            }
        }

        if (glyphIndex == -1) continue; // Character not found

        Rectangle glyphRec = g_Font.recs[glyphIndex];
        GlyphInfo glyph = g_Font.glyphs[glyphIndex];

        // Draw glyph bitmap from font image
        for (int py = 0; py < (int)glyphRec.height; py++) {
            for (int px = 0; px < (int)glyphRec.width; px++) {
                int gx = (int)(glyphRec.x + px);
                int gy = (int)(glyphRec.y + py);

                Color texel = g_FontPixels[gy * g_FontImage.width + gx];

                if (texel.a > 0) {
                    // Scale output
                    for (int sy = 0; sy < scale; sy++) {
                        for (int sx = 0; sx < scale; sx++) {
                            int dstX = cursorX + (px + glyph.offsetX) * scale + sx;
                            int dstY = y + (py + glyph.offsetY) * scale + sy;
                            software_renderer_put_pixel(dstX, dstY, color);
                        }
                    }
                }
            }
        }

        int advance = glyph.advanceX;
        if (advance == 0)
        {
            advance = (int)g_Font.recs[glyphIndex].width + g_Font.glyphPadding;
            advance += 1; // <- add a bit of extra spacing manually
        }
        cursorX += advance * scale;
    }
}


void software_renderer_draw_rectangle_rec(Rectangle rec, Color color)
{
    int startX = (int)rec.x;
    int startY = (int)rec.y;
    int endX = startX + (int)rec.width;
    int endY = startY + (int)rec.height;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > g_Width) endX = g_Width;
    if (endY > g_Height) endY = g_Height;

    for (int y = startY; y < endY; y++)
    {
        for (int x = startX; x < endX; x++)
        {
            int index = (y * g_Width + x) * 4;
            g_SoftwareRendererFramebuffer[index + 0] = color.r;
            g_SoftwareRendererFramebuffer[index + 1] = color.g;
            g_SoftwareRendererFramebuffer[index + 2] = color.b;
            g_SoftwareRendererFramebuffer[index + 3] = color.a;
        }
    }
}


void software_renderer_draw_fps(int x, int y)
{
    int fps = GetFPS();
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);

    software_renderer_draw_text(fpsText, x, y, 20, (Color){ 30, 149, 81, 255 });
}


void software_renderer_draw_rectangle(int x, int y, int width, int height, Color color)
{
    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            int px = x + col;
            int py = y + row;

            // Bounds check
            if (px >= 0 && px < g_Width && py >= 0 && py < g_Height)
            {
                int index = (py * g_Width + px) * 4; // 4 bytes per pixel (RGBA)

                g_SoftwareRendererFramebuffer[index + 0] = color.r;
                g_SoftwareRendererFramebuffer[index + 1] = color.g;
                g_SoftwareRendererFramebuffer[index + 2] = color.b;
                g_SoftwareRendererFramebuffer[index + 3] = 255; // Full opacity
            }
        }
    }
}


void software_renderer_draw_circle_v(Vector2 center, float radius, Color color)
{
    int cx = (int)center.x;
    int cy = (int)center.y;
    int r = (int)radius;
    int rSquared = r * r;

    // Bounding box of the circle
    int minX = cx - r;
    int maxX = cx + r;
    int minY = cy - r;
    int maxY = cy + r;

    // Clamp to screen bounds
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX > g_Width) maxX = g_Width;
    if (maxY > g_Height) maxY = g_Height;

    for (int y = minY; y < maxY; y++)
    {
        for (int x = minX; x < maxX; x++)
        {
            int dx = x - cx;
            int dy = y - cy;

            if (dx*dx + dy*dy <= rSquared)
            {
                int index = (y * g_Width + x) * 4;
                g_SoftwareRendererFramebuffer[index + 0] = color.r;
                g_SoftwareRendererFramebuffer[index + 1] = color.g;
                g_SoftwareRendererFramebuffer[index + 2] = color.b;
                g_SoftwareRendererFramebuffer[index + 3] = color.a;
            }
        }
    }
}


// Helper function to swap two Vector2s
static void swap_vec2(Vector2 *a, Vector2 *b)
{
    Vector2 temp = *a;
    *a = *b;
    *b = temp;
}


// Helper: blend a pixel with alpha blending into the framebuffer
void software_renderer_put_pixel_alpha(int x, int y, Color color)
{
    if (x < 0 || x >= g_Width || y < 0 || y >= g_Height) return;

    int index = (y * g_Width + x) * 4;

    unsigned char dstR = g_SoftwareRendererFramebuffer[index + 0];
    unsigned char dstG = g_SoftwareRendererFramebuffer[index + 1];
    unsigned char dstB = g_SoftwareRendererFramebuffer[index + 2];
    unsigned char dstA = g_SoftwareRendererFramebuffer[index + 3];

    float srcAlpha = color.a / 255.0f;
    float invAlpha = 1.0f - srcAlpha;

    unsigned char outR = (unsigned char)(color.r * srcAlpha + dstR * invAlpha);
    unsigned char outG = (unsigned char)(color.g * srcAlpha + dstG * invAlpha);
    unsigned char outB = (unsigned char)(color.b * srcAlpha + dstB * invAlpha);
    unsigned char outA = (unsigned char)(color.a * srcAlpha + dstA * invAlpha);

    g_SoftwareRendererFramebuffer[index + 0] = outR;
    g_SoftwareRendererFramebuffer[index + 1] = outG;
    g_SoftwareRendererFramebuffer[index + 2] = outB;
    g_SoftwareRendererFramebuffer[index + 3] = outA;
}

// Helper function to draw a horizontal line between two x values at a given y
static void draw_horizontal_line(int y, int x0, int x1, Color color)
{
    if (y < 0 || y >= g_Height) return;

    if (x0 > x1) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
    }

    if (x0 < 0) x0 = 0;
    if (x1 > g_Width) x1 = g_Width;

    for (int x = x0; x < x1; x++)
    {
        software_renderer_put_pixel_alpha(x, y, color);
    }
}


// Main triangle drawing function
void software_renderer_draw_triangle(Vector2 v0, Vector2 v1, Vector2 v2, Color color)
{
    // Sort vertices by y (v0.y <= v1.y <= v2.y)
    if (v0.y > v1.y) swap_vec2(&v0, &v1);
    if (v1.y > v2.y) swap_vec2(&v1, &v2);
    if (v0.y > v1.y) swap_vec2(&v0, &v1);

    // Convert to integers for pixel rasterization

    // Handle flat-bottom triangle
    if ((int)v1.y == (int)v0.y)
    {
        // Sort by x
        if (v0.x > v1.x) swap_vec2(&v0, &v1);

        float inv_slope_left  = (v2.x - v0.x) / (v2.y - v0.y);
        float inv_slope_right = (v2.x - v1.x) / (v2.y - v1.y);

        float curx_left = v0.x;
        float curx_right = v1.x;

        for (int y = (int)v0.y; y <= (int)v2.y; y++) {
            draw_horizontal_line(y, (int)curx_left, (int)curx_right, color);
            curx_left += inv_slope_left;
            curx_right += inv_slope_right;
        }
    }
    // Handle flat-top triangle
    else if ((int)v1.y == (int)v2.y)
    {
        // Sort by x
        if (v1.x > v2.x) swap_vec2(&v1, &v2);

        float inv_slope_left  = (v1.x - v0.x) / (v1.y - v0.y);
        float inv_slope_right = (v2.x - v0.x) / (v2.y - v0.y);

        float curx_left = v0.x;
        float curx_right = v0.x;

        for (int y = (int)v0.y; y <= (int)v1.y; y++) {
            draw_horizontal_line(y, (int)curx_left, (int)curx_right, color);
            curx_left += inv_slope_left;
            curx_right += inv_slope_right;
        }
    }
    // General case: split the triangle into two flat ones
    else
    {
        // Find the split point
        float t = (v1.y - v0.y) / (v2.y - v0.y);
        Vector2 vi = {
            v0.x + t * (v2.x - v0.x),
            v1.y
        };

        software_renderer_draw_triangle(v0, v1, vi, color);
        software_renderer_draw_triangle(v1, vi, v2, color);
    }
}


void software_renderer_draw_line_v(Vector2 v1, Vector2 v2, Color color)
{
    int x0 = (int)v1.x;
    int y0 = (int)v1.y;
    int x1 = (int)v2.x;
    int y1 = (int)v2.y;

    int dx = software_renderer_abs(x1 - x0);
    int dy = software_renderer_abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        software_renderer_put_pixel(x0, y0, color);  // Use your alpha blending pixel function

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void software_renderer_draw_rectangle_lines_ex(Rectangle rec, int thickness, Color color)
{
    int x = (int)rec.x;
    int y = (int)rec.y;
    int w = (int)rec.width;
    int h = (int)rec.height;

    for (int i = 0; i < thickness; i++)
    {
        // Top line
        software_renderer_draw_line_v((Vector2) { x + i, y + i }, (Vector2) { x + w - 1 - i, y + i }, color);
        // Bottom line
        software_renderer_draw_line_v((Vector2) { x + i, y + h - 1 - i }, (Vector2) { x + w - 1 - i, y + h - 1 - i }, color);
        // Left line
        software_renderer_draw_line_v((Vector2) { x + i, y + i }, (Vector2) { x + i, y + h - 1 - i }, color);
        // Right line
        software_renderer_draw_line_v((Vector2) { x + w - 1 - i, y + i }, (Vector2) { x + w - 1 - i, y + h - 1 - i }, color);
    }
}


void software_renderer_draw_circle_lines_v(Vector2 center, float radius, Color color)
{
    int cx = (int)center.x;
    int cy = (int)center.y;
    int r = (int)radius;

    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        software_renderer_put_pixel(cx + x, cy + y, color);
        software_renderer_put_pixel(cx + y, cy + x, color);
        software_renderer_put_pixel(cx - y, cy + x, color);
        software_renderer_put_pixel(cx - x, cy + y, color);
        software_renderer_put_pixel(cx - x, cy - y, color);
        software_renderer_put_pixel(cx - y, cy - x, color);
        software_renderer_put_pixel(cx + y, cy - x, color);
        software_renderer_put_pixel(cx + x, cy - y, color);

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
