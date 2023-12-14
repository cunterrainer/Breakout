#include <stdio.h>

#include "raylib.h"

#include "core.h"
#include "winmain.h"

#define BRICKS_HOR    10 // num of horizontal bricks
#define BRICKS_VER    7  // num of vertical bricks
#define NUM_BRICKS    BRICKS_HOR * BRICKS_VER
#define BRICK_PADDING 5

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)


enum State
{
    Menu,
    Game,
    Break,
    Failed,
    Success,
    Reset
};


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


struct Application
{
    struct GameObjects game_objects;
    enum State state;
    int width;
    int height;
    int font_size_menu;
    Sound sound_hit_brick;
    Sound sound_hit_paddle;
    Sound sound_failed;
    Sound sound_success;
    Sound sound_start;
};


void generate_bricks(struct Brick* bricks, int window_width, int paddle_height)
{
    const float brick_width = window_width / (float)BRICKS_HOR - BRICK_PADDING*2;

    size_t color_idx = 0;
    const Color colors[BRICKS_VER] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, SKYBLUE };

    float current_x = BRICK_PADDING;
    float current_y = 60;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        bricks[i].rec.x = current_x;
        bricks[i].rec.y = current_y;
        bricks[i].rec.width = brick_width;
        bricks[i].rec.height = paddle_height;
        bricks[i].col = colors[color_idx];
        current_x += brick_width + BRICK_PADDING*2;

        if ((i + 1) % BRICKS_HOR == 0)
        {
            current_x = BRICK_PADDING;
            current_y += paddle_height + 5;
            color_idx++;
        }
    }
}


void play_sound(Sound sound)
{
    if (IsSoundPlaying(sound))
    {
        StopSound(sound);
        PlaySound(sound);
    }
    else
    {
        PlaySound(sound);
    }
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
            //bricks[i].rec.height = 0;
            ++collisions;

            ball->direction = ball_calculate_reflected_direction((Vector2) { 0, 1 }, ball->direction);
            break;
        }
    }
    return collisions;
}


int ball_move(struct Ball* ball, Rectangle paddle, int window_width, int window_height, float dt, Sound hit_sound)
{
    const float speed = 500.f;
    ball->center.x += ball->direction.x * dt * speed;
    ball->center.y += ball->direction.y * dt * speed;

    const Vector2 normal_hor = { .x = 1, .y = 0 };
    const Vector2 normal_ver = { .x = 0, .y = 1 };

    if ((ball->center.x + ball->radius >= window_width && ball->direction.x > 0) || (ball->center.x - ball->radius <= 0 && ball->direction.x < 0))
    {
        ball->direction = ball_calculate_reflected_direction(normal_hor, ball->direction);
    }
    else if (ball->center.y - ball->radius <= 0 && ball->direction.y < 0) // || ball->center.y + ball->radius >= window_height && ball->direction.y > 0
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
        play_sound(hit_sound);
    }
    else if (ball->center.y + ball->radius >= window_height)
        return 0;
    return 1;
}


struct GameObjects game_objects_init(int window_width, int window_height, int paddle_width, int paddle_height)
{
    struct GameObjects objects;
    objects.score = 0;
    objects.paddle = (Rectangle){ (window_width - paddle_width) / 2.f, window_height - 60, paddle_width, paddle_height };
    objects.ball = (struct Ball) { { objects.paddle.x + paddle_width / 2.f, objects.paddle.y - 20 }, 15.f, { 1.4f, -1 } };
    generate_bricks(objects.bricks, window_width, paddle_height);
    return objects;
}


enum State on_game_update(struct Application* app, float dt)
{
    static float prev_mouse_pos = 0.f;

    if (IsKeyDown(KEY_ESCAPE))
        return Break;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        app->game_objects.paddle.x = MAX(0, app->game_objects.paddle.x - 800 * dt);
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        app->game_objects.paddle.x = MIN(app->game_objects.paddle.x + 800 * dt, app->width - app->game_objects.paddle.width);
    }

    const float mouse_pos = GetMousePosition().x;
    if (prev_mouse_pos != mouse_pos)
    {
        app->game_objects.paddle.x = mouse_pos - app->game_objects.paddle.width / 2;
        app->game_objects.paddle.x = MIN(app->game_objects.paddle.x, app->width - app->game_objects.paddle.width);
        app->game_objects.paddle.x = MAX(0, app->game_objects.paddle.x);
        prev_mouse_pos = mouse_pos;
    }

    if (!ball_move(&app->game_objects.ball, app->game_objects.paddle, app->width, app->height, dt, app->sound_hit_paddle))
        return Failed;

    const size_t collision = ball_bricks_collision(&app->game_objects.ball, app->game_objects.bricks);
    if (collision)
    {
        play_sound(app->sound_hit_brick);
        app->game_objects.score += collision;
    }
    if (app->game_objects.score == NUM_BRICKS)
        return Success;
    return Game;
}


