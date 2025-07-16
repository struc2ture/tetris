#include <stdlib.h>

#include "tetris.h"
#include "common.h"
#include "gl_glue.h"
#include "lib.h"
#include "pieces.h"

void create_shaders(Game_State *s)
{
    if (s->prog) glDeleteProgram(s->prog);

    const char *vs_src =
        "#version 330 core\n"
        "layout(location = 0) in vec2 aPos;\n"
        "layout(location = 1) in vec3 aColor;\n"
        "out vec3 Color;\n"
        "uniform mat4 u_mvp;\n"
        "void main() {"
        "  gl_Position = u_mvp * vec4(aPos, 0.0, 1.0);\n"
        "  Color = aColor;\n"
        "}\n";

    const char *fs_src =
        "#version 330 core\n"
        "in vec3 Color;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "  FragColor = vec4(Color, 1.0);\n"
        "}\n";

    s->prog = gl_create_shader_program(vs_src, fs_src);
}

void create_vert_buffer(Game_State *s)
{
    if (s->vb) vert_buffer_free(s->vb);
    s->vb = vert_buffer_make();
}

// --------------------------------------------------------------------

static const float tile_dim = 24.0f;

static const float content_w = tile_dim * TETRIS_COLS;
static const float content_h = tile_dim * TETRIS_ROWS;
static const float content_padding = 4.0f;

static const float outer_rect_padding = 4.0f;

static const float content_x = outer_rect_padding + content_padding;
static const float content_y = outer_rect_padding + content_padding;

static const float block_padding = 2.0f;

void draw_canvas_bg(Game_State *s)
{
    const Rect inner_rect = {
        .x = outer_rect_padding,
        .y = outer_rect_padding,
        .w = content_w + 2 * content_padding,
        .h = content_h + 2 * content_padding
    };
    const Col_3f inner_color = {0.15f,0.15f,0.16f};

    const Rect outer_rect = {
        .x = 0, .y = 0,
        .w = inner_rect.w + 2 * outer_rect_padding,
        .h = inner_rect.h + 2 * outer_rect_padding,
    };
    const Col_3f outer_color = {0.1f,0.1f,0.11f};

    vb_add_rect(s->vb, outer_rect, outer_color);
    vb_add_rect(s->vb, inner_rect, inner_color);
}

void draw_block(Game_State *s, float x, float y, Col_3f col)
{
    const Rect outer_rect = {
        .x = x, .y = y,
        .w = tile_dim, .h = tile_dim
    };
    const Col_3f outer_color = {col.r * 0.8f, col.g * 0.8f, col.b * 0.8f};

    const Rect inner_rect = {
        .x = x + block_padding,
        .y = y + block_padding,
        .w = outer_rect.w - 2 * block_padding,
        .h = outer_rect.h - 2 * block_padding
    };
    const Col_3f inner_color = col;

    vb_add_rect(s->vb, outer_rect, outer_color);
    vb_add_rect(s->vb, inner_rect, inner_color);
}

void draw_current_piece(Game_State *s)
{
    const Piece *piece = &s->current_piece;
    const Piece_Spec *spec = piece_spec_get_by_kind(piece->kind);

    for (int col = 0; col < PIECE_MAX_COLS; col++)
    {
        for (int row = 0; row < PIECE_MAX_ROWS; row++)
        {
            if (piece_spec_get_block_state_at(spec, piece->orient, col, row))
            {
                int block_x = piece->x + col;
                int block_y = piece->y + row;
                float x = content_x + block_x * tile_dim;
                float y = content_y + block_y * tile_dim;
                draw_block(s, x, y, spec->color);
            }
        }
    }
}

void draw(Game_State *s)
{
    vert_buffer_clear(s->vb);

    draw_canvas_bg(s);
    for (int row = 0; row < s->tetris_rows; row++)
    {
        for (int col = 0; col < s->tetris_cols; col++)
        {
            Block *b = get_block_at(s, col, row);
            if (b->piece_id > 0)
            {
                Col_3f color = piece_spec_get_by_kind(b->piece_kind)->color;
                float x = content_x + col * tile_dim;
                float y = content_y + row * tile_dim;
                if (s->is_game_over) color = (Col_3f){color.r * 0.4f, color.g * 0.4f, color.b * 0.4f};
                draw_block(s, x, y, color);
            }
        }
    }

    if (!s->is_game_over) draw_current_piece(s);

    vert_buffer_draw_call(s->vb);
}

