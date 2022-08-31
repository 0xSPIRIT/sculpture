#ifndef CURSOR_H_
#define CURSOR_H_

#include <SDL2/SDL.h>

#define set_cursor(cursor) (_set_cursor(cursor, __FILE__, __LINE__))

SDL_Cursor *init_system_cursor();
void _set_cursor(SDL_Cursor *cursor, const char *file, int line);

#endif  /* CURSOR_H_ */
