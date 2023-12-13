#include "raylib.h"

#include "core.h"
#include "winmain.h"

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 750
#define PADDLE_WIDTH  230
#define PADDLE_HEIGHT 30

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

int main()
{
    const Color background_color = { .r = 10, .g = 10, .b = 10, .a = 255 };
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Rectangle paddle = { .x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2.f, .y = WINDOW_HEIGHT - 60, .width = PADDLE_WIDTH, .height = PADDLE_HEIGHT};

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(background_color);
        
        DrawRectangleRec(paddle, RED);

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            paddle.x = MAX(0, paddle.x - 750 * dt);
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            paddle.x = MIN(paddle.x + 750 * dt, WINDOW_WIDTH - paddle.width);

        EndDrawing();
    }
    TerminateWindow();
}