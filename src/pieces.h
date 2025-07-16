#pragma once

#include "common.h"
#include "gl_glue.h"

#define PIECE_MAX_COLS 4
#define PIECE_MAX_ROWS 4

typedef enum {
    PIECE_T,
    PIECE_L,
    PIECE_S,
    PIECE_O,
    PIECE_I,
    PIECE_J,
    PIECE_Z,
    PIECE_KIND_COUNT,
} Piece_Kind;

typedef enum {
    PIECE_ORIENT_UP,
    PIECE_ORIENT_RIGHT,
    PIECE_ORIENT_DOWN,
    PIECE_ORIENT_LEFT,
    PIECE_ORIENT_COUNT
} Piece_Orient;

typedef struct {
    int cols, rows;
    Col_3f color;
    Piece_Kind kind;
    int blocks[PIECE_MAX_COLS * PIECE_MAX_ROWS];
} Piece_Spec;

typedef struct {
    int id;
    int x, y;
    Piece_Kind kind;
    Piece_Orient orient;
} Piece;

static inline int piece_spec_get_block_state_at(const Piece_Spec *spec, Piece_Orient o, int x, int y)
{
    int x_t;
    int y_t;
    switch (o)
    {
        case PIECE_ORIENT_DOWN:
        {
            x_t = spec->cols - 1 - x;
            y_t = spec->rows - 1 - y;
            if (x_t < 0) return 0;
            if (y_t < 0) return 0;
        } break;

        case PIECE_ORIENT_LEFT:
        {
            x_t = spec->cols - 1 - y;
            y_t = x;
            if (x_t < 0) return 0;
        } break;

        case PIECE_ORIENT_RIGHT:
        {
            x_t = y;
            y_t = spec->rows - 1 - x;
            if (y_t < 0) return 0;
        } break;

        default:
        {
            x_t = x;
            y_t = y;
        } break;
    }

    //printf("o = %d; x = %d; x_t = %d; y = %d, y_t = %d\n", o, x, x_t, y, y_t);

    //if (x_t >= spec->cols || y_t >= spec->rows) return 0;

    return spec->blocks[y_t * PIECE_MAX_COLS + x_t];
}

static const Piece_Spec Piece_Spec_T = (Piece_Spec){
    .cols = 3,
    .rows = 2,
    .color = (Col_3f){0.6f, 0.1f, 0.6f},
    .kind = PIECE_T,
    .blocks = {
        0, 1, 0, 0,
        1, 1, 1, 0,
        0, 0 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_L = (Piece_Spec){
    .cols = 2,
    .rows = 3,
    .color = (Col_3f){0.7f, 0.4f, 0.1f},
    .kind = PIECE_L,
    .blocks = {
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 1 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_S = (Piece_Spec){
    .cols = 2,
    .rows = 3,
    .color = (Col_3f){0.25f, 0.75f, 0.2f},
    .kind = PIECE_S,
    .blocks = {
        1, 0, 0, 0,
        1, 1, 0, 0,
        0, 1 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_O = (Piece_Spec){
    .cols = 2,
    .rows = 2,
    .color = (Col_3f){0.75f, 0.75f, 0.1f},
    .kind = PIECE_O,
    .blocks = {
        1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_I = (Piece_Spec){
    .cols = 4,
    .rows = 1,
    .color = (Col_3f){0.4f, 0.4f, 0.7f},
    .kind = PIECE_I,
    .blocks = {
        1, 1, 1, 1,
        0, 0, 0, 0,
        0, 0 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_J = (Piece_Spec){
    .cols = 2,
    .rows = 3,
    .color = (Col_3f){0.15f, 0.15f, 0.6f},
    .kind = PIECE_J,
    .blocks = {
        0, 1, 0, 0,
        0, 1, 0, 0,
        1, 1 ,0, 0,
        0, 0 ,0, 0,
    },
};

static const Piece_Spec Piece_Spec_Z = (Piece_Spec){
    .cols = 2,
    .rows = 3,
    .color = (Col_3f){0.6f, 0.15f, 0.15f},
    .kind = PIECE_Z,
    .blocks = {
        0, 1, 0, 0,
        1, 1, 0, 0,
        1, 0 ,0, 0,
        0, 0 ,0, 0,
    },
};

static inline const Piece_Spec *piece_spec_get_by_kind(Piece_Kind kind)
{
    switch (kind)
    {
        case PIECE_T: return &Piece_Spec_T;
        case PIECE_L: return &Piece_Spec_L;
        case PIECE_S: return &Piece_Spec_S;
        case PIECE_O: return &Piece_Spec_O;
        case PIECE_I: return &Piece_Spec_I;
        case PIECE_J: return &Piece_Spec_J;
        case PIECE_Z: return &Piece_Spec_Z;
        default: return &Piece_Spec_T;
    }
}


