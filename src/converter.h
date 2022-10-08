#ifndef CONVERTER_H
#define CONVERTER_H

#define CONVERTER_NAME_LEN 32

struct Item {
    enum Cell_Type type;
    int amount;
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

struct Slot {
    struct Converter *converter; // The converter this slot belongs to.
    struct Item item;

    enum Slot_Type type;

    char name[32];
    int dx, dy;                  // Orientation of the name string.

    f32 x, y, w, h;            // Relative to the converter it's in.
};

enum Converter_Type {
    CONVERTER_MATERIAL,
    CONVERTER_FUEL,
    CONVERTER_COUNT
};

struct Arrow {
    SDL_Texture *texture;
    int x, y, w, h;
};

enum Converter_State {
    CONVERTER_OFF,
    CONVERTER_ON,
};

struct Converter {
    enum Converter_Type type;
    enum Converter_State state;

    char name[CONVERTER_NAME_LEN];

    f32 x, y, w, h;
    int speed; // Amount of cells converted per tick.

    int timer_max, timer_current;
    
    struct Slot *slots;
    int slot_count;

    struct Button *go_button;

    struct Arrow arrow;
};

struct Converter_Checker {
    struct Item *input1, *input2;
    int current; // 1 or 2 [0 when first initialized]
};

void item_draw(struct Item *item, int x, int y, int w, int h);
void item_tick(struct Item *item, struct Slot *slot, int x, int y, int w, int h);

void all_converters_init();
void all_converters_tick();
void all_converters_draw();

bool is_mouse_in_converter(struct Converter *converter);
bool is_mouse_in_slot(struct Slot *slot);
bool was_mouse_in_slot(struct Slot *slot);

struct Converter *converter_init(int type);
void converter_tick(struct Converter *converter);
void converter_draw(struct Converter *converter);
bool converter_is_layout_valid(struct Converter *converter);
void converter_begin_converting(void *converter);
void converter_set_state(struct Converter *converter, enum Converter_State state);
bool converter_convert(struct Converter *converter);

int get_number_unique_inputs(struct Item *input1, struct Item *input2);

void slot_tick(struct Slot *slot);
void slot_draw(struct Slot *slot);

struct Placer *converter_get_current_placer();

int get_cell_tier(int type);
bool is_cell_fuel(int type);
bool is_cell_stone(int type);

#endif  /* CONVERTER_H */
