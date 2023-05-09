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

typedef struct Tooltip {
    enum Tooltip_Type type;
    
    bool set_this_frame;
    
    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;
    
    Preview *preview;
} Tooltip;

typedef struct Message {
    char str[100];
    Uint8 alpha;
} Message;

typedef struct GUI {
    bool popup;
    f32 popup_y, popup_y_vel, popup_h; // For the bottom part
    f32 popup_inventory_y, popup_inventory_y_vel, popup_inventory_h;
    SDL_Texture *popup_texture;
    Tooltip tooltip;
    
    Overlay_Interface overlay_interface;
    
    Button *tool_buttons[TOOL_COUNT];
    
    Message message_stack[MAX_MESSAGE_STACK];
    int message_count;
} GUI;

typedef struct Button {
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
    Preview *preview;
} Button;

enum Converter_Type {
    CONVERTER_MATERIAL,
    CONVERTER_FUEL,
    CONVERTER_COUNT
};

typedef struct Arrow {
    SDL_Texture *texture;
    int x, y, w, h;
} Arrow;

enum Converter_State {
    CONVERTER_OFF,
    CONVERTER_ON,
    CONVERTER_INACTIVE // Used in levels where the converter is greyed out.
};

typedef struct Converter {
    enum Converter_Type type;
    enum Converter_State state;

    char name[CONVERTER_NAME_LEN];

    f32 x, y, w, h;
    int speed; // Amount of cells converted per tick.

    int timer_max, timer_current;
    
    Slot *slots;
    int slot_count;

    Button *go_button;

    Arrow arrow;
} Converter;

typedef struct Converter Converter;

typedef struct Converter_Checker {
    Item *input1, *input2;
    int current; // 1 or 2 [0 when first initialized]
} Converter_Checker;