// ---------------------------------------------------------------

void commit_piece(Game_State *s, const Piece *piece)
{
    const Piece_Spec *spec = piece_spec_get_by_kind(piece->kind);

    for (int col = 0; col < PIECE_MAX_COLS; col++)
    {
        for (int row = 0; row < PIECE_MAX_ROWS; row++)
        {
            if (piece_spec_get_block_state_at(spec, piece->orient, col, row))
            {
                Block *b = get_block_at(s, piece->x + col, piece->y + row);
                b->piece_kind = piece->kind;
                b->piece_id = piece->id;
            }
        }
    }
}

bool check_piece_collision(Game_State *s, const Piece *piece, int new_x, int new_y, Piece_Orient new_orient)
{
    const Piece_Spec *spec = piece_spec_get_by_kind(piece->kind);
    for (int col = 0; col < PIECE_MAX_COLS; col++)
    {
        for (int row = 0; row < PIECE_MAX_ROWS; row++)
        {
            if (piece_spec_get_block_state_at(spec, new_orient, col, row))
            {
                Block *b = get_block_at(s, new_x + col, new_y + row);
                if (!b || b->piece_id > 0) return false;
            }
        }
    }
    return true;
}

bool set_current_piece(Game_State *s, Piece p)
{
    if (check_piece_collision(s, &p, p.x, p.y, p.orient))
    {
        s->current_piece = p;
        return true;
    }
    return false;
}

bool generate_new_piece(Game_State *s)
{
    Piece p = {
        .id = s->piece_id_seed++,
        .x = 3, .y = 0,
        .kind = (Piece_Kind)(rand() % PIECE_KIND_COUNT),
        .orient = (Piece_Orient)(rand() % PIECE_ORIENT_COUNT)
    };

    return set_current_piece(s, p);
}

bool move_current_piece_down(Game_State *s)
{
    int new_y = s->current_piece.y + 1;
    if (check_piece_collision(s, &s->current_piece, s->current_piece.x, new_y, s->current_piece.orient))
    {
        s->current_piece.y = new_y;
        return true;
    }
    else
    {
        return false;
    }
}

bool rotate_current_piece(Game_State *s)
{
    Piece_Orient new_o = s->current_piece.orient + 1;
    if (new_o >= PIECE_ORIENT_COUNT) new_o = PIECE_ORIENT_UP;
    if (check_piece_collision(s, &s->current_piece, s->current_piece.x, s->current_piece.y, new_o))
    {
        s->current_piece.orient = new_o;
        return true;
    }
    return false;
}

bool slide_current_piece(Game_State *s, int dir)
{
    int new_x = s->current_piece.x + dir;
    if (check_piece_collision(s, &s->current_piece, new_x, s->current_piece.y, s->current_piece.orient))
    {
        s->current_piece.x = new_x;
        return true;
    }
    return false;
}

int find_full_line(Game_State *s)
{
    for (int row = s->tetris_rows - 1; row >= 0; row--)
    {
        bool full_line = true;
        for (int col = 0; col < s->tetris_cols; col++)
        {
            Block *b = get_block_at(s, col, row);
            if (b->piece_id == 0)
            {
                full_line = false;
                break;
            }
        }
        if (full_line) return row;
    }
    return -1;
}

void delete_line(Game_State *s, int line)
{
    for (int row = line; row >= 0; row--)
    {
        for (int col = 0; col < s->tetris_cols; col++)
        {
            Block *this_b = get_block_at(s, col, row);
            Block *prev_b = row > 0 ? get_block_at(s, col, row - 1) : &(Block){.piece_id = 0};
            *this_b = *prev_b;
        }
    }
}

void check_lines(Game_State *s)
{
    int line = find_full_line(s);
    while (line >= 0)
    {
        delete_line(s, line);
        line = find_full_line(s);
    }
}

// -----------------------------------------------

void initialize_game(Game_State *s)
{
    if (s->blocks) free(s->blocks);
    s->blocks = calloc(1, s->tetris_cols * s->tetris_rows * sizeof(s->blocks[0]));
    s->piece_id_seed = 1;
    srand(time(NULL));
    generate_new_piece(s);
    s->move_period = MOVE_PERIOD;
    s->move_timer = 0.0f;
    s->is_game_over = false;
}
