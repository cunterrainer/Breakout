#include <stdio.h>
#include <stdbool.h>

#include "raylib.h"

#include "sounds.h"
#include "images.h"
#include "winmain.h"

#define BRICKS_HOR     10 // num of horizontal bricks
#define BRICKS_VER     7  // num of vertical bricks
#define NUM_BRICKS     BRICKS_HOR * BRICKS_VER
#define BRICK_PADDING  5
#define BRICK_Y_OFFSET 60

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))
#define KEY_REPEAT(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))


enum State
{
    Menu,
    Game,
    Break,
    Failed,
    Success,
    Reset,
    ResetAll,
    Controlls
};


struct Brick
{
    Rectangle rec;
    Color col;
};


struct Tail
{
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
};


struct Ball
{
    Vector2 center;
    float radius;
    float speed;
    Vector2 direction;
    Vector2 prev_direction;
    struct Tail tail;
};


struct GameObjects
{
    struct Brick bricks[NUM_BRICKS];
    Rectangle paddle;
    struct Ball ball;
    size_t score;
};


struct GameSettings
{
    bool make_bottom_hitbox;
    bool paddle_has_hitbox;
    bool show_stats;
    bool increase_ball_speed;
    bool auto_restart;
    bool auto_move;
};


struct ToggleSound
{
    Sound sound;
    int play;
};


struct SoundObjects
{
    struct ToggleSound hit_brick;
    struct ToggleSound hit_paddle;
    struct ToggleSound failed;
    struct ToggleSound success;
    struct ToggleSound start;
};


struct Application
{
    struct GameObjects game_objects;
    struct GameSettings game_settings;
    struct SoundObjects sound_objects;
    enum State state;
    int width;
    int height;
    int frame_rate;
    int font_size_menu;
    size_t wins;
    size_t failes;
    bool x_ray;
    bool show_fps;
    Texture2D volume_on;
    Texture2D volume_off;
};


float max_3(float a, float b, float c)
{
    return a > b ? (a > c ? a : c) : (b > c ? b : c);
}


float min_3(float a, float b, float c)
{
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}


void generate_bricks(struct Brick* bricks, int window_width, int paddle_height)
{
    const float brick_width = window_width / (float)BRICKS_HOR - BRICK_PADDING*2;

    size_t color_idx = 0;
    const Color colors[BRICKS_VER] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, SKYBLUE };

    float current_x = BRICK_PADDING;
    float current_y = BRICK_Y_OFFSET;
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


void play_sound(struct ToggleSound sound)
{
    if (!sound.play) return;

    if (IsSoundPlaying(sound.sound))
    {
        StopSound(sound.sound);
        PlaySoundRaylib(sound.sound);
    }
    else
    {
        PlaySoundRaylib(sound.sound);
    }
}


void tail_set_vertical_collision(struct Ball* ball, int from_above)
{
    const float radius = from_above ? ball->radius : -ball->radius;
    ball->tail.p3.x = (ball->tail.p1.x + ball->tail.p2.x) / 2.f;
    ball->tail.p3.y = (ball->tail.p1.y + ball->tail.p2.y) / 2.f;

    ball->tail.p1.x = ball->center.x - ball->radius * 0.5f;
    ball->tail.p1.y = ball->center.y + radius;

    ball->tail.p2.x = ball->center.x + ball->radius * 0.5f;
    ball->tail.p2.y = ball->center.y + radius;
}


void tail_set_horizontal_collision(struct Ball* ball, int right_wall)
{
    const float radius = right_wall ? ball->radius : -ball->radius;
    ball->tail.p3.x = (ball->tail.p1.x + ball->tail.p2.x) / 2.f;
    ball->tail.p3.y = (ball->tail.p1.y + ball->tail.p2.y) / 2.f;

    ball->tail.p1.x = ball->center.x + radius;
    ball->tail.p1.y = ball->center.y - ball->radius * 0.5f;

    ball->tail.p2.x = ball->center.x + radius;
    ball->tail.p2.y = ball->center.y + ball->radius * 0.5f;
}


