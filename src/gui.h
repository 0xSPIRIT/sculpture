#pragma once

#define MAX_TOOLTIP_LEN 128
#define MAX_TOOLTIP_LINE_LEN 128
#define CONVERTER_NAME_LEN 32

#define GUI_POPUP_H 336 // window_height/2
#define GUI_H 96

#define MAX_MESSAGE_STACK 256

#define ITEM_SIZE 48

enum Tooltip_Type {
    TOOLTIP_TYPE_OFF,    // Off state
    TOOLTIP_TYPE_BUTTON, // From GUI Buttons
    TOOLTIP_TYPE_ITEM,   // From hovering over items
    TOOLTIP_TYPE_PLACER  // From the placer
};

enum Button_Type {
    BUTTON_TYPE_TOOL_BAR,
    BUTTON_TYPE_CONVERTER,
    BUTTON_TYPE_OVERLAY_INTERFACE
};

struct Tooltip {
    enum Tooltip_Type type;

    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;
};

struct Message {
    char str[100];
    Uint8 alpha;
};

struct GUI {
    int popup;
    bool is_placer_active;
    f32 popup_y, popup_y_vel, popup_h;
    SDL_Texture *popup_texture;
    struct Tooltip tooltip;

    struct Overlay_Interface overlay_interface;

    struct Button *tool_buttons[TOOL_COUNT];

    struct Message message_stack[MAX_MESSAGE_STACK];
    int message_count;
};

struct Button {
    enum Button_Type type;
    int x, y, w, h;
    int index;
    SDL_Texture *texture;
    char tooltip_text[128];
    bool activated;
    bool just_had_tooltip; // Used to disable the GUI tooltip when the mouse goes off me.
    void (*on_pressed)(void*);
};

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

    f32 x, y, w, h;              // Relative to the converter it's in.
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
