#define MAX_PRESSURE (gs->chisel->size == 2 ? 48.0 : 120.0)

#define MAX_OBJECTS 32
#define NUM_GRID_LAYERS 4

#define GRAV 0.5
#define MAX_GRAV 4

enum Tool_Type {
    TOOL_CHISEL_SMALL,
    TOOL_CHISEL_MEDIUM,
    TOOL_CHISEL_LARGE,
    TOOL_OVERLAY,
    TOOL_DELETER,
    TOOL_PLACER,
    TOOL_GRABBER,
    TOOL_COUNT
};

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
    // CELL_GOLD,    // Perhaps too much cell types. Aren't that many levels.
    
    CELL_UNREFINED_COAL,
    CELL_REFINED_COAL,
    CELL_LAVA,

    CELL_SMOKE,
    CELL_DUST,

    CELL_TYPE_COUNT
};

struct Cell {
    enum Cell_Type type;  // The type of this cell.
    int id;               // Unique ID for each cell.
    int object;           // Object index the cell belongs. -1 for none
    int temp;             // Temporary variable for algorithms
    bool updated;         // Updated for the frame yet?
    Uint8 depth;          // Z-depth. Controls brightness of the cell.
    int time;             // Time since set
    f32 vx, vy;           // Velocity
    f32 vx_acc, vy_acc;   // When vel < 1, we need to keep track of that
    int px, py;           // Previous positions
    int rand;             // Random value per cell
};

struct Blob_Data {
    Uint32 *blobs; // Grid (gs->gw, gs->gh) with blob IDs as ints
    int *blob_pressures; // Index into this using the blob index.
    int blob_count;
};

struct Object {
    struct Blob_Data blob_data[3]; // 3 Blob sizes for 3 chisels. 
    int cell_count;
};
