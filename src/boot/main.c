#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <windows.h>
#include <stdbool.h>

#include "../game.h"
#include "assets.h"

#include "cursor.h"

#define GAME_DLL_NAME "sculpture.dll"
#define TEMP_DLL_NAME "sculpture_temp.dll"

typedef bool (*GameInitProc)(struct Game_State *state);
typedef bool (*GameRunProc)(struct Game_State *state);
typedef bool (*GameDeinitProc)(struct Game_State *state);

struct Game_Code {
    HMODULE dll;
    FILETIME last_write_time;

    GameInitProc game_init;
    GameRunProc game_run;
    GameDeinitProc game_deinit;
};

static void game_init_sdl(struct Game_State *state, const char *window_title, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    state->window = SDL_CreateWindow(window_title,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     w,
                                     h,
                                     0);
    SDL_assert(state->window);

    /* SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11"); */
    state->renderer = SDL_CreateRenderer(state->window,
                                         -1,
                                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_assert(state->renderer);
}

static struct Memory make_memory(size_t size) {
    struct Memory memory = {0};

    LPVOID base_address = (LPVOID) Terabytes(2);
    
    memory.data = VirtualAlloc(base_address, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    SDL_assert(memory.data);

    memory.cursor = memory.data;
    memory.size = size;
    return memory;
}

static void game_init(struct Game_State *state) {
    srand(time(0));

    state->S = 6;
    state->window_width = 128*state->S;
    state->window_height = 128*state->S + GUI_H;
     
    game_init_sdl(state, "Alaska", state->window_width, state->window_height);

    // Load assets.
    textures_init(state->renderer,
                  state->window_width/state->S,
                  state->window_height/state->S,
                  state->S,
                  &state->textures);

    surfaces_init(&state->surfaces);

    state->render_texture = state->textures.render_texture;

    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
    
    state->normal_cursor = SDL_GetCursor();
    state->grabber_cursor = init_system_cursor(arrow_cursor_data);
    state->placer_cursor = init_system_cursor(placer_cursor_data);

    fonts_init(state);
}

static void game_deinit(struct Game_State *state) {
    textures_deinit(&state->textures);
    surfaces_deinit(&state->surfaces);
    fonts_deinit(state);

    SDL_DestroyWindow(state->window);
    SDL_DestroyRenderer(state->renderer);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

inline FILETIME get_last_write_time(char *filename) {
    FILETIME write_time = {0};

    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filename, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE) {
        write_time = find_data.ftLastWriteTime;
        FindClose(find_handle);
    }

    return write_time;
}

static void load_game_code(struct Game_Code *code) {
    code->last_write_time = get_last_write_time(GAME_DLL_NAME);

    CopyFileA(GAME_DLL_NAME, TEMP_DLL_NAME, FALSE);
    code->dll = LoadLibraryA(TEMP_DLL_NAME);
    if (!code->dll) {
        fprintf(stderr, "Error loading the DLL!\n");
        exit(1);
    }
    code->game_init = (GameInitProc) GetProcAddress(code->dll, "game_init");
    code->game_run = (GameRunProc) GetProcAddress(code->dll, "game_run");
    code->game_deinit = (GameDeinitProc) GetProcAddress(code->dll, "game_deinit");
    if (!code->game_run) {
        fprintf(stderr, "Error finding the functions in the DLL!\n");
        exit(1);
    }
}

static void unload_game_code(struct Game_Code *code) {
    FreeLibrary(code->dll);
    code->game_init = 0;
    code->game_run = 0;
    code->game_deinit = 0;
}

int main(int argc, char **argv) {
    struct Game_Code game_code;
    load_game_code(&game_code);
    
    struct Game_State game_state = {0};
    game_state.memory = make_memory(Megabytes(256));

    game_init(&game_state);
    game_code.game_init(&game_state);

    const int timer_max = 5;
    int timer = timer_max; // Frames of delay.

    while (1) {
        FILETIME new_dll_write_time = get_last_write_time(GAME_DLL_NAME);
        if (CompareFileTime(&new_dll_write_time, &game_code.last_write_time) != 0) {
            timer--;
        }

        if (timer <= 0) {
            unload_game_code(&game_code);
            load_game_code(&game_code);
            printf("Reloaded game code!\n");
            timer = timer_max;
        }

        input_tick(&game_state);

        unsigned size_current = game_state.memory.cursor - game_state.memory.data;
        unsigned size_max = game_state.memory.size;
        float percentage = (float)size_current / (float)size_max;
        percentage *= 100.f;

        if (!game_code.game_run(&game_state)) {
            goto end;
        }
    }
 end:
    game_code.game_deinit(&game_state);
    game_deinit(&game_state);

    VirtualFree(game_state.memory.data, 0, MEM_RELEASE);

    return 0;
}
