//
// This file is shared between the platform layer
// and game layer, containing information necessary
// for both.
//
// For example, the Game_State structure, memory
// information, assert macros etc. are all here.
//

#define Kilobytes(x) ((Uint64)x*1024)
#define Megabytes(x) ((Uint64)x*1024*1024)
#define Gigabytes(x) ((Uint64)x*1024*1024*1024)
#define Terabytes(x) ((Uint64)x*1024*1024*1024*1024)

// Assert using an SDL MessageBox popup. Prints to the console too.
#define Assert(condition) (_assert((condition), gs->window, __func__, __FILE__, __LINE__))
// Assert without the popup (no window); use only console instead.
#define AssertNW(condition) (_assert((condition), NULL, __func__, __FILE__, __LINE__))

#define arena_alloc(mem, num, size) (_arena_alloc(mem, num, size, __FILE__, __LINE__))
// 'which' is an enum in assets.h
#define RenderTarget(which) (gs->textures.render_targets[gs->level_current][which])

#include "all.h"

//
// To allocate permanent memory that will persist until
// the end of the session, use persistent allocation.
//   [arena_alloc(gs->persistent_memory, ...)]
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use transient allocation.
//   [arena_alloc(gs->transient_memory, ...)]
//
struct Memory {
    Uint8 *data;
    Uint8 *cursor;
    Uint64 size;
};

// Contains the entirety of the game's state.
// If you're adding values at runtime into the struct, add it
// to the end, because we have pointers pointing to variables
// in here and we don't want to mess that up.
struct Game_State {
    struct Memory *persistent_memory, *transient_memory;

    struct SDL_Window *window;
    struct SDL_Renderer *renderer;

    SDL_Event *event;

    struct Textures textures; // Contains pointers to SDL textures.
    struct Surfaces surfaces;
    struct Fonts fonts;

    int S;
    int window_width, window_height;
    f32 delta_time;

    int current_tool, previous_tool;
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
    struct Deleter deleter;
    clock_t global_start, global_end;

    bool undo_initialized;
    // struct Save_State *current_state, *start_state;
    struct Save_State save_states[MAX_UNDO];
    int save_state_count; // Number of states saved.

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

    struct Blocker blocker;

    struct Converter *material_converter, *fuel_converter;

    struct Item item_holding;

    struct Chisel chisel_small, chisel_medium, chisel_large;
    struct Chisel *chisel;

    struct Chisel_Hammer chisel_hammer;
};

struct Game_State *gs = NULL;

inline void _assert(bool condition, SDL_Window *window, const char *func, const char *file, const int line) {
    if (condition) return;

    char message[64] = {0};
    char line_of_code[2048] = {0};

    FILE *f = fopen(file, "r");
    if (f) {
        int iterator = 0;
        while (fgets(line_of_code, 2048, f)) {
            iterator++;
            if (iterator == line) {
                break;
            }
        }

        sprintf(message, "%s :: at %s:%d\nLine:%s", func, file, line, line_of_code);
        if (window) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion Failed!", message, window);
        }
    } else {
        __debugbreak();
    }

    fprintf(stderr, "\n:::: ASSERTION FAILED ::::\n%s", message);
    fflush(stderr);

    __debugbreak();
}

inline void *_arena_alloc(struct Memory *memory, Uint64 num, Uint64 size_individual, const char *file, int line) {
    Uint64 size;
    void *output = NULL;

    size = num * size_individual;

    const int debug_memory = false;
    
    if (debug_memory && gs) {
        char memory_name[64] = {0};
        if (memory == gs->persistent_memory) {
            strcpy(memory_name, "Persistent");
        } else if (memory == gs->transient_memory) {
            strcpy(memory_name, "Transient");
        } else {
            Assert(0);
        }

        printf("%s Memory :: Allocated %zd bytes at %s(%d)\n", memory_name, size, file, line);
    }

    if (memory->cursor+size > memory->data+memory->size) {
        char message[256] = {0};
        sprintf(message, "Out of Memory!\nAllowed memory: %zd bytes, Attempted allocation to %zd bytes.\n%s:%d",
                memory->size,
                size+(memory->cursor-memory->data),
                file, line);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory Error :(", message, gs->window);
        exit(1);
    }

    output = memory->cursor;
    memory->cursor += size;

    return output;
}