void on_game_render(const struct GameObjects* game_objects, int window_width)
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
    const int x_pos = (window_width - text_length) / 2;
    DrawText(str, x_pos, 10, font_size, GRAY);
}


float app_transform(float old_value, float old_dimension, float new_dimension)
{
    return old_value / old_dimension * new_dimension;
}


void on_app_resize(struct Application* app, int new_width, int new_height)
{
    const float brick_width = new_width / (float)BRICKS_HOR - BRICK_PADDING * 2;
    const float brick_height = app_transform(app->game_objects.bricks[0].rec.height, app->height, new_height);

    app->game_objects.paddle.height = brick_height;
    app->game_objects.paddle.width = app_transform(app->game_objects.paddle.width, app->width, new_width);
    app->game_objects.paddle.x = app_transform(app->game_objects.paddle.x, app->width, new_width);
    app->game_objects.paddle.y = app_transform(app->game_objects.paddle.y, app->height, new_height);

    app->game_objects.ball.center.x = app_transform(app->game_objects.ball.center.x, app->width, new_width);
    app->game_objects.ball.center.y = app_transform(app->game_objects.ball.center.y, app->height, new_height);
    app->game_objects.ball.radius = app_transform(app->game_objects.ball.radius, app->width, new_width);

    float current_x = BRICK_PADDING;
    float current_y = 60;
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        if (app->game_objects.bricks[i].rec.width == 0)
            continue;
        app->game_objects.bricks[i].rec.x = current_x;
        app->game_objects.bricks[i].rec.y = current_y;
        app->game_objects.bricks[i].rec.width = brick_width;
        app->game_objects.bricks[i].rec.height = brick_height;
        current_x += brick_width + BRICK_PADDING * 2;

        if ((i + 1) % BRICKS_HOR == 0)
        {
            current_x = BRICK_PADDING;
            current_y += brick_height + 5;
        }
    }

    app->font_size_menu = app_transform(app->font_size_menu, MAX(app->width, app->height), MAX(new_width, new_height));
    app->width = new_width;
    app->height = new_height;
}


enum State on_menu_update(const struct Application* app, const char* text)
{
    const int text_length = MeasureText(text, app->font_size_menu);
    const int x_pos = (app->width - text_length) / 2;
    const int y_pos = (app->height - app->font_size_menu) / 2;

    switch (app->state)
    {
    case Menu:
    case Break:
        DrawText(text, x_pos, y_pos, app->font_size_menu, DARKGRAY);
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_SPACE) /* pressed instead of down because otherwise you'd always jump back in the game after pressing space when failed or finished */)
        {
            play_sound(app->sound_start);
            return Game;
        }
        if (IsKeyPressed(KEY_R))
            return Reset;
        return app->state;
    case Success:
        DrawText(text, x_pos, y_pos, app->font_size_menu, GOLD);
        break;
    case Failed:
        DrawText(text, x_pos, y_pos, app->font_size_menu, RED);
        break;
    default:
        break;
    }

    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE))
        return Reset;
    return app->state;
}


int main()
{
    struct Application app;
    app.width = 1200;
    app.height = 750;
    app.state = Menu;
    app.font_size_menu = 90;
    app.game_objects = game_objects_init(app.width, app.height, 230, 30);
    
    InitAudioDevice();
    app.sound_hit_brick = LoadSound("data/hit.wav");
    app.sound_hit_paddle = LoadSound("data/hit_paddle.wav");
    app.sound_start = LoadSound("data/start.wav");
    InitWindow(app.width, app.height, "Breakout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color) { 10, 10, 10, 255 });

        if (IsWindowResized())
        {
            on_app_resize(&app, GetScreenWidth(), GetScreenHeight());
        }

        switch (app.state)
        {
        case Menu:
            on_game_render(&app.game_objects, app.width);
            app.state = on_menu_update(&app, "Press A|D to start"); // to render the menu on top of the game not vice versa
            break;
        case Game:
            app.state = on_game_update(&app, GetFrameTime());
            on_game_render(&app.game_objects, app.width);
            break;
        case Break:
            on_game_render(&app.game_objects, app.width);
            app.state = on_menu_update(&app, "Paused");
            break;
        case Success:
            on_game_render(&app.game_objects, app.width);
            app.state = on_menu_update(&app, "You won!");
            break;
        case Failed:
            on_game_render(&app.game_objects, app.width);
            app.state = on_menu_update(&app, "You lost!");
            break;
        case Reset:
            app.game_objects = game_objects_init(app.width, app.height, 230, 30);
            app.state = Menu;
            break;
        default:
            break;
        }

        EndDrawing();
    }
    UnloadSound(app.sound_start);
    UnloadSound(app.sound_hit_brick);
    UnloadSound(app.sound_hit_paddle);
    CloseAudioDevice();
    TerminateWindow();
}