#include "raylib.h"

#include "core.h"

int main()
{
    const Color background_color = { .r = 10, .g = 10, .b = 10, .a = 255 };
    InitWindow(850, 600, "Breakout");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        //const float deltaTime = GetFrameTime();
        BeginDrawing();
        ClearBackground(background_color);

        EndDrawing();
    }
    TerminateWindow();
}