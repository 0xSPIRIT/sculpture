//
// This file is shared between the platform layer
// and game layer, containing information necessary
// for both.
//
// For example, the Game_State structure, memory
// information, assert macros etc. are all here.
//

#include "headers.h" // Used to get type size information.

#define Kilobytes(x) ((u64)x*1024)
#define Megabytes(x) ((u64)x*1024*1024)
#define Gigabytes(x) ((u64)x*1024*1024*1024)
#define Terabytes(x) ((u64)x*1024*1024*1024*1024)

#ifdef ALASKA_RELEASE_MODE
  #define __debugbreak() (exit(1))
#endif

#ifdef __EMSCRIPTEN__
  #define SDL_ShowSimpleMessageBox(a, b, c, d) (puts(b))
#endif

// Assert using an SDL MessageBox popup. Prints to the console too.
#ifndef ALASKA_RELEASE_MODE
  #define Assert(cond) if(!(cond)) { _assert(__func__, __FILE__, __LINE__), __debugbreak(); }
#else
  #define Assert(cond) ;
#endif

#define PushSize(arena, size) (_push_array(arena, 1, size, __FILE__, __LINE__))
#define PushArray(arena, count, size) (_push_array(arena, count, size, __FILE__, __LINE__))

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
    u8 *data;
    u8 *cursor;
    u64 size;
} Memory_Arena;

typedef enum {
    GAME_STATE_TITLESCREEN,
    GAME_STATE_PLAY
} GameStateEnum;

// Contains the entirety of the game's state.
// If you're adding values at runtime into the struct, add it
// to the end, because we have pointers pointing to variables
// in here and we don't want to mess that up.
typedef struct Game_State {
    Memory_Arena *persistent_memory, *transient_memory;
    
    bool close_game; // Closes the game after the current frame.
    
    // Some stupid hacky debugging variables used around the place.
    char func[64];
    f64 a; // global timer used for profiling, not used anywhere else
    f64 accum;
    int amt;
    bool test;
    bool draw_fps;
    f64 highest_frametime;
    int timer;
    
    bool needs_manual_fps_lock; // If the screen's refresh rate != 60, we need to manually do this.

    f64 dt; // Time taken for previous frame.
    f64 device_pixel_ratio; // Only used in emscripten builds.
    
    int audio_channel_volumes[AUDIO_CHANNEL_COUNT];
    int channel_editing;

    bool just_resized;

    bool web_clicked;
    Pause_Menu pause_menu;

    SDL_Color border_color;

    Audio_Handler audio_handler;

    SDL_Window *window;
    SDL_Renderer *renderer;
    Render render;

    Lighting lighting;

    int levels_completed_perfectly;

    bool is_mouse_on_tab_icon; // Hacky...

    int wasd_popup_alpha; // 0 - 255
    bool wasd_popup_active;

    Recipes recipes;

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
    int game_width, game_height;
    int desktop_w, desktop_h;
    int real_width, real_height;
    f32 delta_time;

    int current_tool, previous_tool;

    int blob_type;

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
    bool did_inventory_tutorial;
    bool did_fuel_converter_tutorial;
    bool did_placer_rectangle_tutorial;

    Hammer hammer;

    SDL_Surface *pixel_format_surf;

    Chisel chisels[3];
    Chisel *chisel;

    int save_state_count; // Number of states saved.
    bool has_player_interacted_since_last_state;
    bool has_any_placed;
    Save_State save_states[MAX_UNDO]; // Big array (65MB I think?)
} Game_State;

Game_State *gs = null;

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
void *_push_array(Memory_Arena *memory, u64 num, u64 size_individual, const char *file, int line) {
    u64 size;
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

        Log("%s Memory_Arena :: Allocated %llu bytes at %s(%d)\n", memory_name, size, file, line);
    }

    if (memory->cursor+size > memory->data+memory->size) {
        char message[256] = {0};
        sprintf(message, "Out of Memory!\nAllowed memory: %llu bytes, Attempted allocation to %llu bytes.\n%s:%d",
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

// NOTE: Kind of a weird place to put this but it's the end of the project
//       so I don't care about formatting it in a proper place.
//       This is related to audio, and setting the volume of each of the
//       channels every frame.
static void assign_audio_channel_volumes(void) {
    f32 master = gs->pause_menu.slider;
    Mix_Volume(AUDIO_CHANNEL_CHISEL,   master * gs->audio_channel_volumes[AUDIO_CHANNEL_CHISEL]);
    Mix_Volume(AUDIO_CHANNEL_GUI,      master * gs->audio_channel_volumes[AUDIO_CHANNEL_GUI]); 
    Mix_Volume(AUDIO_CHANNEL_MUSIC,    master * gs->audio_channel_volumes[AUDIO_CHANNEL_MUSIC]); 
    Mix_Volume(AUDIO_CHANNEL_AMBIENCE, master * gs->audio_channel_volumes[AUDIO_CHANNEL_AMBIENCE]); 
}