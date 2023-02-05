#define MAX_TOOLTIP_LEN 128
#define MAX_TOOLTIP_LINE_LEN 128
#define CONVERTER_NAME_LEN 32

#define GUI_POPUP_H (0.4375*gs->window_height) // window_height/2
#define GUI_H (gs->window_width/8)

#define MAX_MESSAGE_STACK 256

#define ITEM_SIZE (0.0625*gs->window_width)

enum Tooltip_Type {
    TOOLTIP_TYPE_OFF,    // Off state
    TOOLTIP_TYPE_BUTTON, // From GUI Buttons
    TOOLTIP_TYPE_ITEM,   // From hovering over items
    TOOLTIP_TYPE_PLACER  // From the placer
};

enum Button_Type {
    BUTTON_TYPE_TOOL_BAR,
    BUTTON_TYPE_CONVERTER,
    BUTTON_TYPE_OVERLAY_INTERFACE,
    BUTTON_TYPE_TUTORIAL
};

struct Tooltip {
    enum Tooltip_Type type;
    
    bool set_this_frame;
    
    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;
};

struct Message {
    char str[100];
    Uint8 alpha;
};

struct GUI {
    bool popup;
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
    bool active;
    bool disabled;
    bool highlighted;
    bool just_had_tooltip; // Used to disable the GUI tooltip when the mouse goes off me.
    void (*on_pressed)(void*);
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
    CONVERTER_INACTIVE // Used in levels where the converter is greyed out.
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
