#ifndef LOADER_H
#define LOADER_H 

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

typedef void (*game_init_proc)(struct Game_State *);
typedef bool (*game_run_proc)(struct Game_State *);
typedef void (*game_deinit_proc)(void);

struct Loader {
    HMODULE game_code_dll;

    game_init_proc game_init;
    game_run_proc game_run;
    game_deinit_proc game_deinit;
};

struct Loader *win32_load_game_code() {
    struct Loader *loader;

    loader = malloc(sizeof(struct Loader));

    CopyFileA("../bin/sculpture.dll", "../bin/sculpture_temp.dll", false);

    loader->game_code_dll = LoadLibraryA("../bin/sculpture_temp.dll");
    if (!loader->game_code_dll)
        return NULL;
    
    loader->game_init = (game_init_proc) GetProcAddress(loader->game_code_dll, "game_init");
    loader->game_run = (game_run_proc) GetProcAddress(loader->game_code_dll, "game_run");
    loader->game_deinit = (game_deinit_proc) GetProcAddress(loader->game_code_dll, "game_deinit");

    printf("Loaded sculpture.dll successfully!\n"); fflush(stdout);

    return loader;
}

void win32_free_game_code(struct Loader *loader) {
    loader->game_init = 0;
    loader->game_deinit = 0;
    loader->game_run = 0;

    FreeLibrary(loader->game_code_dll);
    free(loader);
}

#endif // LOADER_H
