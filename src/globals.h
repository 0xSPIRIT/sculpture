#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define GUI_H 96

enum {
    TOOL_CHISEL_SMALL,
    TOOL_CHISEL_MEDIUM,
    TOOL_CHISEL_LARGE,
    TOOL_KNIFE,
    TOOL_POINT_KNIFE,
    TOOL_DRILL,
    TOOL_PLACER,
    TOOL_GRABBER,
    TOOL_COUNT
};

enum {
    CELL_NONE,
    CELL_MARBLE,
    CELL_COBBLESTONE,
    CELL_QUARTZ,
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
    CELL_MAX
};

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern SDL_Texture *render_tex;

extern int current_tool;
extern int debug_mode;

extern TTF_Font *font, *title_font;

extern SDL_Cursor *grabber_cursor, *normal_cursor;

extern int mx, my; // Fake in game coordinates (pixel art scaled)
extern int real_mx, real_my; // In real window coordinates
extern int pmx, pmy;
extern Uint32 mouse;

extern int S;

extern int window_width, window_height;

extern float delta_time;

extern Uint8 *keys;

#endif  /* GLOBALS_H_ */
