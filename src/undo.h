#define MAX_UNDO 16384

typedef struct {
    Uint8 type;
    Uint8 object;
} Stored_Cell;

#define UNDO_GRID_LAYERS 1

// We don't want to save the entire memory of the game since
// that will destroy memory usage.
typedef struct Save_State {
    Stored_Cell *grid_layers[1];

    Item placer_items[TOTAL_SLOT_COUNT+1];
    // Format: Inventory Slots(5),
    //         Material Converter Slots(4),
    //         Fuel Converter Slots(3)
    //         Item Holding(1)

    Source_Cell source_cell[SOURCE_CELL_MAX];
    int source_cell_count;
} Save_State;

static void undo_system_init(void);
static void undo_system_reset(void);
static Save_State *current_state(void);
static void save_state_to_next(void);
static void set_state(int num);
static bool is_current_grid_same_as(Save_State *state);
static bool is_current_slots_same_as(Save_State *state);
static void set_state_to_string_hook(const char *string);
static void undo(void);
