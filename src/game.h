#ifndef GAME_H_
#define GAME_H_

//
// To allocate permanent memory that will persist until
// the end of the session, use persist_alloc.
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use transient_alloc.
//

#include "shared.h"

__declspec(dllexport) void game_init(struct Game_State *state, int level);
__declspec(dllexport) bool game_tick_event(struct Game_State *state, SDL_Event *event);
__declspec(dllexport) void game_run(struct Game_State *state);
__declspec(dllexport) void game_deinit(struct Game_State *state);

#endif // GAME_H_
