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

#ifdef __EMSCRIPTEN__
#define __debugbreak() (exit(1))
#define SDL_ShowSimpleMessageBox(a, b, c, d) (puts(b))
#endif

// Assert using an SDL MessageBox popup. Prints to the console too.
#define Assert(condition) if(!(condition)) {   _assert(gs->window, __func__, __FILE__, __LINE__), __debugbreak(); }
// Assert without the popup (no window); use only console instead.
#define AssertNW(condition) if(!(condition)) { _assert(NULL, __func__, __FILE__, __LINE__), __debugbreak(); }

#define PushSize(arena, size) (_push_array(arena, 1, size, __FILE__, __LINE__))
#define PushArray(arena, count, size) (_push_array(arena, count, size, __FILE__, __LINE__))

// 'which' is an enum in assets.h
#define RenderTarget(which) (gs->textures.render_targets[gs->level_current][which])


#include "headers.h" // Used to get type size information.

//
// To allocate permanent memory that will persist until
// the end of the session, use persistent allocation.
//   [PushArray(gs->persistent_memory, ...)]
//
// Otherwise, when allocating memory that you will
// just need for a specific function and will free it,
// use transient allocation.
//   [PushArray(gs->transient_memory, ...)]
//
struct Memory_Arena {
    Uint8 *data;
    Uint8 *cursor;
    Uint64 size;
};

enum State_Game {
    GAME_STATE_TITLESCREEN,
    GAME_STATE_PLAY
};

// Contains the entirety of the game's state.
// If you're adding values at runtime into the struct, add it
// to the end, because we have pointers pointing to variables
// in here and we don't want to mess that up.
struct Game_State {
    struct Memory_Arena *persistent_memory, *transient_memory;
    
    bool use_software_renderer;
    
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;
    
    struct Preview current_preview;
    struct Preview tool_previews[TOOL_COUNT];
    
    enum State_Game gamestate;
    
    struct Titlescreen titlescreen;
    
    SDL_Event *event;
    
    struct Conversions conversions;
    
    struct Tutorial_Rect tutorial;
    bool show_tutorials;
    
    bool level1_set_highlighted;
    
    struct Credits credits;
    struct Narrator narrator;
    struct Object3D obj;
    
    struct View view;
    
    struct Timelapse timelapse;
    
    struct Audio audio;
    
    int item_prev_amounts[ITEM_COUNT];
    // the amount an item has at the time it was last drawn.
    // (The rendering data is stored in textures.items and surfaces.items)
    
    struct Textures textures; // Contains pointers to SDL textures.
    struct Surfaces surfaces;
    struct Fonts fonts;
    char texts[TEXT_INDEX_COUNT][128];
    
    bool is_mouse_over_any_button;
    
    f64 S;
    bool fullscreen;
    int window_width, window_height;
    int real_width, real_height;
    f32 delta_time;
    
    int current_tool, previous_tool;
    
    SDL_Cursor *grabber_cursor, *normal_cursor, *placer_cursor;
    
    enum Blob_Type blob_type;
    
    struct Dust_Data dust_data;
    struct Cell *grid_layers[NUM_GRID_LAYERS]; 
    struct Cell *grid, *gas_grid; // Pointers into grid_layers
    
    // grid = regular everyday grid
    // gas_grid = only used for gases
    
    int gw, gh; // Grid width, grid height
    int grid_show_ghost;
    
    struct Object objects[MAX_OBJECTS];
    int object_count, object_current;
    int do_draw_blobs, do_draw_objects;
    
    bool paused;
    int frames;
    bool step_one;
    
    struct Inventory inventory;
    
    struct Deleter deleter;
    struct Overlay overlay;
    
    clock_t global_start, global_end;
    
    bool undo_initialized;
    
    struct Save_State save_states[MAX_UNDO];
    int save_state_count; // Number of states saved.
    
    struct Text_Field text_field;
    
    bool creative_mode;
    
    struct Placer placers[PLACER_COUNT];
    int current_placer;
    
    struct Level levels[LEVEL_COUNT];
    int level_current, level_count, new_level;
    
    struct GUI gui;
    SDL_Texture *gui_texture;
    
    struct Input input;
    
    struct Grabber grabber;
    struct Effect current_effect;
    
    struct Converter *material_converter, *fuel_converter;
    
    struct Item item_holding;
    
    bool did_chisel_tutorial;
    bool did_undo_tutorial;
    bool did_pressure_tutorial;
    bool did_inventory_tutorial;
    bool did_fuel_converter_tutorial;
    bool did_placer_rectangle_tutorial;
    
    struct Chisel chisel_small, chisel_medium, chisel_large;
    struct Chisel *chisel;
    
    struct Chisel_Hammer chisel_hammer;
};

struct Game_State *gs = NULL;

void _assert(SDL_Window *window, const char *func, const char *file, const int line) {
    char message[64] = {0};
    char line_of_code[2048] = {0};
    
    FILE *f = fopen(file, "r");
    if (f) {
        int i = 0;
        while (fgets(line_of_code, 2048, f)) {
            i++;
            if (i == line) {
                break;
            }
        }
        sprintf(message, "%s :: at %s:%d\nLine:%s", func, file, line, line_of_code);
    } else {
        sprintf(message, "%s :: at %s:%d", func, file, line);
    }
        
    if (window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion Failed!", message, window);
    }
    
    Error("\n:::: ASSERTION FAILED ::::\n%s", message);
}

// Gives pointer to zeroed memory.
void *_push_array(struct Memory_Arena *memory, Uint64 num, Uint64 size_individual, const char *file, int line) {
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

        Log("%s Memory_Arena :: Allocated %zd bytes at %s(%d)\n", memory_name, size, file, line);
    }

    if (memory->cursor+size > memory->data+memory->size) {
        char message[256] = {0};
        sprintf(message, "Out of Memory_Arena!\nAllowed memory: %zd bytes, Attempted allocation to %zd bytes.\n%s:%d",
                memory->size,
                size+(memory->cursor-memory->data),
                file, line);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Memory_Arena Error :(", message, gs->window);
        exit(1);
    }

    output = memory->cursor;
    memory->cursor += size;

    return output;
}
