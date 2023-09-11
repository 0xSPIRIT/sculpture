#define GUI_POPUP_H (0.4375*gs->game_height) // game_height/2
#define GUI_H (gs->game_width/8)

#define MAX_MESSAGE_STACK 256

#define ITEM_SIZE (0.0625*gs->game_width)

typedef enum {
    BUTTON_TOOL_BAR = 1,
    BUTTON_CONVERTER,
    BUTTON_OVERLAY_INTERFACE,
    BUTTON_TUTORIAL,
    BUTTON_EOL_POPUP_CONFIRM,
    BUTTON_EOL_POPUP_CANCEL,
    BUTTON_RESTART_POPUP_CONFIRM,
    BUTTON_RESTART_POPUP_CANCEL,
    BUTTON_DESTROY
} Button_Type;

typedef struct Message {
    char str[100];
    u8 alpha;
} Message;

typedef struct GUI {
    bool popup; // shouldn't this be called popup_active? Bad name!!
    f32 popup_y, popup_y_vel, popup_h; // For the bottom part
    f32 popup_inventory_y, popup_inventory_y_vel, popup_inventory_h;
    Texture *popup_texture; // unused
    Tooltip tooltip;

    f64 stored_game_scale; // So we can know to resize if the scale changed.

    Overlay_Interface overlay_interface;

    Button *tool_buttons[TOOL_COUNT];

    Popup_Confirm eol_popup_confirm, restart_popup_confirm;

    Message message_stack[MAX_MESSAGE_STACK];
    int message_count;
} GUI;

typedef struct Button {
    Button_Type type;
    int x, y, w, h;
    int index;
    Texture *texture;
    char tooltip_text[128];
    bool active;
    bool disabled;
    bool highlighted;
    bool just_had_tooltip; // Used to disable the GUI tooltip when the mouse goes off me.
    void (*on_pressed)(void*);
    Preview *preview;
} Button;

typedef struct Arrow {
    Texture *texture;
    int x, y, w, h;
} Arrow;

static Button *button_allocate(Button_Type type, Texture *texture, const char *tooltip_text, void (*on_pressed)(void*));
static void tool_button_set_disabled(int level);
static void click_gui_tool_button(void *type_ptr);
static bool button_tick(Button *b, void *data);
static void button_draw_prefer_color(int target, Button *b, SDL_Color color);
static void button_draw(int target, Button *b);
static void gui_message_stack_push(const char *str);
static void gui_init(void);
static void gui_tick(void);
static void profile_array(Cell *desired, char out[64][CELL_TYPE_COUNT], int *count);
static void gui_draw_profile(int target);
static void gui_draw(int target);
static void gui_popup_draw(int target);
static bool is_cell_stone(int type);
static int get_cell_tier(int type);
static int get_number_unique_inputs(Item *input1, Item *input2);
