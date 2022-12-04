#define LEVEL_COUNT 10
#define POPUP_TIME 45

enum Level_State {
    LEVEL_STATE_INTRO,
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
    struct Source_Cell source_cell[256];
    int source_cell_count;
    int w, h;
    int popup_time_current, popup_time_max;
};
