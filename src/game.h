#ifndef GAME_H_
#define GAME_H_

//
// To allocate permanent memory that will persist until
// the end of the session, use persist_alloc.
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use temp_alloc.
//

#include "shared.h"

// Put all this shit into the platform/sdl layer.

__declspec(dllexport) bool game_run(struct Game_State *state);

#endif // GAME_H_
