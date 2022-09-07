#include "cursor.h"

#include <stdio.h>

// Taken from the SDL wiki.

static const char *arrow[] = {
    /* width height num_colors chars_per_pixel */
    "    32    32        3            1",
    /* colors */
    "X c #000000",
    ". c #ff0000",
    "  c None",
    /* pixels */
    "X                               ",
    "XX                              ",
    "X.X                             ",
    "X..X                            ",
    "X...X                           ",
    "X....X                          ",
    "X.....X                         ",
    "X......X                        ",
    "X.......X                       ",
    "X........X                      ",
    "X.........X                     ",
    "X..........X                    ",
    "X...........X                   ",
    "X............X                  ",
    "X.............X                 ",
    "X..............X                ",
    "X.......XXXXXXXXX               ",
    "X......X                        ",
    "X.....X                         ",
    "X....X                          ",
    "X...X                           ",
    "X..X                            ",
    "X.X                             ",
    "XX                              ",
    "X                               ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "0,0"
};

SDL_Cursor *init_system_cursor() {
    const char **image = arrow;
        
    int i, row, col;
    Uint8 data[4*32];
    Uint8 mask[4*32];
    int hot_x, hot_y;

    i = -1;
    for (row=0; row<32; ++row) {
        for (col=0; col<32; ++col) {
            if (col % 8) {
                data[i] <<= 1;
                mask[i] <<= 1;
            } else {
                ++i;
                data[i] = mask[i] = 0;
            }
            switch (image[4+row][col]) {
            case '.':
                data[i] |= 0x01;
                mask[i] |= 0x01;
                break;
            case 'X':
                mask[i] |= 0x01;
                break;
            case ' ':
                break;
            }
        }
    }
    sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
    return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void _set_cursor(SDL_Cursor *cursor, const char *file, int line) {
    /* printf("Set to %p at file %s and line %d.\n", (void*)cursor, file, line); fflush(stdout); */
    SDL_SetCursor(cursor);
}