Vector2 ball_calculate_reflected_direction(Vector2 normal, Vector2 current_direction)
{
    const float dot_product = current_direction.x * normal.x + current_direction.y * normal.y;

    Vector2 reflected_direction;
    reflected_direction.x = current_direction.x - 2.0f * dot_product * normal.x;
    reflected_direction.y = current_direction.y - 2.0f * dot_product * normal.y;
    return reflected_direction;
}


bool ball_bricks_collision(struct Ball* ball, struct Brick* bricks)
{
    for (size_t i = 0; i < NUM_BRICKS; ++i)
    {
        if (bricks[i].rec.x > 0 && CheckCollisionCircleRec(ball->center, ball->radius, bricks[i].rec))
        {
            bricks[i].rec.x = -20;
            bricks[i].rec.y = -20;
            bricks[i].rec.width = 0;

            ball->prev_direction = ball->direction;
            tail_set_vertical_collision(ball, ball->direction.y > 0);
            ball->direction = ball_calculate_reflected_direction((Vector2) { 0, 1 }, ball->direction);
            return true;
        }
    }
    return false;
}


void ball_move_tail_tip(struct Ball* ball, float speed, float dt)
{
    ball->tail.p3.x += ball->prev_direction.x * dt * speed;
    ball->tail.p3.y += ball->prev_direction.y * dt * speed;
}


void ball_animate_tail(struct Ball* ball, float dt)
{
    static const float speed_multiply = 0.9f;
    const float speed = ball->speed * speed_multiply;

    // p1 is left p2 is right
    // if from upper left to lower right
    if ((ball->prev_direction.y > 0 && ball->prev_direction.x > 0)
        && ((ball->tail.p1.x == ball->tail.p2.x && ball->tail.p3.x < ball->tail.p1.x)
            || (ball->tail.p1.y == ball->tail.p2.y && ball->tail.p3.y < ball->tail.p1.y))
        )
    {
        ball_move_tail_tip(ball, speed, dt);
    }
    // if from lower left to upper right
    else if ((ball->prev_direction.x > 0 && ball->prev_direction.y < 0)
        && ((ball->tail.p1.x == ball->tail.p2.x && ball->tail.p3.x < ball->tail.p1.x)
            || (ball->tail.p1.y == ball->tail.p2.y && ball->tail.p3.y > ball->tail.p1.y))
        )
    {
        ball_move_tail_tip(ball, speed, dt);
    }
    // if from upper right to lower left
    else if ((ball->prev_direction.x < 0 && ball->prev_direction.y > 0)
        && ((ball->tail.p1.x == ball->tail.p2.x && ball->tail.p3.x > ball->tail.p1.x)
            || (ball->tail.p1.y == ball->tail.p2.y && ball->tail.p3.y < ball->tail.p1.y))
        )
    {
        ball_move_tail_tip(ball, speed, dt);
    }
    // if from lower right to upper left
    else if ((ball->prev_direction.x < 0 && ball->prev_direction.y < 0)
        && ((ball->tail.p1.x == ball->tail.p2.x && ball->tail.p3.x > ball->tail.p1.x)
            || (ball->tail.p1.y == ball->tail.p2.y && ball->tail.p3.y > ball->tail.p1.y))
        )
    {
        ball_move_tail_tip(ball, speed, dt);
    }
    else
    {
        ball->tail.p3.x = (ball->tail.p1.x + ball->tail.p2.x) / 2.f;
        ball->tail.p3.y = (ball->tail.p1.y + ball->tail.p2.y) / 2.f;
    }
}


