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

Converter *converter_init(int type, bool allocated);
void all_converters_init(void);

void converter_set_state(Converter *converter, enum Converter_State state);
bool converter_is_layout_valid(Converter *converter);
void converter_begin_converting(void *converter_ptr);

void converter_draw(Converter *converter);
void all_converters_draw(void);

void auto_set_material_converter_slots(Converter *converter);
void converter_setup_position(Converter *converter);
int get_number_unique_inputs(Item *input1, Item *input2);
    
Converter_Checker converter_checker(Item *input1, Item *input2);

bool is_either_input_type(Converter_Checker *checker, int type, bool restart);
bool is_either_input_tier(Converter_Checker *checker, int tier, bool is_fuel, bool restart);
bool is_either_input_stone(Converter_Checker *checker, bool restart);

int  fuel_converter_convert(Item *input1, Item *input2);
int  material_converter_convert(Item *input1, Item *input2, Item *fuel);
bool converter_convert(Converter *converter);
void converter_tick(Converter *converter);
void all_converters_tick(void);
void setup_item_indices();
