#pragma once

#include <GLFW/glfw3.h>

#include "common.h"
#include "platform_types.h"
#include "pieces.h"

#define TETRIS_COLS 10
#define TETRIS_ROWS 20
#define MOVE_PERIOD 0.5f
#define MOVE_PERIOD_FAST 0.01f

typedef struct {
    int piece_id;
    Piece_Kind piece_kind;
} Block;

typedef struct {
    GLFWwindow *window;
    float w, h;

    GLuint prog;
    Vert_Buffer *vb;

    Block *blocks;
    int tetris_cols, tetris_rows;

    int piece_id_seed;

    Piece current_piece;
    float move_timer;
    float move_period;
    bool is_game_over;
} Game_State;