bool ball_move(struct Ball* ball, Rectangle paddle, Vector2 window_size, float dt, struct ToggleSound hit_sound, struct GameSettings settings)
{
    ball->center.x += ball->direction.x * dt * ball->speed;
    ball->center.y += ball->direction.y * dt * ball->speed;

    static const Vector2 normal_hor = { .x = 1, .y = 0 };
    static const Vector2 normal_ver = { .x = 0, .y = 1 };

    ball_animate_tail(ball, dt);

    if ((ball->center.x + ball->radius >= window_size.x && ball->direction.x > 0) || (ball->center.x - ball->radius <= 0 && ball->direction.x < 0))
    {
        ball->prev_direction = ball->direction;
        tail_set_horizontal_collision(ball, ball->direction.x > 0);
        ball->direction = ball_calculate_reflected_direction(normal_hor, ball->direction);
    }
    else if ((ball->center.y - ball->radius <= 0 && ball->direction.y < 0) || (settings.make_bottom_hitbox && ball->center.y + ball->radius >= window_size.y && ball->direction.y > 0))
    {
        ball->prev_direction = ball->direction;
        tail_set_vertical_collision(ball, ball->direction.y > 0);
        ball->direction = ball_calculate_reflected_direction(normal_ver, ball->direction);
    }
    else if (settings.paddle_has_hitbox && CheckCollisionCircleRec(ball->center, ball->radius, paddle) && ball->direction.y > 0)
    {
        ball->prev_direction = ball->direction;
        tail_set_vertical_collision(ball, ball->direction.y > 0);
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
    else if (!settings.make_bottom_hitbox && ball->center.y + ball->radius >= window_size.y)
        return false;
    return true;
}


struct GameObjects game_objects_init(int window_width, int window_height, int paddle_width, int paddle_height, float ball_speed /* we dont want to reset the ball speed */)
{
    struct GameObjects objects;
    objects.score = 0;
    objects.paddle = (Rectangle) { (window_width - paddle_width) / 2.f, window_height - 60, paddle_width, paddle_height };
    objects.ball = (struct Ball){ { objects.paddle.x + paddle_width / 2.f, objects.paddle.y - 20 }, 15.f, ball_speed, { 1.4f, -1 }, { 0, 0 } };
    objects.ball.tail.p1 = (Vector2) { objects.ball.center.x - 7.f, objects.paddle.y };
    objects.ball.tail.p2 = (Vector2){ objects.ball.center.x + 7.f, objects.paddle.y };
    objects.ball.tail.p3 = (Vector2) { objects.paddle.x + paddle_width / 2.f, objects.paddle.y };
    generate_bricks(objects.bricks, window_width, paddle_height);
    return objects;
}


enum State on_game_update(struct Application* app, float dt)
{
    static float prev_mouse_pos = 0.f;

    if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        return Break;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < 0)
    {
        app->game_objects.paddle.x = MAX(0, app->game_objects.paddle.x - 1250 * dt);
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0)
    {
        app->game_objects.paddle.x = MIN(app->game_objects.paddle.x + 1250 * dt, app->width - app->game_objects.paddle.width);
    }

    const float mouse_pos = GetMousePosition().x;
    if (prev_mouse_pos != mouse_pos)
    {
        app->game_objects.paddle.x = mouse_pos - app->game_objects.paddle.width / 2;
        app->game_objects.paddle.x = MIN(app->game_objects.paddle.x, app->width - app->game_objects.paddle.width);
        app->game_objects.paddle.x = MAX(0, app->game_objects.paddle.x);
        prev_mouse_pos = mouse_pos;
    }

    if (!ball_move(&app->game_objects.ball, app->game_objects.paddle, (Vector2){ app->width, app->height }, dt, app->sound_objects.hit_paddle, app->game_settings))
    {
        app->failes++;
        play_sound(app->sound_objects.failed);
        return Failed;
    }

    if (app->game_settings.auto_move)
    {
        app->game_objects.paddle.x = app->game_objects.ball.center.x - app->game_objects.paddle.width / 2.f;
        app->game_objects.paddle.x = MIN(app->game_objects.paddle.x, app->width - app->game_objects.paddle.width);
        app->game_objects.paddle.x = MAX(0, app->game_objects.paddle.x);
    }

    if (ball_bricks_collision(&app->game_objects.ball, app->game_objects.bricks))
    {
        play_sound(app->sound_objects.hit_brick);
        app->game_objects.score++;
        if (app->game_settings.increase_ball_speed)
            app->game_objects.ball.speed += 6.f;
    }
    if (app->game_objects.score == NUM_BRICKS)
    {
        app->wins++;
        play_sound(app->sound_objects.success);
        return Success;
    }
    return Game;
}


