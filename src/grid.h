#define MAX_PRESSURE (gs->chisel->size == 2 ? 48.0 : 120.0)

#define MAX_OBJECTS 32
#define NUM_GRID_LAYERS 2

#define GRAV 0.5
#define MAX_GRAV 4

typedef enum {
    TOOL_CHISEL_SMALL,
    TOOL_CHISEL_MEDIUM,
    TOOL_CHISEL_LARGE,
    TOOL_OVERLAY,
    TOOL_GRABBER,
    TOOL_PLACER,
    TOOL_DESTROY,
    TOOL_FINISH_LEVEL,
    TOOL_COUNT
} Tool_Type;

typedef enum Cell_Type {
    CELL_NONE,

    CELL_DIRT,
    CELL_SAND,

    CELL_WATER,
    CELL_ICE,
    CELL_STEAM,

    CELL_STONE,
    CELL_MARBLE,
    CELL_SANDSTONE,

    CELL_CEMENT,
    CELL_CONCRETE,

    CELL_QUARTZ,
    CELL_GLASS,

    CELL_GRANITE,
    CELL_BASALT,
    CELL_DIAMOND,

    CELL_UNREFINED_COAL,
    CELL_REFINED_COAL,
    CELL_LAVA,

    CELL_SMOKE,
    CELL_DUST,

    CELL_TYPE_COUNT
} Cell_Type;

typedef struct {
    u8  type;            // The type of this cell. (Cell_Type)
    u16 id;              // Unique ID for each cell.
    s16 object;          // Object index the cell belongs. -1 for none
    s16 temp;            // Temporary variable for algorithms
    u8  is_initial;      // Is this from the initial state?
    u8  updated;         // Updated for the frame yet?
    u16 rand;            // Random value per cell
    f32 vx, vy;          // Velocity
    f32 vx_acc, vy_acc;  // When vel < 1, we need to keep track of that
    u8  px, py;          // Previous positions
} Cell;

typedef struct Line {
    int x1, y1, x2, y2;
} Line;

static int compare_cells_to_int_count(Cell *a, int *b);
static SDL_Color pixel_from_index_grid(Cell *grid, enum Cell_Type type, int i);
static SDL_Color pixel_from_index(enum Cell_Type type, int i);
static bool is_array_empty(Cell *grid);
