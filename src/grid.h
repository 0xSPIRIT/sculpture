#define MAX_PRESSURE (gs->chisel->size == 2 ? 48.0 : 120.0)

#define MAX_OBJECTS 32
#define NUM_GRID_LAYERS 4

#define GRAV 0.5
#define MAX_GRAV 4

#define DEGTORAD (2*M_PI / 360.0)

enum Blob_Type {
    BLOB_CIRCLE_A,
    BLOB_CIRCLE_B,
    BLOB_CIRCLE_C,
    BLOB_RECTANGLE,
};

enum Tool_Type {
    TOOL_CHISEL_SMALL,
    TOOL_CHISEL_MEDIUM,
    TOOL_CHISEL_LARGE,
    TOOL_OVERLAY,
    TOOL_DELETER,
    TOOL_PLACER,
    TOOL_GRABBER,
    TOOL_FINISH_LEVEL,
    TOOL_COUNT
};

// Don't mess with the arrangement,
// It'll fuck up the table in level.c!
enum Cell_Type {
    CELL_NONE,
    
    CELL_DIRT,
    CELL_SAND,
    
    CELL_WATER,
    CELL_ICE,
    CELL_STEAM,
    
    CELL_WOOD_LOG,
    CELL_WOOD_PLANK,
    
    CELL_COBBLESTONE,
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
};

typedef struct Cell {
    enum Cell_Type type;  // The type of this cell.
    int id;               // Unique ID for each cell.
    int object;           // Object index the cell belongs. -1 for none
    int temp;             // Temporary variable for algorithms
    Uint8 pressure;       // Pressure from 0 to 255.
    bool is_initial;      // Is this from the initial state?
    bool updated;         // Updated for the frame yet?
    int time;             // Time since set
    f32 vx, vy;           // Velocity
    f32 vx_acc, vy_acc;   // When vel < 1, we need to keep track of that
    int px, py;           // Previous positions
    int rand;             // Random value per cell
} Cell;

// Provides higher fidelity X and Y values
// SOA not AOS for simplicity.
typedef struct Dust_Data {
    enum Cell_Type *types;
    f64 *xs;
    f64 *ys;
    f64 *vxs;
    f64 *vys;
    int *timers;
    int count;
} Dust_Data;

typedef struct Line {
    int x1, y1, x2, y2;
} Line;

int compare_cells_to_int_count(Cell *a, int *b);