Vector2 min_vector_x(Vector2 v1, Vector2 v2)
{
    return v1.x < v2.x ? v1 : v2;
}


Vector2 max_vector_x(Vector2 v1, Vector2 v2)
{
    return v1.x > v2.x ? v1 : v2;
}


Vector2 min_vector_y(Vector2 v1, Vector2 v2)
{
    return v1.y < v2.y ? v1 : v2;
}


Vector2 max_vector_y(Vector2 v1, Vector2 v2)
{
    return v1.y > v2.y ? v1 : v2;
}


void draw_triangle(Vector2 p1, Vector2 p2, Vector2 p3, Color color)
{
    Vector2 first;
    Vector2 second;
    Vector2 third;

    const float min = min_3(p1.y, p2.y, p3.y);

    if (p3.y == min)
    {
        first = p3;

        if (p1.y == p2.y)
        {
            second = min_vector_x(p1, p2);
            third = max_vector_x(p1, p2);
        }
        else if (p3.x > p1.x)
        {
            second = min_vector_y(p1, p2);
            third = max_vector_y(p1, p2);
        }
        else
        {
            second = max_vector_y(p1, p2);
            third = min_vector_y(p1, p2);
        }
    }
    else if (p1.y < p2.y)
    {
        first = p1;
        second = min_vector_x(p2, p3);
        third = max_vector_x(p2, p3);
    }
    else if (p2.y < p1.y)
    {
        first = p2;
        second = min_vector_x(p1, p3);
        third = max_vector_x(p1, p3);
    }
    else // p1.y == p2.y
    {
        first = min_vector_x(p1, p2);
        second = p3;
        third = max_vector_x(p1, p2);
    }

    DrawTriangle(first, second, third, color);
}


void game_render(const struct GameObjects* game_objects, Vector2 ball_p1, Vector2 ball_p2, Color tail_color)
{
    draw_triangle(game_objects->ball.tail.p1, game_objects->ball.tail.p2, ball_p1, tail_color);
    draw_triangle(ball_p1, ball_p2, game_objects->ball.tail.p2, tail_color);
    draw_triangle(game_objects->ball.tail.p1, game_objects->ball.tail.p2, game_objects->ball.tail.p3, tail_color);

    DrawRectangleRec(game_objects->paddle, RED);
    DrawCircleV(game_objects->ball.center, game_objects->ball.radius, LIGHTGRAY);

    for (size_t i = 0; i < NUM_BRICKS; ++i) {
        DrawRectangleRec(game_objects->bricks[i].rec, game_objects->bricks[i].col);
    }
}


void game_render_xray(const struct GameObjects* game_objects, Vector2 ball_p1, Vector2 ball_p2, Color tail_color)
{
    DrawLineV(ball_p1, game_objects->ball.tail.p1, tail_color);
    DrawLineV(ball_p2, game_objects->ball.tail.p2, tail_color);
    DrawLineV(game_objects->ball.tail.p1, game_objects->ball.tail.p3, tail_color);
    DrawLineV(game_objects->ball.tail.p2, game_objects->ball.tail.p3, tail_color);

    DrawRectangleLinesEx(game_objects->paddle, 1.f, RED);
    DrawCircleLinesV(game_objects->ball.center, game_objects->ball.radius, LIGHTGRAY);

    for (size_t i = 0; i < NUM_BRICKS; ++i) {
        DrawRectangleLinesEx(game_objects->bricks[i].rec, 1.f, game_objects->bricks[i].col);
    }
}


