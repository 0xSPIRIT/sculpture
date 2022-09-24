//
// This file is shared between the platform layer
// and game layer, containing information necessary
// for both.
//
// For example, the Game_State structure, memory
// information, assert macros etc. are all here.
//

#ifndef SHARED_H_
#define SHARED_H_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdbool.h>
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
#include "input.h"
#include "grabber.h"
#include "effects.h"
#include "chisel_blocker.h"
#include "assets.h"

#define internal static
#define persist static // You shouldn't use this anywhere in the game layer.

#define Kilobytes(x) ((Uint64)x*1024)
#define Megabytes(x) ((Uint64)x*1024*1024)
#define Gigabytes(x) ((Uint64)x*1024*1024*1024)
#define Terabytes(x) ((Uint64)x*1024*1024*1024*1024)

// Assert using an SDL MessageBox popup. Prints to the console too.
#define Assert(window, condition) (_assert(condition, window, __func__, __FILE__, __LINE__) ? DebugBreak() : 0 )
// Assert without the popup (no window); use only console instead.
#define AssertNW(condition) (_assert(condition, NULL, __func__, __FILE__, __LINE__) ? DebugBreak() : 0 )

//
// To allocate permanent memory that will persist until
// the end of the session, use persistent allocation.
//   [push_memory(gs->persistent_memory, ...)]
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use transient allocation.
//   [push_memory(gs->transient_memory, ...)]
//

struct Memory {
    char name[64]; // The name of this memory buffer for debugging purposes.

    Uint8 *data;
    Uint8 *cursor;
    Uint64 size;
};

// Theoretically contains the entirety of the game's state.
// Perhaps to be able to load game states totally, we could
// add an SDL_Event in here as well, and dump this to a file.
struct Game_State {
    struct Memory *persistent_memory, *transient_memory;

    int transient_allocation_count; // A counter for every transient_alloc.

    struct SDL_Window *window;
    struct SDL_Renderer *renderer;

    struct Textures textures; // Contains pointers to SDL textures.
    struct Surfaces surfaces;
    struct Fonts fonts;

    int S; // scale
    int window_width, window_height;
    float delta_time;

    int current_tool, prev_tool;
    int debug_mode;
    
    SDL_Cursor *grabber_cursor, *normal_cursor, *placer_cursor;

    struct Cell *grid_layers[NUM_GRID_LAYERS]; 
    struct Cell *grid, *fg_grid, *gas_grid, *pickup_grid;
    int gw, gh; // Grid width, grid height
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

    struct Level levels[LEVEL_COUNT];
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

// Defined in game.c and main.c
extern struct Game_State *gs;

#define push_memory(mem, num, size) (_push_memory(mem, num, size, __FILE__, __LINE__))

inline void *_push_memory(struct Memory *memory, Uint64 num, Uint64 size_individual, const char *file, int line) {
    Uint64 size;
    void *output = NULL;

    size = num * size_individual;

    // printf("%s Memory :: Allocated %zd at %s(%d)\n", memory->name, size, file, line);

    if (memory->cursor+size > memory->data+memory->size) {
        char message[256] = {0};
        sprintf(message, "Out of Memory!\nAllowed memory: %zd bytes, Attempted allocation to %zd bytes.\n%s:%d",
                memory->size,
                size+(memory->cursor-memory->data),
                file, line);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory Error :(", message, gs->window);
        exit(1);
        return NULL;
    }

    output = memory->cursor;
    memory->cursor += size;

    return output;
}

inline bool _assert(bool condition, SDL_Window *window, const char *func, const char *file, const int line) {
    if (condition) return false;

    char message[64] = {0};
    char line_of_code[2048] = {0};

    FILE *f = fopen(file, "r");
    if (f) {
        int iterator = 0;
        char temp_buffer[2048] = {0};
        while (fgets(temp_buffer, 2048, f)) {
            iterator++;
            if (iterator == line) {
                strncpy(line_of_code, temp_buffer, 2048);
                break;
            }
        }

        sprintf(message, "%s :: at %s:%d\nLine:%s", func, file, line, line_of_code);
        if (window) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion Failed!", message, window);
        }
    } else {
        DebugBreak();
    }

    fprintf(stderr, "\n:::: ASSERTION FAILED ::::\n%s", message);
    fflush(stderr);

    return !condition;
}

// which is an enum in assets.h
#define RenderTarget(gs, which) (gs->textures.render_targets[gs->level_current][which])

#endif // SHARED_H_
