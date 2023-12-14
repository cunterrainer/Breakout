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


struct GameObjects
{
    struct Brick bricks[NUM_BRICKS];
    Rectangle paddle;
    struct Ball ball;
    size_t score;
};


enum State
{
    Menu,
    Game,
    Break,
    Failed,
    Success
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
    BeginDrawing();
    ClearBackground((Color){ 10, 10, 10, 255 });

    DrawRectangleRec(paddle, RED);
    DrawCircleV(ball.center, ball.radius, LIGHTGRAY);

    for (size_t i = 0; i < NUM_BRICKS; ++i) {
        DrawRectangleRec(bricks[i].rec, bricks[i].col);
    }


    const int font_size = 45;
    char str[5] = { 0 }; // max score 9999
    snprintf(str, sizeof(str), "%zu", score);

    const int text_length = MeasureText(str, font_size);
    const int x_pos = (WINDOW_WIDTH - text_length) / 2;
    DrawText(str, x_pos, 10, font_size, GRAY);
    EndDrawing();
}


Vector2 ball_calculate_reflected_direction(Vector2 normal, Vector2 current_direction)
{
    const float dot_product = current_direction.x * normal.x + current_direction.y * normal.y;

    Vector2 reflected_direction;
    reflected_direction.x = current_direction.x - 2.0f * dot_product * normal.x;
    reflected_direction.y = current_direction.y - 2.0f * dot_product * normal.y;
    return reflected_direction;
}


size_t ball_bricks_collision(struct Ball* ball, struct Brick* bricks)
{
    size_t collisions = 0;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        if (bricks[i].rec.x > 0 && CheckCollisionCircleRec(ball->center, ball->radius, bricks[i].rec))
        {
            bricks[i].rec.x = -20;
            bricks[i].rec.y = -20;
            bricks[i].rec.width = 0;
            bricks[i].rec.height = 0;
            ++collisions;

            ball->direction = ball_calculate_reflected_direction((Vector2) { 0, 1 }, ball->direction);
            break;
        }
    }
    return collisions;
}


void ball_move(struct Ball* ball, Rectangle paddle, float dt)
{
    const float speed = 500.f;
    ball->center.x += ball->direction.x * dt * speed;
    ball->center.y += ball->direction.y * dt * speed;

    const Vector2 normal_hor = { .x = 1, .y = 0 };
    const Vector2 normal_ver = { .x = 0, .y = 1 };

    if (ball->center.x + ball->radius >= WINDOW_WIDTH && ball->direction.x > 0 || ball->center.x - ball->radius <= 0 && ball->direction.x < 0)
    {
        ball->direction = ball_calculate_reflected_direction(normal_hor, ball->direction);
    }
    else if (ball->center.y + ball->radius >= WINDOW_HEIGHT && ball->direction.y > 0 || ball->center.y - ball->radius <= 0 && ball->direction.y < 0)
    {
        ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
    }
    else if (CheckCollisionCircleRec(ball->center, ball->radius, paddle) && ball->direction.y > 0)
    {
        // if you hit the ball in the first 35 % the ball goes back the direction reverses
        // if going from left to right                                                    if going from right to left
        if ((ball->direction.x > 0 && ball->center.x < paddle.x + paddle.width * 0.35) || (ball->direction.x < 0 && ball->center.x > paddle.x + paddle.width * 0.65))
        {
            ball->direction.x = -ball->direction.x;
            ball->direction.y = -ball->direction.y;
        }
        else
        {
            ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
        }
    }
}


struct GameObjects game_objects_init()
{
    struct GameObjects objects;
    objects.score = 0;
    objects.paddle = (Rectangle){ (WINDOW_WIDTH - PADDLE_WIDTH) / 2.f, WINDOW_HEIGHT - 60, PADDLE_WIDTH, PADDLE_HEIGHT };
    objects.ball = (struct Ball) { { objects.paddle.x + PADDLE_WIDTH / 2.f, objects.paddle.y - 20 }, 15.f, { 1.4f, -1 } };
    generate_bricks(objects.bricks);
    return objects;
}


void on_game_update(struct GameObjects* game_objects, float dt)
{
    static float prev_mouse_pos = 0.f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        game_objects->paddle.x = MAX(0, game_objects->paddle.x - 800 * dt);
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        game_objects->paddle.x = MIN(game_objects->paddle.x + 800 * dt, WINDOW_WIDTH - game_objects->paddle.width);
    }

    const float mouse_pos = GetMousePosition().x;
    if (prev_mouse_pos != mouse_pos)
    {
        game_objects->paddle.x = mouse_pos - game_objects->paddle.width / 2;
        game_objects->paddle.x = MIN(game_objects->paddle.x, WINDOW_WIDTH - game_objects->paddle.width);
        game_objects->paddle.x = MAX(0, game_objects->paddle.x);
        prev_mouse_pos = mouse_pos;
    }

    ball_move(&game_objects->ball, game_objects->paddle, dt);
    game_objects->score += ball_bricks_collision(&game_objects->ball, game_objects->bricks);
}


void on_game_render(const struct GameObjects* game_objects)
{
    DrawRectangleRec(game_objects->paddle, RED);
    DrawCircleV(game_objects->ball.center, game_objects->ball.radius, LIGHTGRAY);

    for (size_t i = 0; i < NUM_BRICKS; ++i) {
        DrawRectangleRec(game_objects->bricks[i].rec, game_objects->bricks[i].col);
    }


    const int font_size = 45;
    char str[5] = { 0 }; // max score 9999
    snprintf(str, sizeof(str), "%zu", game_objects->score);

    const int text_length = MeasureText(str, font_size);
    const int x_pos = (WINDOW_WIDTH - text_length) / 2;
    DrawText(str, x_pos, 10, font_size, GRAY);
}


enum State on_menu_update()
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
        return Game;
    return Menu;
}


void on_menu_render(const char* text, enum State game_state)
{
    const int font_size = 90;
    game_state = 1;
    const int text_length = MeasureText(text, font_size);
    const int x_pos = (WINDOW_WIDTH - text_length) / 2;
    const int y_pos = (WINDOW_HEIGHT - font_size) / 2;
    DrawText(text, x_pos, y_pos, font_size, GOLD);
}


int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    enum State game_state = Menu;
    struct GameObjects game_objects = game_objects_init();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color) { 10, 10, 10, 255 });

        switch (game_state)
        {
        case Menu:
            game_state = on_menu_update();
            on_game_render(&game_objects);
            on_menu_render("Press A|D to start", Menu);
            break;
        case Game:
            on_game_update(&game_objects, GetFrameTime());
            on_game_render(&game_objects);
            break;
        case Break:
                break;
        case Success:
        case Failed:
            break;
        }

        EndDrawing();
    }
    TerminateWindow();
}