void on_game_render(const struct Application* app)
{
    const struct Ball* ball = &app->game_objects.ball;
    const struct Tail* tail = &app->game_objects.ball.tail;

    Vector2 ball_p1 = { .x = ball->center.x, .y = ball->center.y - ball->radius };
    Vector2 ball_p2 = { .x = ball->center.x, .y = ball->center.y + ball->radius };

    Vector2 coll_point;
    if (CheckCollisionLines(ball_p1, tail->p1, ball_p2, tail->p2, &coll_point))
    {
        const Vector2 tmp = ball_p1;
        ball_p1 = ball_p2;
        ball_p2 = tmp;
    }

    static const int score_font_size = 45;
    char score_str[5] = { 0 }; // max score 9999
    snprintf(score_str, ARRAY_SIZE(score_str), "%zu", app->game_objects.score);

    const int text_length = MeasureText(score_str, score_font_size);
    const int score_x_pos = (app->width - text_length) / 2;

    static const Color tail_color = { .r = 200, .g = 200, .b = 200, .a = 70 };
    if (!app->x_ray)
    {
        game_render(&app->game_objects, ball_p1, ball_p2, tail_color);
    }
    else
    {
        game_render_xray(&app->game_objects, ball_p1, ball_p2, tail_color);
    }

    DrawText(score_str, score_x_pos, 10, score_font_size, GRAY); // otherwise ball will be rendered on top of the score

    if (app->game_settings.show_stats)
    {
        const char* ball_speed_str = TextFormat("W: %zu F: %zu %zu", app->wins, app->failes, (size_t)app->game_objects.ball.speed);
        const int speed_length = MeasureText(ball_speed_str, score_font_size);
        DrawText(ball_speed_str, app->width - speed_length - 10, 10, score_font_size, GRAY);
    }
}


void menu_render_controll(int font_size, const char* text, Color color, bool reset)
{
    static int y_pos = 10;
    DrawText(text, 10, y_pos, font_size, color);
    y_pos += font_size;
    if (reset) y_pos = 10;
}


enum State menu_show_controlls(const struct Application* app)
{
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0 || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < 0 || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        return Break;

    DrawRectangle(app->width / 2.f - 25, 0, 50, 55, (Color) { 10, 10, 10, 255 }); // Draw over the score
    DrawRectangle(0, BRICK_Y_OFFSET, app->width, app->height - BRICK_Y_OFFSET, (Color) { 10, 10, 10, 255 });

    const int font_size = (app->height - 10) / 31;
    menu_render_controll(font_size, "Keyboard", WHITE, false);
    menu_render_controll(font_size, "(A|D|Left|Right) Controll the paddle", WHITE, false);
    menu_render_controll(font_size, "(W|A|Up|Down) Increase/Decrease the ball's speed", WHITE, false);
    menu_render_controll(font_size, "(Space) Launch the ball at the start of the game or resume after a failed attempt", WHITE, false);
    menu_render_controll(font_size, "(R) Reset the game (Doesn't reset the ball speed, wins and fails", WHITE, false);
    menu_render_controll(font_size, "(ESC) Pause/resume the game", WHITE, false);
    menu_render_controll(font_size, "(.|,) Increase/Decrease the fps limit (if limit = 0, there is no limit)", WHITE, false);
    menu_render_controll(font_size, "(L) Reset the game (Including ball speed, wins and fails", WHITE, false);

    menu_render_controll(font_size, "(X) Render only the outlines of objects", app->x_ray ? GREEN : RED, false);
    menu_render_controll(font_size, "(O) Auto move the paddle", app->game_settings.auto_move ? GREEN : RED, false);
    menu_render_controll(font_size, "(U) Auto restart after success or failure", app->game_settings.auto_restart ? GREEN : RED, false);
    menu_render_controll(font_size, "(G) Bottom has hitbox (Game can no longer be lost)", app->game_settings.make_bottom_hitbox ? GREEN : RED, false);
    menu_render_controll(font_size, "(P) Paddle has hitbox", app->game_settings.paddle_has_hitbox ? GREEN : RED, false);
    menu_render_controll(font_size, "(B) Show the game stats (wins, fails, ball speed)", app->game_settings.show_stats ? GREEN : RED, false);
    menu_render_controll(font_size, "(I) Ball speed increases when scored", app->game_settings.increase_ball_speed ? GREEN : RED, false);
    menu_render_controll(font_size, "(F) Show fps", app->show_fps ? GREEN : RED, false);
    menu_render_controll(font_size, "(M) Mute game audio", !app->sound_objects.failed.play ? GREEN : RED, false);


