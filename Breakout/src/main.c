#include "raylib.h"

#include "core.h"
#include "winmain.h"

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 750
#define PADDLE_WIDTH  230
#define PADDLE_HEIGHT 30
#define BRICKS_HOR    10 // num of horizontal bricks
#define BRICKS_VER    7  // num of vertical bricks
#define NUM_BRICKS    BRICKS_HOR * BRICKS_VER
#define BRICK_PADDING 5

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)


struct Brick
{
    Rectangle rec;
    Color col;
};


void generate_bricks(struct Brick* bricks)
{
    const float brick_width = WINDOW_WIDTH / (float)BRICKS_HOR - BRICK_PADDING*2;

    size_t color_idx = 0;
    const Color colors[BRICKS_VER] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, SKYBLUE };

    float current_x = BRICK_PADDING;
    float current_y = 50;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        bricks[i].rec.x = current_x;
        bricks[i].rec.y = current_y;
        bricks[i].rec.width = brick_width;
        bricks[i].rec.height = PADDLE_HEIGHT;
        bricks[i].col = colors[color_idx];
        current_x += brick_width + BRICK_PADDING*2;

        if ((i + 1) % BRICKS_HOR == 0)
        {
            current_x = BRICK_PADDING;
            current_y += PADDLE_HEIGHT + 5;
            color_idx++;
        }
    }
}


int main()
{
    const Color background_color = { .r = 10, .g = 10, .b = 10, .a = 255 };
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    struct Brick bricks[NUM_BRICKS] = { 0 };
    generate_bricks(bricks);
    Rectangle paddle = { .x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2.f, .y = WINDOW_HEIGHT - 60, .width = PADDLE_WIDTH, .height = PADDLE_HEIGHT};

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(background_color);
        

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            paddle.x = MAX(0, paddle.x - 750 * dt);
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            paddle.x = MIN(paddle.x + 750 * dt, WINDOW_WIDTH - paddle.width);

        DrawRectangleRec(paddle, RED);

        for (size_t i = 0; i < NUM_BRICKS; ++i)
            DrawRectangleRec(bricks[i].rec, bricks[i].col);

        EndDrawing();
    }
    TerminateWindow();
}