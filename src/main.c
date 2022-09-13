#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "game.h"

int main(int argc, char **argv) {
    struct Game_State *game_state = calloc(1, sizeof(struct Game_State));
    game_state->memory = make_memory(Megabytes(256));
    
    game_init(game_state);

    while (1) {
        if (!game_run(game_state)) break;
    }

    game_deinit(game_state);

    free(game_state->memory.data);
    free(game_state);

    return 0;
}