    menu_render_controll(font_size, "", WHITE, false);
    menu_render_controll(font_size, "Controller (PS4 layout as example)", WHITE, false);
    menu_render_controll(font_size, "(DPAD or Left stick) Controll the paddle", WHITE, false);
    menu_render_controll(font_size, "(DPAD Up|Down) Increase/Decrease the ball's speed", WHITE, false);
    menu_render_controll(font_size, "(X) Launch the ball at the start of the game or resume after a failed attempt", WHITE, false);
    menu_render_controll(font_size, "(/\\) Reset the game (Doesn't reset the ball speed, wins and fails", WHITE, false);
    menu_render_controll(font_size, "(OPTIONS) Pause/resume the game", WHITE, false);

    menu_render_controll(font_size, "([]) Render only the outlines of objects", app->x_ray ? GREEN : RED, false);
    menu_render_controll(font_size, "(L1) Show the game stats (wins, fails, ball speed)", app->game_settings.show_stats ? GREEN : RED, false);
    menu_render_controll(font_size, "(R1) Ball speed increases when scored", app->game_settings.increase_ball_speed ? GREEN : RED, false);
    menu_render_controll(font_size, "(SHARE) Show fps", app->show_fps ? GREEN : RED, false);
    menu_render_controll(font_size, "(O) Mute game audio", !app->sound_objects.failed.play ? GREEN : RED, false);
    menu_render_controll(font_size, "(Right Stick pressed) Bottom has hitbox (Game can no longer be lost)", app->game_settings.make_bottom_hitbox ? GREEN : RED, false);
    menu_render_controll(font_size, "(Left Stick pressed) Paddle has hitbox", app->game_settings.paddle_has_hitbox ? GREEN : RED, true);
    return app->state;
}


enum State on_menu_update(const struct Application* app, const char* text)
{
    const int text_length = MeasureText(text, app->font_size_menu);
    const int x_pos = (app->width - text_length) / 2;
    const int y_pos = (app->height - app->font_size_menu) / 2;

    DrawTextureEx(app->sound_objects.start.play ? app->volume_on : app->volume_off, (Vector2) { app->width - 70.f, 10 }, 0, 0.08f, WHITE);

    switch (app->state)
    {
    case Menu:
    case Break:
        DrawText(text, x_pos, y_pos, app->font_size_menu, DARKGRAY);
        if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0 || GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < 0 || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        {
            play_sound(app->sound_objects.start);
            return Game;
        }
        return IsKeyPressed(KEY_R) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP) ? Reset : (IsKeyPressed(KEY_L) ? ResetAll : app->state);
    case Success:
        DrawText(text, x_pos, y_pos, app->font_size_menu, GOLD);
        break;
    case Failed:
        DrawText(text, x_pos, y_pos, app->font_size_menu, RED);
        break;
    default:
        break;
    }

    if (IsKeyPressed(KEY_L))
        return ResetAll;

    if (app->game_settings.auto_restart || IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))
        return Reset;
    return app->state;
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
    app->game_objects.ball.tail.p1.x = app_transform(app->game_objects.ball.tail.p1.x, app->width, new_width);
    app->game_objects.ball.tail.p1.y = app_transform(app->game_objects.ball.tail.p1.y, app->height, new_height);
    app->game_objects.ball.tail.p2.x = app_transform(app->game_objects.ball.tail.p2.x, app->width, new_width);
    app->game_objects.ball.tail.p2.y = app_transform(app->game_objects.ball.tail.p2.y, app->height, new_height);
    app->game_objects.ball.tail.p3.x = app_transform(app->game_objects.ball.tail.p3.x, app->width, new_width);
    app->game_objects.ball.tail.p3.y = app_transform(app->game_objects.ball.tail.p3.y, app->height, new_height);

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


