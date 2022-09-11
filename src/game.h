#ifndef GAME_H_
#define GAME_H_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdbool.h>

#include "blob_hammer.h"
#include "grid.h"
#include "chisel.h"
#include "knife.h"
#include "point_knife.h"
#include "popup.h"
#include "placer.h"
#include "level.h"
#include "gui.h"
#include "input.h"
#include "grabber.h"
#include "effects.h"
#include "chisel_blocker.h"

struct Game_State {
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;

    int S; // scale
    int window_width, window_height;
    float delta_time;

    SDL_Texture *render_texture;
    SDL_Texture *item_textures[CELL_COUNT];

    int current_tool;
    int debug_mode;
    
    TTF_Font *font,
        *font_consolas,
        *font_courier,
        *small_font,
        *bold_small_font,
        *title_font;

    SDL_Cursor *grabber_cursor, *normal_cursor, *placer_cursor;

    struct Cell *grid_layers[NUM_GRID_LAYERS]; 
    struct Cell *grid, *fg_grid, *gas_grid, *pickup_grid;
    int gw, gh;
    int grid_show_ghost;

    struct Object objects[MAX_OBJECTS];
    int object_count, object_current;
    int do_draw_blobs, do_draw_objects;

    bool paused;
    int frames;
    bool step_one;
    
    struct Blob_Hammer blob_hammer;
    struct Knife knife;
    struct Point_Knife point_knife;
    clock_t global_start, global_end;

    struct Save_State *current_state, *start_state;

    struct Text_Field text_field;

    struct Placer placers[PLACER_COUNT];
    int current_placer;

    struct Level levels[MAX_LEVELS];
    int level_current, level_count, new_level;

    struct GUI gui;
    SDL_Texture *gui_texture;

    struct Input input;

    struct Grabber grabber;
    struct Effect current_effect;

    struct Chisel_Blocker chisel_blocker;
    int chisel_blocker_mode; // Edit mode for the points.

    struct Converter *material_converter, *fuel_converter;

    struct Item item_holding;

    struct Chisel chisel_small, chisel_medium, chisel_large;
    struct Chisel *chisel;

    struct Chisel_Hammer chisel_hammer;
};

extern struct Game_State *gs;

__declspec(dllexport) void game_init(struct Game_State *_gs);
__declspec(dllexport) void game_deinit(void);
__declspec(dllexport) bool game_run(struct Game_State *_gs);

#endif // GAME_H_
