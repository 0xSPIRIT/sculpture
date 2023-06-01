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
    BUTTON_TYPE_TUTORIAL,
    BUTTON_TYPE_POPUP_CONFIRM,
    BUTTON_TYPE_POPUP_CANCEL
};

typedef struct Message {
    char str[100];
    Uint8 alpha;
} Message;

typedef struct {
    bool active;
    char text[128];
    SDL_Rect r;
    Button *a, *b;
} Popup_Confirm;

typedef struct GUI {
    bool popup;
    f32 popup_y, popup_y_vel, popup_h; // For the bottom part
    f32 popup_inventory_y, popup_inventory_y_vel, popup_inventory_h;
    SDL_Texture *popup_texture;
    Tooltip tooltip;
    
    Overlay_Interface overlay_interface;
    
    Button *tool_buttons[TOOL_COUNT];
    
    Popup_Confirm popup_confirm;
    
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

typedef struct Arrow {
    SDL_Texture *texture;
    int x, y, w, h;
} Arrow;

static Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*));
static void tool_button_set_disabled(int level);
static void click_gui_tool_button(void *type_ptr);
static void button_tick(Button *b, void *data);
static void button_draw_prefer_color(Button *b, SDL_Color color);
static void button_draw(Button *b);
static void gui_message_stack_push(const char *str);
static void gui_message_stack_tick_and_draw(void);
static Popup_Confirm popup_confirm_init(void);
static void gui_init(void);
static void gui_tick(void);
static void profile_array(Cell *desired, char out[64][CELL_TYPE_COUNT], int *count);
static void gui_draw_profile();
static void gui_draw(void);
static void popup_confirm_confirm(void* ptr);
static void popup_confirm_cancel(void* ptr);
static Popup_Confirm popup_confirm_init();
static void popup_confirm_tick_and_draw(Popup_Confirm *popup);
static void gui_popup_draw(void);
static bool is_cell_stone(int type);
static int get_cell_tier(int type);
static int get_number_unique_inputs(Item *input1, Item *input2);