void app_load_audio(struct Application* app)
{
    Wave w1 = LoadWaveFromMemory(".wav", sg_Hit_sound,        ARRAY_SIZE(sg_Hit_sound));
    Wave w2 = LoadWaveFromMemory(".wav", sg_Hit_paddle_sound, ARRAY_SIZE(sg_Hit_paddle_sound));
    Wave w3 = LoadWaveFromMemory(".wav", sg_Start_sound,      ARRAY_SIZE(sg_Start_sound));
    Wave w4 = LoadWaveFromMemory(".wav", sg_Game_faile_sound, ARRAY_SIZE(sg_Game_faile_sound));
    Wave w5 = LoadWaveFromMemory(".wav", sg_Game_win_sound,   ARRAY_SIZE(sg_Game_win_sound));

    app->sound_objects.hit_brick.sound  = LoadSoundFromWave(w1);
    app->sound_objects.hit_paddle.sound = LoadSoundFromWave(w2);
    app->sound_objects.start.sound      = LoadSoundFromWave(w3);
    app->sound_objects.failed.sound     = LoadSoundFromWave(w4);
    app->sound_objects.success.sound    = LoadSoundFromWave(w5);

    app->sound_objects.hit_brick.play  = 1;
    app->sound_objects.hit_paddle.play = 1;
    app->sound_objects.start.play      = 1;
    app->sound_objects.failed.play     = 1;
    app->sound_objects.success.play    = 1;

    UnloadWave(w1);
    UnloadWave(w2);
    UnloadWave(w3);
    UnloadWave(w4);
    UnloadWave(w5);
}


Texture2D load_image(const unsigned char* data, int size)
{
    Image img = LoadImageFromMemory(".png", data, size);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}


struct Application app_start()
{
    struct Application app;
    app.wins = 0;
    app.failes = 0;
    app.width = 1200;
    app.height = 750;
    app.state = Menu;
    app.x_ray = false;
    app.show_fps = false;
    app.frame_rate = 60;
    app.font_size_menu = 90;
    app.game_objects = game_objects_init(app.width, app.height, 230, 30, 500.f);
    app.game_settings = (struct GameSettings){ .make_bottom_hitbox = false, .paddle_has_hitbox = true, .show_stats = false, .increase_ball_speed = true, .auto_restart = false, .auto_move = false };

    InitAudioDevice();
    InitWindow(app.width, app.height, "Breakout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetExitKey(KEY_NULL);
    Image icon = LoadImageFromMemory(".png", sg_Icon_image, ARRAY_SIZE(sg_Icon_image));
    SetWindowIcon(icon);
    UnloadImage(icon);

    app_load_audio(&app);
    app.volume_on = load_image(sg_Volume_on_image, ARRAY_SIZE(sg_Volume_on_image));
    app.volume_off = load_image(sg_Volume_off_image, ARRAY_SIZE(sg_Volume_off_image));
    return app;
}


void app_shutdown(const struct Application* app)
{
    UnloadTexture(app->volume_on);
    UnloadTexture(app->volume_off);
    UnloadSound(app->sound_objects.success.sound);
    UnloadSound(app->sound_objects.failed.sound);
    UnloadSound(app->sound_objects.start.sound);
    UnloadSound(app->sound_objects.hit_brick.sound);
    UnloadSound(app->sound_objects.hit_paddle.sound);
    CloseAudioDevice();
    TerminateWindow();
}


void on_app_key_input(struct Application* app)
{
    if (IsKeyPressed(KEY_X) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
    {
        app->x_ray = !app->x_ray;
    }

    if (IsKeyPressed(KEY_M) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
    {
        app->sound_objects.hit_brick.play = !app->sound_objects.hit_brick.play;
        app->sound_objects.hit_paddle.play = !app->sound_objects.hit_paddle.play;
        app->sound_objects.start.play = !app->sound_objects.start.play;
        app->sound_objects.failed.play = !app->sound_objects.failed.play;
        app->sound_objects.success.play = !app->sound_objects.success.play;
    }

    if (IsKeyPressed(KEY_F3) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))
    {
        app->state = Controlls;
    }

    if (IsKeyPressed(KEY_F) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT))
    {
        app->show_fps = !app->show_fps;
    }

    if (IsKeyPressed(KEY_G) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_THUMB))
    {
        app->game_settings.make_bottom_hitbox = !app->game_settings.make_bottom_hitbox;
    }

