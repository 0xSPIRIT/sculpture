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
#define Assert(cond) if(!(cond)) { _assert(__func__, __FILE__, __LINE__), __debugbreak(); }

#define PushSize(arena, size) (_push_array(arena, 1, size, __FILE__, __LINE__))
#define PushArray(arena, count, size) (_push_array(arena, count, size, __FILE__, __LINE__))

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
typedef struct Memory_Arena {
    Uint8 *data;
    Uint8 *cursor;
    Uint64 size;
} Memory_Arena;

typedef enum  {
    GAME_STATE_TITLESCREEN,
    GAME_STATE_PLAY
} GameStateEnum;

// Contains the entirety of the game's state.
// If you're adding values at runtime into the struct, add it
// to the end, because we have pointers pointing to variables
// in here and we don't want to mess that up.
typedef struct Game_State {
    Memory_Arena *persistent_memory, *transient_memory;

    f64 dt; // Time taken for previous frame.
    
    Audio_Handler audio_handler;

    bool use_software_renderer;

    SDL_Window *window;
    SDL_Renderer *renderer;
    Render render;
    
    bool is_mouse_on_tab_icon; // Hacky...

    SDL_Point real_top_left; // Probably should be in Render.

    // All stored surfaces and textures.
    Textures textures;
    Surfaces surfaces;

    Preview current_preview;
    Preview tool_previews[TOOL_COUNT];

    Background background;

    GameStateEnum gamestate;

    Titlescreen titlescreen;

    SDL_Event *event;

    Fade fade;

    Conversions converter;

    Tutorial_Rect tutorial;
    bool show_tutorials;

    bool level1_set_highlighted;

    Credits credits;
    Narrator narrator;
    Object3D obj;

    Timelapse timelapse;

    Audio audio;

    Fonts fonts;

    bool is_mouse_over_any_button;

    f64 S;
    bool fullscreen;
    int window_width, window_height;
    int desktop_w, desktop_h;
    int real_width, real_height;
    f32 delta_time;

    int current_tool, previous_tool;

    SDL_Cursor *grabber_cursor, *normal_cursor, *placer_cursor;

    enum Blob_Type blob_type;

    Dust dust[MAX_DUST_COUNT];
    int dust_count;
    
    Cell *grid_layers[NUM_GRID_LAYERS];
    Cell *grid, *gas_grid; // Pointers into grid_layers

    // grid = regular everyday grid
    // gas_grid = only used for gases

    bool resized;

    int gw, gh; // Grid width, grid height
    int grid_show_ghost;

    int object_count, object_current;
    int do_draw_blobs, do_draw_objects;

    bool paused;
    int frames;
    bool step_one;

    Inventory inventory;

    Overlay overlay;

    bool undo_initialized;

    Save_State save_states[MAX_UNDO];
    int save_state_count; // Number of states saved.
    bool has_player_interacted_since_last_state;
    bool has_any_placed;

    Text_Field text_field;

    bool creative_mode;

    Placer placers[PLACER_COUNT];
    int current_placer;

    Level levels[LEVEL_COUNT];
    int level_current, level_count, new_level;

    GUI gui;
    SDL_Texture *gui_texture;

    Input input;

    Grabber grabber;
    Effect current_effect;

    Converter *material_converter, *fuel_converter;

    Item item_holding;

    bool did_placer_hard_tutorial;
    bool did_chisel_tutorial;
    bool did_undo_tutorial;
    bool did_pressure_tutorial;
    bool did_inventory_tutorial;
    bool did_fuel_converter_tutorial;
    bool did_placer_rectangle_tutorial;

    Hammer hammer;

    SDL_Surface *pixel_format_surf;

    Chisel chisel_small, chisel_medium, chisel_large;
    Chisel *chisel;
} Game_State;

static Game_State *gs = null;

static void _assert(const char *func, const char *file, const int line) {
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

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion Failed!", message, null);

    Error("\n:::: ASSERTION FAILED ::::\n%s", message);
}

// Gives pointer to zeroed memory.
static void *_push_array(Memory_Arena *memory, Uint64 num, Uint64 size_individual, const char *file, int line) {
    Uint64 size;
    void *output = null;

    size = num * size_individual;

    const int debug_memory = false;

    if (debug_memory && gs != null) {
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
