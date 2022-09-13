#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <windows.h>
#include <stdbool.h>

#include "../shared.h"

typedef void (*GameInitProc)(struct Game_State *state);
typedef bool (*GameRunProc)(struct Game_State *state);
typedef void (*GameDeinitProc)(struct Game_State *state);

struct Game_Code {
    HMODULE dll;
    GameInitProc game_init;
    GameRunProc game_run;
    GameDeinitProc game_deinit;
};

void load_game_code(struct Game_Code *code) {
    CopyFileA("sculpture.dll", "sculpture_temp.dll", FALSE);
    printf("%d\n", GetLastError()); fflush(stdout);
    code->dll = LoadLibraryA("sculpture_temp.dll");
    if (!code->dll) {
        fprintf(stderr, "Error loading the DLL!\n");
        exit(1);
    }
    code->game_init = (GameInitProc) GetProcAddress(code->dll, "game_init");
    code->game_run = (GameRunProc) GetProcAddress(code->dll, "game_run");
    code->game_deinit = (GameDeinitProc) GetProcAddress(code->dll, "game_deinit");
    if (!code->game_init || !code->game_run || !code->game_deinit) {
        fprintf(stderr, "Error finding the functions in the DLL!\n");
        exit(1);
    }
}

void unload_game_code(struct Game_Code *code) {
    FreeLibrary(code->dll);
    code->game_init = 0;
    code->game_run = 0;
    code->game_deinit = 0;
}

int main(int argc, char **argv) {
    struct Game_Code game_code;
    load_game_code(&game_code);
    // debug();
    
    struct Game_State game_state = {0};
    game_state.memory = make_memory(Megabytes(256));

    // debug();
    game_code.game_init(&game_state);

    // debug();

    int timer = 0;
    while (1) {
        // debug();
        timer++;
        if (timer >= 3*60) {
            unload_game_code(&game_code);
            load_game_code(&game_code);
            printf("Reloaded game code!\n");
        }
        if (!game_code.game_run(&game_state)) break;
        // debug();
    }

    game_code.game_deinit(&game_state);

    free(game_state.memory.data);

    return 0;
}