    if (IsKeyPressed(KEY_P) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_THUMB))
    {
        app->game_settings.paddle_has_hitbox = !app->game_settings.paddle_has_hitbox;
    }

    if (IsKeyPressed(KEY_B) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
    {
        app->game_settings.show_stats = !app->game_settings.show_stats;
    }

    if (IsKeyPressed(KEY_I) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
    {
        app->game_settings.increase_ball_speed = !app->game_settings.increase_ball_speed;
    }

    if (IsKeyPressed(KEY_O))
    {
        app->game_settings.auto_move = !app->game_settings.auto_move;
    }

    if (IsKeyPressed(KEY_U))
    {
        app->game_settings.auto_restart = !app->game_settings.auto_restart;
    }

    if (KEY_REPEAT(KEY_PERIOD))
    {
        app->frame_rate += 1;
        SetTargetFPS(app->frame_rate);
    }

    if (KEY_REPEAT(KEY_COMMA))
    {
        app->frame_rate -= 1;
        app->frame_rate = MAX(app->frame_rate, 0);
        SetTargetFPS(app->frame_rate);
    }

    if (KEY_REPEAT(KEY_W))
    {
        app->game_objects.ball.speed += 100.f;
        app->game_objects.ball.speed = MIN(app->game_objects.ball.speed, 10000000);
    }

    if (KEY_REPEAT(KEY_S))
    {
        app->game_objects.ball.speed -= 100.f;
        app->game_objects.ball.speed = MAX(app->game_objects.ball.speed, 1);
    }

    if (KEY_REPEAT(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
    {
        app->game_objects.ball.speed += 10.f;
        app->game_objects.ball.speed = MIN(app->game_objects.ball.speed, 10000000);
    }

    if (KEY_REPEAT(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
    {
        app->game_objects.ball.speed -= 10.f;
        app->game_objects.ball.speed = MAX(app->game_objects.ball.speed, 1);
    }
}


int main()
{
    struct Application app = app_start();

    while (!WindowShouldClose())
    {
        on_app_key_input(&app);

        if (IsWindowResized())
        {
            on_app_resize(&app, GetScreenWidth(), GetScreenHeight());
        }

        BeginDrawing();
        ClearBackground((Color) { 10, 10, 10, 255 });

        if (app.show_fps)
        {
            DrawFPS(10, 10);
        }


        switch (app.state)
        {
        case Menu:
            on_game_render(&app);
            app.state = on_menu_update(&app, "Press A|D to start"); // to render the menu on top of the game not vice versa
            break;
        case Game:
            app.state = on_game_update(&app, GetFrameTime());
            on_game_render(&app);
            break;
        case Break:
            on_game_render(&app);
            app.state = on_menu_update(&app, "Paused");
            break;
        case Success:
            on_game_render(&app);
            app.state = on_menu_update(&app, "You won!");
            break;
        case Failed:
            on_game_render(&app);
            app.state = on_menu_update(&app, "You lost!");
            break;
        case Reset:
            app.game_objects = game_objects_init(app.width, app.height, 230, 30, app.game_objects.ball.speed);
            if (app.game_settings.auto_restart)
                app.state = Game;
            else
                app.state = Menu;
            break;
        case ResetAll:
            app.wins = 0;
            app.failes = 0;
            app.game_objects = game_objects_init(app.width, app.height, 230, 30, 500.f);
            app.state = app.game_settings.auto_restart ? Game : Menu;
            break;
        case Controlls:
            on_game_render(&app); // render the game to see stats like the ball speed etc.
            app.state = menu_show_controlls(&app);
            break;
        default:
            break;
        }

        EndDrawing();
    }
    app_shutdown(&app);
}