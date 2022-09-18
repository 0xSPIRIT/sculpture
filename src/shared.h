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
#include "boot/input.h"
#include "grabber.h"
#include "effects.h"
#include "chisel_blocker.h"
#include "boot/assets.h"

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
// the end of the session, use persist_alloc.
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use transient_alloc and temp_dealloc respsectively.
//

struct Memory {
    uint8_t *data;
    uint8_t *cursor;
    size_t size;
};

// Theoretically contains the entirety of the game's state.
// Perhaps to be able to load game states totally, we could
// add an SDL_Event in here as well, and dump this to a file.
struct Game_State {
    struct Memory *memory;

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

    // A temporary buffer for algorithms.
    // Allocated persistantly so you don't
    // need to call transient_alloc every time you
    // do the algorithm.
    struct Cell *grid_temp;

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

#define persist_alloc(mem, num, size) (_persist_allocate(mem, num, size, __FILE__, __LINE__))
#define transient_alloc(num, size) (_transient_alloc(num, size, __FILE__, __LINE__))
#define temp_dealloc(ptr) (_temp_dealloc(ptr, __FILE__, __LINE__))

inline void *_persist_allocate(struct Memory *memory, size_t num, size_t size_individual, const char *file, int line) {
    size_t size;
    void *output;

    size = num * size_individual;

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

// Used for allocations you want to deallocate at the end of the frame.
// Try not to use this too much, especially things
// that occur every frame.
inline void *_transient_alloc(size_t num, size_t size_individual, const char *file, int line) {
    if (!num || !size_individual) return NULL;

    // TODO: Replace this with your own allocated transient memory.
    void *allocation = calloc(num, size_individual);

    /* printf("Allocated %zd at %s(%d)!\n", num*size_individual, file, line); */

    if (!allocation) {
        char message[256] = {0};
        sprintf(message, "Failed temporary allocation at %s:%d", file, line);
        fprintf(stderr, message); fflush(stderr);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory Error :(", message, gs->window);
        exit(1);
    }
    gs->transient_allocation_count++;
    return allocation;
}

inline void _temp_dealloc(void *ptr, const char *file, int line) {
    free(ptr);
    /* printf("Freed pointer! at %s(%d)\n", file, line); */
    gs->transient_allocation_count--;
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
