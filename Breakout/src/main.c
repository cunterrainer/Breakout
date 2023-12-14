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
    Success,
    Reset
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


int ball_move(struct Ball* ball, Rectangle paddle, float dt)
{
    const float speed = 500.f;
    ball->center.x += ball->direction.x * dt * speed;
    ball->center.y += ball->direction.y * dt * speed;

    const Vector2 normal_hor = { .x = 1, .y = 0 };
    const Vector2 normal_ver = { .x = 0, .y = 1 };

    if ((ball->center.x + ball->radius >= WINDOW_WIDTH && ball->direction.x > 0) || (ball->center.x - ball->radius <= 0 && ball->direction.x < 0))
    {
        ball->direction = ball_calculate_reflected_direction(normal_hor, ball->direction);
    }
    else if (ball->center.y - ball->radius <= 0 && ball->direction.y < 0)
    {
        ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
    }
    else if (CheckCollisionCircleRec(ball->center, ball->radius, paddle) && ball->direction.y > 0)
    {
        // if you hit the ball in the first 25 % the ball goes back the direction reverses
        // if going from left to right                                                    if going from right to left
        if ((ball->direction.x > 0 && ball->center.x < paddle.x + paddle.width * 0.25) || (ball->direction.x < 0 && ball->center.x > paddle.x + paddle.width * 0.75))
        {
            ball->direction.x = -ball->direction.x;
            ball->direction.y = -ball->direction.y;
        }
        else
        {
            ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
        }
    }
    else if (ball->center.y + ball->radius >= WINDOW_HEIGHT)
        return 0;
    return 1;
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


enum State on_game_update(struct GameObjects* game_objects, float dt)
{
    static float prev_mouse_pos = 0.f;

    if (IsKeyDown(KEY_ESCAPE))
        return Break;

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

    if (!ball_move(&game_objects->ball, game_objects->paddle, dt))
        return Failed;

    game_objects->score += ball_bricks_collision(&game_objects->ball, game_objects->bricks);
    if (game_objects->score == NUM_BRICKS)
        return Success;
    return Game;
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


void on_game_resize(struct GameObjects* game_objects, float width, float height)
{
    const float brick_width = width / (float)BRICKS_HOR - BRICK_PADDING * 2;
    const float brick_height = height / (WINDOW_HEIGHT / (float)PADDLE_HEIGHT);

    float current_x = BRICK_PADDING;
    float current_y = 60;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        if (game_objects->bricks[i].rec.width == 0)
            continue;
        game_objects->bricks[i].rec.x = current_x;
        game_objects->bricks[i].rec.y = current_y;
        game_objects->bricks[i].rec.width = brick_width;
        game_objects->bricks[i].rec.height = brick_height;
        current_x += brick_width + BRICK_PADDING * 2;

        if ((i + 1) % BRICKS_HOR == 0)
        {
            current_x = BRICK_PADDING;
            current_y += brick_height + 5;
        }
    }
}


enum State on_menu_update(const char* text, enum State game_state)
{
    const int font_size = 90;
    const int text_length = MeasureText(text, font_size);
    const int x_pos = (WINDOW_WIDTH - text_length) / 2;
    const int y_pos = (WINDOW_HEIGHT - font_size) / 2;

    switch (game_state)
    {
    case Menu:
    case Break:
        DrawText(text, x_pos, y_pos, font_size, DARKGRAY);
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_SPACE) /* pressed instead of down because otherwise you'd always jump back in the game after pressing space when failed or finished */)
            return Game;
        if (IsKeyPressed(KEY_R))
            return Reset;
        return game_state;
    case Success:
        DrawText(text, x_pos, y_pos, font_size, GOLD);
        break;
    case Failed:
        DrawText(text, x_pos, y_pos, font_size, RED);
        break;
    default:
        break;
    }

    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE))
        return Reset;
    return game_state;
}


int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetExitKey(KEY_NULL);

    enum State game_state = Menu;
    struct GameObjects game_objects = game_objects_init();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color) { 10, 10, 10, 255 });

        if (IsWindowResized())
        {
            on_game_resize(&game_objects, (float)GetScreenWidth(), (float)GetScreenHeight());
        }

        switch (game_state)
        {
        case Menu:
            on_game_render(&game_objects);
            game_state = on_menu_update("Press A|D to start", Menu); // to render the menu on top of the game not vice versa
            break;
        case Game:
            game_state = on_game_update(&game_objects, GetFrameTime());
            on_game_render(&game_objects);
            break;
        case Break:
            on_game_render(&game_objects);
            game_state = on_menu_update("Paused", Break);
            break;
        case Success:
            on_game_render(&game_objects);
            game_state = on_menu_update("You won!", Success);
            break;
        case Failed:
            on_game_render(&game_objects);
            game_state = on_menu_update("You lost!", Failed);
            break;
        case Reset:
            game_objects = game_objects_init();
            game_state = Menu;
            break;
        default:
            break;
        }

        EndDrawing();
    }
    TerminateWindow();
}