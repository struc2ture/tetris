#include <e.h>

void on_run(Editor_State *s)
{
    const char *src_path = "src/main.c";
    const char *out_path = "bin/tetris.dylib";

    const char *cc = "clang";
    const char *cflags = "-I/opt/homebrew/include -Ithird_party -DGL_SILENCE_DEPRECATION -Wall -Werror -Wno-unused-function -Wno-unused-variable";
    const char *lflags = "-L/opt/homebrew/lib -lglfw -framework OpenGL";
    char *compile_command = strf("%s -g -dynamiclib %s %s %s -o %s", cc, cflags, lflags, src_path, out_path);

    printf("\nCompilation:\n%s\n\n", compile_command);
    int result = system(compile_command);
    printf("\nCompilation finished. Status: %d\n\n", result);

    free(compile_command);
}
