#ifndef GAME_H_
#define GAME_H_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

void game_init(void);
void game_deinit();
void game_run(void);
void game_tick_event(SDL_Event *event);

void fonts_init(void);
void fonts_deinit(void);

#endif // GAME_H_
