#ifndef GAME_H_
#define GAME_H_

#if defined(_DO_NOT_EXPORT)
#define DllExport
#else
#define DllExport __declspec(dllexport)
#endif

#include <SDL2/SDL.h>
#include <stdbool.h>

DllExport void game_init(struct Game_State *state);
DllExport bool game_tick_event(struct Game_State *state, SDL_Event *event);
DllExport void game_run(struct Game_State *state);
DllExport void game_deinit(struct Game_State *state);

#endif // GAME_H_
