#include "game.h"

#include "util.h"

int main(int argc, char **argv) {
    game_init();
    game_run();
    game_deinit();
    return 0;
}
