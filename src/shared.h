#ifndef SHARED_H_
#define SHARED_H_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "blob_hammer.h"
#include "grid.h"
#include "chisel.h"
#include "knife.h"
#include "point_knife.h"
#include "popup.h"
#include "placer.h"
#include "level.h"
#include "gui.h"
#include "boot/input.h"
#include "grabber.h"
#include "effects.h"
#include "chisel_blocker.h"
#include "boot/assets.h"

#define Kilobytes(x) ((Uint64)x*1024)
#define Megabytes(x) ((Uint64)x*1024*1024)
#define Gigabytes(x) ((Uint64)x*1024*1024*1024)
#define Terabytes(x) ((Uint64)x*1024*1024*1024*1024)

//
// To allocate permanent memory that will persist until
// the end of the session, use persist_alloc.
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use temp_alloc.
//

struct Memory {
    uint8_t *data;
    uint8_t *cursor;
    size_t size;
};

// Put all the SDL initialization eg texture creation or
// window / renderer creation & fonts init into the platform layer.
// The platform layer will need to know the sizes of these types
// so we should include the header files as it is right now,
// but we should not compile any .c files from the actual game
// code. asset.c is not part of the game layer, it's part of the
// platform layer.
//
// The ***_init() functions in the game layer doesn't actually allocate
// anything.
struct Game_State {
    struct Memory memory;

    int allocation_count;

    struct SDL_Window *window;
    struct SDL_Renderer *renderer;

    struct Textures textures; // Contains pointers to SDL textures.
    struct Surfaces surfaces;

    int S; // scale
    int window_width, window_height;
    float delta_time;

    SDL_Texture *render_texture;

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
    int state_count; // Number of states saved.

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

#define persist_alloc(num, size) (_allocate(num, size, __FILE__, __LINE__))
#define temp_alloc(num, size) (_temp_alloc(num, size, __FILE__, __LINE__))
#define temp_dealloc(ptr) (_temp_dealloc(ptr, __FILE__, __LINE__))

inline void *_allocate(size_t num, size_t size_individual, const char *file, int line) {
    size_t size;
    void *output;

    size = num * size_individual;

    if (gs->memory.cursor+size > gs->memory.data+gs->memory.size) {
        char message[256] = {0};
        sprintf(message, "Out of Memory!\nAllowed memory: %zd bytes, Attempted allocation to %zd bytes.\n%s:%d", gs->memory.size, size+(gs->memory.cursor-gs->memory.data), file, line);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory Error :(", message, gs->window);
        exit(1);
        return NULL;
    }

    output = gs->memory.cursor;
    gs->memory.cursor += size;

    return output;
}

// Used for allocations you want to deallocate soon.
inline void *_temp_alloc(size_t num, size_t size_individual, const char *file, int line) {
    if (!num || !size_individual) return NULL;
    void *allocation = calloc(num, size_individual);
    if (!allocation) {
        char message[256] = {0};
        sprintf(message, "Failed temporary allocation at %s:%d", file, line);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory Error :(", message, gs->window);
    }
    gs->allocation_count++;
    return allocation;
}

inline void _temp_dealloc(void *ptr, const char *file, int line) {
    gs->allocation_count--;
    free(ptr);
}

#endif // SHARED_H_
