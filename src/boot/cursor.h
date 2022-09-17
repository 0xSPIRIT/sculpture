#ifndef CURSOR_H_
#define CURSOR_H_

#include <SDL2/SDL.h>

extern const char *arrow_cursor_data[];
extern const char *placer_cursor_data[];

SDL_Cursor *init_system_cursor(const char **image);

#endif  /* CURSOR_H_ */
