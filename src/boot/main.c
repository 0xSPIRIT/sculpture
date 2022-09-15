#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <windows.h>
#include <stdbool.h>

// Include all files to compile in one translation unit for
// compilation speed's sake.
#include "assets.c"
#include "cursor.c"
#include "input.c"

#define _DO_NOT_EXPORT
#include "../game.h"
#include "win32_SetProcessDpiAware.h"

#define GAME_DLL_NAME "sculpture.dll"
#define TEMP_DLL_NAME "sculpture_temp.dll"

typedef void (*GameInitProc)(struct Game_State *state);
typedef bool (*GameTickEventProc)(struct Game_State *state, SDL_Event *event);
typedef void (*GameRunProc)(struct Game_State *state);
typedef void (*GameDeinitProc)(struct Game_State *state);

struct Game_Code {
    HMODULE dll;
    FILETIME last_write_time;
    
    GameInitProc game_init;
    GameTickEventProc game_tick_event;
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
    Assert(state->window, state->window);
    
    /* SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11"); */
    state->renderer = SDL_CreateRenderer(state->window,
                                         -1,
                                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    Assert(state->window, state->renderer);
}

static struct Memory make_memory(size_t size) {
    struct Memory memory = {0};
    
    LPVOID base_address = (LPVOID) Terabytes(2);
    
    memory.data = VirtualAlloc(base_address, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    AssertNW(memory.data);
    
    memory.cursor = memory.data;
    memory.size = size;
    return memory;
}

static void game_init(struct Game_State *state) {
    srand(time(0));
    
    state->S = 6;
    state->window_width = 128*state->S;
    state->window_height = 128*state->S + GUI_H;

    // Taken from https://github.com/kumar8600/win32_SetProcessDpiAware
    win32_SetProcessDpiAware();

    game_init_sdl(state, "Alaska", state->window_width, state->window_height);
    
    // Load all assets.
    textures_init(state->renderer,
                  state->window,
                  state->window_width,
                  64,
                  64,
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
    code->game_tick_event = (GameTickEventProc) GetProcAddress(code->dll, "game_tick_event");
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
    code->game_tick_event = 0;
    code->dll = 0;
}

int main(int argc, char **argv) {
    // Make sure we're running in the right folder.
    {
        char cwd[1024] = {0};
        size_t length = 0;
        char *final_three_chars = NULL;
    
        GetCurrentDirectory(1024, cwd);
        length = strlen(cwd);

        final_three_chars = cwd+length-3;

        AssertNW(0 == strcmp(final_three_chars, "bin"));
    }

    struct Game_Code game_code;
    load_game_code(&game_code);
    
    struct Game_State game_state = {0};
    game_state.memory = make_memory(Megabytes(512));
    
    game_init(&game_state);
    game_code.game_init(&game_state);

    const int reload_delay_max = 5;
    int reload_delay = reload_delay_max; // Frames of delay.

    bool running = true;
    
    while (running) {
        game_state.temp_allocation_count = 0;
        
        // We push the DLL reload to a delay because if we lock the file too fast
        // the compiler doesn't have enough time to write to it... or something.
        // It's only a 5-frame delay so it doesn't matter compared to compilation
        // times anyways.
        FILETIME new_dll_write_time = get_last_write_time(GAME_DLL_NAME);
        if (CompareFileTime(&new_dll_write_time, &game_code.last_write_time) != 0) {
            reload_delay--;
        }
        
        if (reload_delay <= 0) {
            unload_game_code(&game_code);
            load_game_code(&game_code);
            reload_delay = reload_delay_max;
        }
        
        input_tick(&game_state);
        
        // Memory usage statistics.
        {
            unsigned size_current = game_state.memory.cursor - game_state.memory.data;
            unsigned size_max = game_state.memory.size;
            float percentage = (float)size_current / (float)size_max;
            percentage *= 100.f;

            char title[128] = {0};
            sprintf(title, "Alaska | Memory Used: %.2f%%", percentage);

            SDL_SetWindowTitle(game_state.window, title);
        }

        SDL_Event event;

        if (SDL_PollEvent(&event)) {
            running = game_code.game_tick_event(&game_state, &event);
        }

        game_code.game_run(&game_state);

        // Ensure our allocations are not leaked!
        Assert(game_state.window, game_state.temp_allocation_count == 0);
    }
    
    game_code.game_deinit(&game_state);
    game_deinit(&game_state);
    
    VirtualFree(game_state.memory.data, 0, MEM_RELEASE);
    
    return 0;
}
