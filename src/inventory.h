#define INVENTORY_SLOT_COUNT 5

struct Item {
    enum Cell_Type type;
    int amount;
    
    SDL_Texture *texture;
    SDL_Surface *surface;
    int prev_amount; // The amount stored the last time it updated.
};

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

// A generic slot.
struct Slot {
    struct Converter *converter; // Pointer to a converter if needed.
    int inventory_index; // 0 to INVENTORY_SLOT_COUNT
    struct Item item;
    
    enum Slot_Type type;
    
    char name[32];
    int dx, dy;                  // Orientation of the name string.
    
    f32 x, y, w, h;              // If it's in a converter, this is relative to that.
};

struct Inventory {
    struct Slot slots[INVENTORY_SLOT_COUNT];
};

bool add_item_to_inventory_slot(enum Cell_Type type, int amount);
