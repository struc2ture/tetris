#pragma once

#include <stdbool.h>
#include <stdio.h>

#include <OpenGL/gl3.h>
#include <stb_image.h>

typedef struct {
    GLuint texture_id;
    int w, h, ch;
    GLenum internal_format, format;
} Texture;

static bool gl_check_compile_success(GLuint shader, const char *src)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);

        fprintf(stderr, "Shader compile error:\n%s\n\nSource:\n", log);
        fprintf(stderr, "%s\n", src);
    }
    return (bool)success;
}

bool gl_check_link_success(GLuint prog)
{
    GLint success = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);

        fprintf(stderr, "Program link error:\n%s\n", log);
    }
    return (bool)success;
}

static GLuint gl_create_shader_program(const char *vs_src, const char *fs_src)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, 0);
    glCompileShader(vs);
    gl_check_compile_success(vs, vs_src);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, 0);
    glCompileShader(fs);
    gl_check_compile_success(fs, fs_src);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    gl_check_link_success(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

static Texture gl_load_texture(const char *path, GLint sampling_type)
{
    Texture tex;
    unsigned char *tex_data = stbi_load(path, &tex.w, &tex.h, &tex.ch, 0);

    if (tex.ch == 4) { tex.internal_format = GL_RGBA8; tex.format = GL_RGBA; }
    else if (tex.ch == 3) { tex.internal_format = GL_RGB8; tex.format = GL_RGB; }
    else if (tex.ch == 2) { tex.internal_format = GL_RG8; tex.format = GL_RG; }
    else if (tex.ch == 1) { tex.internal_format = GL_R8; tex.format = GL_RED; }

    glGenTextures(1, &tex.texture_id);
    glBindTexture(GL_TEXTURE_2D, tex.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, tex.internal_format, tex.w, tex.h, 0, tex.format, GL_UNSIGNED_BYTE, tex_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling_type);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling_type);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(tex_data);

    return tex;
}

static void gl_delete_texture(Texture *tex)
{
    glDeleteTextures(1, &tex->texture_id);
}

typedef struct {
    float r, g, b;
} Col_3f;

typedef struct {
    float x, y;
    Col_3f color;
} Vert;

#define VERT_MAX 4096
#define INDEX_MAX 8192
typedef struct {
    Vert verts[VERT_MAX];
    int vert_count;

    unsigned int indices[INDEX_MAX];
    int index_count;

    GLuint vao, vbo, ebo;
} Vert_Buffer;

static inline size_t vert_buffer_vert_size(const Vert_Buffer *vb)
{
    return sizeof(Vert) * vb->vert_count;
}

static inline size_t vert_buffer_max_vert_size()
{
    return sizeof(Vert) * VERT_MAX;
}

static inline size_t vert_buffer_index_size(const Vert_Buffer *vb)
{
    return sizeof(unsigned int) * vb->index_count;
}

static inline size_t vert_buffer_max_index_size()
{
    return sizeof(unsigned int) * INDEX_MAX;
}

static inline Vert_Buffer *vert_buffer_make()
{
    Vert_Buffer *vb = malloc(sizeof(Vert_Buffer));
    vb->vert_count = 0;
    vb->index_count = 0;

    glGenVertexArrays(1, &vb->vao);
    glGenBuffers(1, &vb->vbo);
    glGenBuffers(1, &vb->ebo);

    glBindVertexArray(vb->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vb->vbo);
    glBufferData(GL_ARRAY_BUFFER, vert_buffer_max_vert_size(), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vert_buffer_max_index_size(), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void *)(offsetof(Vert, color)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return vb;
}

static inline void vert_buffer_clear(Vert_Buffer *vb)
{
    vb->vert_count = 0;
    vb->index_count = 0;
}

static inline void vert_buffer_free(Vert_Buffer *vb)
{
    glDeleteBuffers(1, &vb->vbo);
    glDeleteBuffers(1, &vb->ebo);
    glDeleteVertexArrays(1, &vb->vao);
    free(vb);
}

static inline void vert_buffer_add_vert(Vert_Buffer *vert_buffer, Vert vert)
{
    if (vert_buffer->vert_count < VERT_MAX)
    {
        vert_buffer->verts[vert_buffer->vert_count++] = vert;
    }
}

static inline int vert_buffer_next_vert_index(const Vert_Buffer *vb)
{
    return vb->vert_count;
}

static inline void vert_buffer_add_indices(Vert_Buffer *vb, int base, int *indices, int index_count)
{
    for (int i = 0; i < index_count; i++)
    {
        if (vb->index_count < INDEX_MAX)
        {
            vb->indices[vb->index_count++] = base + indices[i];
        }
    }
}

static inline void vert_buffer_draw_call(const Vert_Buffer *vb)
{
    glBindVertexArray(vb->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vb->vbo);
    glBufferData(GL_ARRAY_BUFFER, vert_buffer_max_vert_size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_buffer_vert_size(vb), vb->verts);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vert_buffer_max_index_size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, vert_buffer_index_size(vb), vb->indices);

    glDrawElements(GL_TRIANGLES, vb->index_count, GL_UNSIGNED_INT, 0);
}

