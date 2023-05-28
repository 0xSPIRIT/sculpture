#define LEVEL_COUNT 11
#ifndef ALASKA_RELEASE_MODE
#define POPUP_TIME 120
#else
#define POPUP_TIME 220
#endif

#define SOURCE_CELL_MAX 8

#define COMPARE_LEEWAY 3

#define SHOW_NARRATION 1

enum Level_State {
    LEVEL_STATE_INTRO,
    LEVEL_STATE_NARRATION,
    LEVEL_STATE_PLAY,
    LEVEL_STATE_OUTRO,
    LEVEL_STATE_CONFIRMATION
};

typedef struct Source_Cell {
    int x, y;
    int type;
} Source_Cell;

typedef struct Level {
    enum Level_State state;
    
    int index;
    char name[256];
    
    int effect_type;
    
    Cell *desired_grid; // What the inspiration is
    Cell *initial_grid; // Starting state of grid
    
    char profile_lines[64][CELL_TYPE_COUNT];
    
    Source_Cell source_cell[SOURCE_CELL_MAX];
    Source_Cell default_source_cell[SOURCE_CELL_MAX];
    
    int source_cell_count, default_source_cell_count;
    int w, h;
    int popup_time_current, popup_time_max;
    
    bool first_frame_compare;
    
    f64 outro_alpha, desired_alpha;
    bool off;
} Level;

void level_setup_initial_grid(void);
int  level_add(const char *name, const char *desired_image, const char *initial_image, int effect_type);
void levels_setup(void);
void goto_level(int lvl);
void level_set_state(int level, enum Level_State state);
void goto_level_string_hook(const char *string);
void level_tick(Level *level);

void level_tick_intro(Level *level);
void level_tick_outro(Level *level);
void level_tick_play(Level *level);
    
void level_draw_confirm(void);
void level_draw_narration(void);
void level_draw_name_intro(Level *level, SDL_Rect rect);
void level_draw_desired_grid(Level *level, int dx, int dy);
void level_draw_intro(Level *level);
void level_draw_outro(Level *level);
void level_draw_outro_or_play(Level *level);
    
void level_draw(Level *level);

SDL_Color type_to_rgb(int type);

int  rgb_to_type(Uint8 r, Uint8 g, Uint8 b);
void level_output_to_png(const char *output_file);
void level_get_cells_from_image(const char *path, Cell **out, Source_Cell *source_cells, int *out_source_cell_count, int *out_w, int *out_h);

Uint8 type_to_rgb_table[CELL_TYPE_COUNT*4] = {
    // Type              R    G    B
    CELL_NONE,            0,   0,   0,
    CELL_DIRT,          200,   0,   0,
    CELL_SAND,          255, 255,   0,
    
    CELL_WATER,           0,   0, 255,
    CELL_ICE,           188, 255, 255,
    CELL_STEAM,         225, 225, 225,
    
    CELL_WOOD_LOG,      128,  80,   0,
    CELL_WOOD_PLANK,    200,  80,   0,
    
    CELL_COBBLESTONE,   128, 128, 128,
    CELL_MARBLE,        255, 255, 255,
    CELL_SANDSTONE,     255, 128,   0,
    
    CELL_CEMENT,        130, 130, 130,
    CELL_CONCRETE,      140, 140, 140,
    
    CELL_QUARTZ,        200, 200, 200,
    CELL_GLASS,         180, 180, 180,
    
    CELL_GRANITE,       132, 158, 183,
    CELL_BASALT,         32,  32,  32,
    CELL_DIAMOND,       150, 200, 200,
    
    CELL_UNREFINED_COAL, 50,  50,  50,
    CELL_REFINED_COAL,   70,  70,  70,
    CELL_LAVA,          255,   0,   0,
    
    CELL_SMOKE,         170, 170, 170,
    CELL_DUST,          150, 150, 150
};