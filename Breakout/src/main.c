#include <stdio.h>

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


struct Ball
{
    Vector2 center;
    float radius;
    Vector2 direction;
};


void generate_bricks(struct Brick* bricks)
{
    const float brick_width = WINDOW_WIDTH / (float)BRICKS_HOR - BRICK_PADDING*2;

    size_t color_idx = 0;
    const Color colors[BRICKS_VER] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, SKYBLUE };

    float current_x = BRICK_PADDING;
    float current_y = 60;
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


void draw_entities(Rectangle paddle, struct Ball ball, const struct Brick* bricks, size_t score)
{
    DrawRectangleRec(paddle, RED);
    DrawCircle((int)ball.center.x, (int)ball.center.y, ball.radius, LIGHTGRAY);

    for (size_t i = 0; i < NUM_BRICKS; ++i) {
        DrawRectangleRec(bricks[i].rec, bricks[i].col);
    }


    const int font_size = 45;
    char str[5] = { 0 }; // max score 9999
    snprintf(str, sizeof(str), "%zu", score);

    const int text_length = MeasureText(str, font_size);
    const int x_pos = (WINDOW_WIDTH - text_length) / 2;
    DrawText(str, x_pos, 10, font_size, GRAY);
}


size_t ball_bricks_collision(const struct Ball ball, struct Brick* bricks)
{
    size_t collisions = 0;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        if (bricks[i].rec.x > 0 && CheckCollisionCircleRec(ball.center, ball.radius, bricks[i].rec))
        {
            bricks[i].rec.x = -20;
            bricks[i].rec.y = -20;
            bricks[i].rec.width = 0;
            bricks[i].rec.height = 0;
            ++collisions;
        }
    }
    return collisions;
}


Vector2 ball_calculate_reflected_direction(Vector2 normal, Vector2 current_direction)
{
    const float dot_product = current_direction.x * normal.x + current_direction.y * normal.y;

    Vector2 reflected_direction;
    reflected_direction.x = current_direction.x - 2.0f * dot_product * normal.x;
    reflected_direction.y = current_direction.y - 2.0f * dot_product * normal.y;
    return reflected_direction;
}


void ball_move(struct Ball* ball, float dt)
{
    const float speed = 100.f;
    ball->center.x += ball->direction.x * dt * speed;
    ball->center.y += ball->direction.y * dt * speed;

    const Vector2 normal_hor = { .x = 1, .y = 0 };
    const Vector2 normal_ver = { .x = 0, .y = 1 };

    if (ball->center.x >= WINDOW_WIDTH && ball->direction.x > 0 || ball->center.x <= 0 && ball->direction.x < 0)
    {
        ball->direction = ball_calculate_reflected_direction(normal_hor, ball->direction);
    }
    else if (ball->center.y >= WINDOW_HEIGHT && ball->direction.y > 0 || ball->center.y <= 0 && ball->direction.y < 0)
    {
        ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
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
    struct Ball ball = { .center = { paddle.x + PADDLE_WIDTH / 2.f, paddle.y - 15 }, .radius = 15.f, .direction = { 1, -1 } };

    size_t score = 0;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(background_color);

        ball_move(&ball, dt);
        score += ball_bricks_collision(ball, bricks);

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            paddle.x = MAX(0, paddle.x - 750 * dt);
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            paddle.x = MIN(paddle.x + 750 * dt, WINDOW_WIDTH - paddle.width);

        draw_entities(paddle, ball, bricks, score);
        EndDrawing();
    }
    TerminateWindow();
}