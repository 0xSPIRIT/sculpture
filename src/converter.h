#ifndef CONVERTER_H
#define CONVERTER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define CONVERTER_NAME_LEN 256

struct Item {
    int type;
    int amount;
};

// If adding more slots, ensure that the slots
// every converter has is at the top.
enum Slot_Type {
    SLOT_INPUT1,
    SLOT_INPUT2,
    SLOT_OUTPUT,
    SLOT_FUEL
};

struct Slot {
    struct Converter *converter; // The converter this slot belongs to.
    struct Item item;

    int type;                    // Slot_Type

    char name[32];
    int dx, dy;                  // Orientation of the name string.

    float x, y, w, h;            // Relative to the converter it's in.
};

enum Converter_Type {
    CONVERTER_MATERIAL,
    CONVERTER_FUEL
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
    int type, state;
    char *name;

    float x, y, w, h;
    int speed;
    
    struct Slot *slots;
    int slot_count;

    struct Button *go_button;

    struct Arrow arrow;
};

extern struct Converter *material_converter,
                        *fuel_converter;
extern struct Item item_holding;

void item_init();
void item_deinit();
void item_draw(struct Item *item, int x, int y, int w, int h);
void item_tick(struct Item *item, struct Slot *slot, int x, int y, int w, int h);

void all_converters_init();
void all_converters_deinit();
void all_converters_tick();
void all_converters_draw();

bool is_mouse_in_converter(struct Converter *converter);
bool is_mouse_in_slot(struct Slot *slot);

struct Converter *converter_init(int type);
void converter_deinit(struct Converter *converter);
void converter_tick(struct Converter *converter);
void converter_draw(struct Converter *converter);
void converter_begin_converting(void *converter);

void slot_tick(struct Slot *slot);
void slot_draw(struct Slot *slot);

struct Placer *converter_get_current_placer();

#endif  /* CONVERTER_H */
