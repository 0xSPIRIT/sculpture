#define LEVEL_COUNT 11
#ifdef ALASKA_DEBUG
  #define POPUP_TIME 90
#else
  #define POPUP_TIME 180
#endif

#define SOURCE_CELL_MAX 8

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

enum Level_State {
    LEVEL_STATE_INTRO,
    LEVEL_STATE_NARRATION,
    LEVEL_STATE_PLAY,
    LEVEL_STATE_OUTRO
};

struct Source_Cell {
    int x, y;
    int type;
};

struct Level {
    enum Level_State state;
    int index;
    char name[256];
    int effect_type;
    struct Cell *desired_grid; // What the inspiration is
    struct Cell *initial_grid; // Starting state of grid
    char profile_lines[64][CELL_TYPE_COUNT];
    struct Source_Cell source_cell[SOURCE_CELL_MAX];
    struct Source_Cell default_source_cell[SOURCE_CELL_MAX];
    int source_cell_count, default_source_cell_count;
    int w, h;
    int popup_time_current, popup_time_max;
};

void level_set_state(int level, enum Level_State state);
