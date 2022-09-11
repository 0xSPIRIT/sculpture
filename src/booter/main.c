#include <windows.h>

#include "win32_loader.h"
#include "../game.h"

int main(int argc, char **argv) {
    struct Game_State *game_state = calloc(1, sizeof(struct Game_State));
    
    struct Loader *loader = win32_load_game_code();
    if (!loader) return 1;

    loader->game_init(game_state);

    int frame_counter = 3*60;
    while (1) {
        if (frame_counter == 0) {
            win32_free_game_code(loader);
            loader = win32_load_game_code();
            frame_counter = 5*60;
        } else {
            frame_counter--;
        }

        if (!loader->game_run(game_state)) break;
    }

    loader->game_deinit(game_state);

    win32_free_game_code(loader);

    free(game_state);
    
    return 0;
}
