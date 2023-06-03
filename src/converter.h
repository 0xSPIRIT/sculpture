#define MAX_CONVERSION_LINES 100
#define CONVERTER_NAME_LEN 32

enum Converter_Type {
    CONVERTER_MATERIAL,
    CONVERTER_FUEL,
    CONVERTER_COUNT
};

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

typedef struct Converter_Checker {
    Item *input1, *input2;
    int current; // 1 or 2 [0 when first initialized]
} Converter_Checker;

typedef struct Conversions {
    bool active; // Is the panel active?
    
    char *string;
    char lines[MAX_CONVERSION_LINES][100];
    int line_count;
    
    bool calculated_render_target;
    
    SDL_Rect r; // The panel position and size.
} Conversions;

static Converter *converter_init(int type, bool allocated);
static void all_converters_init(void);

static void converter_set_state(Converter *converter, enum Converter_State state);
static bool converter_is_layout_valid(Converter *converter);
static void converter_begin_converting(void *converter_ptr);

static void converter_draw(int target, Converter *converter);
static void all_converters_draw(int target);

static void auto_set_material_converter_slots(Converter *converter);
static void converter_setup_position(Converter *converter);
static int get_number_unique_inputs(Item *input1, Item *input2);
    
static Converter_Checker converter_checker(Item *input1, Item *input2);

static bool is_either_input_type(Converter_Checker *checker, int type, bool restart);
static bool is_either_input_tier(Converter_Checker *checker, int tier, bool is_fuel, bool restart);
static bool is_either_input_stone(Converter_Checker *checker, bool restart);

static int  fuel_converter_convert(Item *input1, Item *input2);
static int  material_converter_convert(Item *input1, Item *input2, Item *fuel);
static bool converter_convert(Converter *converter);
static void converter_tick(Converter *converter);
static void all_converters_tick(void);
static void setup_item_indices();
