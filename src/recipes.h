typedef struct Recipe_Item {
    int converter; // CONVERTER_FUEL or CONVERTER_MATERIAL
    int fuel;
    int input1;
    int input2;
    int output;
    bool alternate_button; // Is there a button to alternate through the conversions, or will we auto-switch through everything?
} Recipe_Item;

typedef struct Recipes {
    bool active;
    f64 y, y_to;

    Button button;
    
    Recipe_Item *conversions;
    int conversions_count;

    int timer;

    int override_indices[32];

    int definition;
    u8 definition_alpha;

    bool holding_scroll_bar;
    int max_height;
} Recipes;

enum {
    CELL_ANY_STONE = -1,
    CELL_ANY_COAL = -2,
    CELL_ANY_CONVERT_TO_COAL = -3,
    CELL_ANY_FUEL = -4,
    CELL_ANY_STEAM_OR_ICE = -5,
    CELL_ANY_NONE_OR_UNREFINED_COAL = -6,
    CELL_ANY_WATER_OR_ICE = -7,
};

bool can_recipes_be_active(void);
void recipe_draw(int target);
