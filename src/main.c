#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include "gl_glue.h"
#include "tetris.h"

#include "tetris.c"

void on_init(Game_State *state, GLFWwindow *window, float window_w, float window_h, float window_px_w, float window_px_h, bool is_live_scene, GLuint fbo, int argc, char **argv)
{
    state->window = window;
    state->w = window_w;
    state->h = window_h;

    state->tetris_cols = TETRIS_COLS;
    state->tetris_rows = TETRIS_ROWS;
    state->move_period = MOVE_PERIOD;

    create_shaders(state);
    create_vert_buffer(state);
    initialize_game(state);
}

void on_reload(Game_State *state)
{
}

void on_frame(Game_State *state, const Platform_Timing *t)
{
    glViewport(0, 0, (GLsizei)state->w, (GLsizei)state->h);

    glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->prog);
    Mat_4 proj = mat4_proj_ortho(0, state->w, state->h, 0, -1, 1);

    glUniformMatrix4fv(glGetUniformLocation(state->prog, "u_mvp"), 1, GL_FALSE, proj.m);

    if (!state->is_game_over)
    {
        state->move_timer += t->prev_delta_time;
        if (state->move_timer >= state->move_period)
        {
            state->move_timer -= state->move_period;
            if (!move_current_piece_down(state))
            {
                commit_piece(state, &state->current_piece);
                check_lines(state);
                state->move_period = MOVE_PERIOD;
                state->move_timer = 0.0f;
                if (!generate_new_piece(state))
                {
                    state->is_game_over = true;
                }
            }
        }
    }

    draw(state);
}

void on_platform_event(Game_State *state, const Platform_Event *e)
{
    switch (e->kind)
    {
        case PLATFORM_EVENT_KEY:
        {
            if (e->key.action == GLFW_PRESS && state->is_game_over)
            {
                initialize_game(state);
                return;
            }

            if (e->key.key == GLFW_KEY_UP &&
                (e->key.action == GLFW_PRESS || e->key.action == GLFW_REPEAT))
            {
                rotate_current_piece(state);
            }

            if ((e->key.key == GLFW_KEY_LEFT || e->key.key == GLFW_KEY_RIGHT) &&
                (e->key.action == GLFW_PRESS || e->key.action == GLFW_REPEAT))
            {
                slide_current_piece(state, (e->key.key == GLFW_KEY_RIGHT) ? +1 : -1);
            }

            if (e->key.key == GLFW_KEY_DOWN)
            {
                if (e->key.action == GLFW_PRESS)
                {
                    state->move_period = MOVE_PERIOD_FAST;
                    state->move_timer = state->move_period;
                }
                else if (e->key.action == GLFW_RELEASE)
                {
                    state->move_period = MOVE_PERIOD;
                    state->move_timer = 0.0f;
                }
            }
        } break;
        default: break;
    }
}

void on_destroy(Game_State *state)
{
    free(state->blocks);
}
