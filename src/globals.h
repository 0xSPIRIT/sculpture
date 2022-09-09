#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "input.h"

#define GUI_H 96

enum Tool_Type {
    TOOL_CHISEL_SMALL,
    TOOL_CHISEL_MEDIUM,
    TOOL_CHISEL_LARGE,
    TOOL_KNIFE,
    TOOL_POINT_KNIFE,
    TOOL_HAMMER,
    TOOL_PLACER,
    TOOL_GRABBER,
    TOOL_COUNT
};

enum Cell_Type {
    CELL_NONE,
    CELL_MARBLE,
    CELL_COBBLESTONE,
    CELL_QUARTZ,
    CELL_GRANITE,
    CELL_BASALT,
    CELL_WOOD_LOG,
    CELL_WOOD_PLANK,
    CELL_DIRT,
    CELL_SAND,
    CELL_GLASS,
    CELL_WATER,
    CELL_COAL,
    CELL_STEAM,
    CELL_DIAMOND,
    CELL_ICE,
    CELL_LEAF,
    CELL_SMOKE,
    CELL_DUST,
    CELL_LAVA,
    CELL_COUNT
};

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern SDL_Texture *render_tex;

extern SDL_Texture *item_textures[CELL_COUNT];

extern int current_tool;
extern int debug_mode;

extern TTF_Font *font, *font_consolas, *small_font, *title_font;

extern SDL_Cursor *grabber_cursor, *normal_cursor, *placer_cursor;

extern int S;

extern int window_width, window_height;

extern float delta_time;

extern bool running;

#endif  /* GLOBALS_H_ */
