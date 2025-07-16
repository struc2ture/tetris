#pragma once

#include "common.h"
#include "gl_glue.h"

void vb_add_rect(Vert_Buffer *vb, Rect rect, Col_3f color)
{
    int index_base = vert_buffer_next_vert_index(vb);
    float x_min = rect.x;
    float x_max = rect.x + rect.w;
    float y_min = rect.y;
    float y_max = rect.y + rect.h;

    vert_buffer_add_vert(vb, (Vert){x_min, y_min, color});
    vert_buffer_add_vert(vb, (Vert){x_max, y_min, color});
    vert_buffer_add_vert(vb, (Vert){x_max, y_max, color});
    vert_buffer_add_vert(vb, (Vert){x_min, y_max, color});

    int indices[] = {0, 3, 1, 1, 3, 2};
    vert_buffer_add_indices(vb, index_base, indices, 6);
}

Block *get_block_at(Game_State *s, int x, int y)
{
    if (x < 0 || x >= s->tetris_cols ||
        y < 0 || y >= s->tetris_rows)
    {
        return NULL;
    }

    return &s->blocks[s->tetris_cols * y + x];
}

