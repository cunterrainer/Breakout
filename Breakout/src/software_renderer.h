#ifndef SOFTWARE_RENDERER_H
#define SOFTWARE_RENDERER_H

#include "raylib.h"

static int g_Width = 1200;
static int g_Height = 750;
static unsigned char g_SoftwareRendererFramebuffer[1200*750*4];
static Texture2D g_SoftwareRendererTexture;

void software_renderer_init()
{
    Image image = {
        .data = g_SoftwareRendererFramebuffer,
        .width = 1200,
        .height = 750,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    g_SoftwareRendererTexture = LoadTextureFromImage(image);
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
void PutPixelAlpha(int x, int y, Color color)
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
        PutPixelAlpha(x, y, color);
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

#endif // SOFTWARE_RENDERER_H
