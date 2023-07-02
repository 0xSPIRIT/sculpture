#define INVENTORY_SLOT_COUNT 5

// We store the item's "amount" texture data
// in textures.item_nums, surfaces.item_nums
// as well as the last known amount in
// gs->item_prev_amounts in order
// to know when we need to update the
// texture.
typedef struct Item {
    int index; // I can't be bothered to check what this is for.
    enum Cell_Type type;
    int amount;
} Item;

// If adding more slots, ensure that the slots
// every converter has is at the top.
enum Slot_Type {
    SLOT_INPUT1,
    SLOT_INPUT2,
    SLOT_OUTPUT,
    SLOT_FUEL,
    SLOT_MAX_COUNT
};

#define TOTAL_SLOT_COUNT (SLOT_MAX_COUNT * CONVERTER_COUNT + INVENTORY_SLOT_COUNT - 1)

typedef struct Converter Converter;

// A generic slot.
typedef struct {
    Converter *converter; // Pointer to a converter if needed.
    int inventory_index; // 0 to INVENTORY_SLOT_COUNT
    Item item;

    enum Slot_Type type;

    char name[32];
    int dx, dy;                  // Orientation of the name string.

    f32 x, y, w, h;              // If it's in a converter, this is relative to that.
} Slot;

typedef struct Inventory {
    Slot slots[INVENTORY_SLOT_COUNT];
} Inventory;

static bool add_item_to_inventory_slot(enum Cell_Type type, int amount);
static bool can_add_item_to_inventory(enum Cell_Type type);

static bool is_mouse_in_slot(Slot *slot);
static bool was_mouse_in_slot(Slot *slot);

static void inventory_init(void);
static void inventory_setup_slots(void);

static void item_tick(Item *item, Slot *slot, int x, int y, int w, int h);
static void slot_tick(Slot *slot);
static void inventory_tick();
static void inventory_draw(int target);

static bool can_add_item_to_inventory(enum Cell_Type type);
static bool add_item_to_inventory_slot(enum Cell_Type type, int amount);

static void item_draw(int target, Item *item, int x, int y, int w, int h);
static void slot_draw(int target, Slot *slot, f32 rx, f32 ry);

static bool is_cell_fuel(int type);
static bool can_place_item_in_slot(int type, enum Slot_Type slot);
