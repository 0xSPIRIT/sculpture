#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <windows.h>
#include <stdbool.h>

#include "../game.h"

#include "cursor.h"

typedef bool (*GameRunProc)(struct Game_State *state);

struct Game_Code {
    HMODULE dll;
    GameRunProc game_run;
};


void game_init_sdl(const char *window_title, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    gs->window = SDL_CreateWindow(window_title,
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w,
                              h,
                              0);

    // This function takes ~0.25 seconds.
    gs->renderer = SDL_CreateRenderer(gs->window,
                                      -1,
                                      SDL_RENDERER_PRESENTVSYNC);

    gs->render_texture = SDL_CreateTexture(gs->renderer,
                                           SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_TARGET,
                                           gs->window_width/gs->S,
                                           gs->window_height/gs->S);
    SDL_SetRenderDrawBlendMode(gs->renderer, SDL_BLENDMODE_BLEND);
}

// Separate out the SDL alloc stuff eg textures, fonts, etc
// from just normal initializations that belong in the
// game layer. Put those into the normal game.c
void game_init(struct Game_State *state) {
    gs = state;
    
    srand(time(0));

    gs->S = 6;

    gs->window_width = 128*gs->S;
    gs->window_height = 128*gs->S + GUI_H;
     
    game_init_sdl("Alaska", gs->window_width, gs->window_height);
    
    gs->normal_cursor = SDL_GetCursor();
    gs->grabber_cursor = init_system_cursor(arrow_cursor_data);
    gs->placer_cursor = init_system_cursor(placer_cursor_data);

    textures_init(gs->renderer,
                  gs->window_width/gs->S,
                  gs->window_height/gs->S,
                  gs->S,
                  &gs->textures);

    fonts_init(gs);
    item_init();
    levels_setup();

    goto_level(0);
}

void game_deinit(struct Game_State *state) {
    gs = state;
    
    textures_deinit(&gs->textures);
    fonts_deinit(gs);
    levels_deinit();
    undo_system_deinit();
    grid_deinit();

    SDL_DestroyWindow(state->window);
    SDL_DestroyRenderer(state->renderer);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


void load_game_code(struct Game_Code *code) {
    CopyFileA("sculpture.dll", "sculpture_temp.dll", FALSE);
    printf("%d\n", GetLastError()); fflush(stdout);
    code->dll = LoadLibraryA("sculpture_temp.dll");
    if (!code->dll) {
        fprintf(stderr, "Error loading the DLL!\n");
        exit(1);
    }
    code->game_run = (GameRunProc) GetProcAddress(code->dll, "game_run");
    if (!code->game_run) {
        fprintf(stderr, "Error finding the functions in the DLL!\n");
        exit(1);
    }
}

void unload_game_code(struct Game_Code *code) {
    FreeLibrary(code->dll);
    code->game_run = 0;
}

int main(int argc, char **argv) {
    struct Game_Code game_code;
    load_game_code(&game_code);
    
    struct Game_State game_state = {0};
    game_state.memory = make_memory(Megabytes(256));

    game_init(&game_state);

    int timer = 0;
    while (1) {
        timer++;
        if (timer >= 3*60) {
            unload_game_code(&game_code);
            load_game_code(&game_code);
            printf("Reloaded game code!\n");
        }
        if (!game_code.game_run(&game_state)) break;
    }

    game_deinit(&game_state);

    free(game_state.memory.data);

    return 0;
}
