#include <windows.h>

#include "win32_loader.h"
#include "../game.h"
#include "../assets.h"

#define debug_print() (printf("%s : %s:%d\n", __func__, __FILE__, __LINE__), fflush(stdout))

int main(int argc, char **argv) {
    struct Game_State *game_state = calloc(1, sizeof(struct Game_State));
    
    struct Loader *loader = win32_load_game_code();
    if (!loader) return 1;

    game_state->memory = make_memory(Megabytes(256)); // 1024kb

    game_state->S = 6;
    game_state->window_width = 128*game_state->S;
    game_state->window_height = 128*game_state->S + GUI_H;

    debug_print();

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    debug_print();
    game_state->window = SDL_CreateWindow("Alaska",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          game_state->window_width,
                                          game_state->window_height,
                                          0);

    debug_print();
    // This function takes ~0.25 seconds.
    game_state->renderer = SDL_CreateRenderer(game_state->window,
                                              -1,
                                              SDL_RENDERER_PRESENTVSYNC);

    debug_print();
    textures_init(game_state->renderer, 128, 128, 6, &game_state->textures);

    game_state->font = TTF_OpenFont("../res/cour.ttf", 19);
    game_state->font_consolas = TTF_OpenFont("../res/consola.ttf", 24);
    game_state->font_courier = TTF_OpenFont("../res/cour.ttf", 20);
    game_state->small_font = TTF_OpenFont("../res/cour.ttf", 16);
    game_state->bold_small_font = TTF_OpenFont("../res/courbd.ttf", 16);
    game_state->title_font = TTF_OpenFont("../res/cour.ttf", 45);
    
    debug_print();

    loader->game_init(game_state);
    debug_print();

    /* int frame_counter = 3*60; */
    while (1) {
        /* if (frame_counter == 0) { */
        /*     win32_free_game_code(loader); */
        /*     loader = win32_load_game_code(); */
        /*     frame_counter = 5*60; */
        /* } else { */
        /*     frame_counter--; */
        /* } */

        if (!loader->game_run(game_state)) break;
    }

    loader->game_deinit(game_state);

    win32_free_game_code(loader);

    free(game_state->memory.data);
    free(game_state);
    
    return 0;
}
