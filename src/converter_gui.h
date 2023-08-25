typedef struct Conversions {
    bool active;
    f64 y, y_to;

    Button button;

    int timer;

    int override_indices[32];

    int definition;
    u8 definition_alpha;

    bool holding_scroll_bar;
    int max_height;
} Conversions;

enum {
    CELL_ANY_STONE = -1,
    CELL_ANY_COAL = -2,
    CELL_ANY_CONVERT_TO_COAL = -3,
    CELL_ANY_FUEL = -4,
    CELL_ANY_STEAM_OR_ICE = -5,
    CELL_ANY_NONE_OR_UNREFINED_COAL = -6,
    CELL_ANY_WATER_OR_ICE = -7,
};

static const int conversions[] = {
    // Converter,  Fuel, Input 1, Input 2, Output, Alternate button
    CONVERTER_FUEL,     0,                   CELL_ANY_CONVERT_TO_COAL, 0,                 CELL_UNREFINED_COAL, false,
    CONVERTER_FUEL,     0,                   CELL_UNREFINED_COAL,      CELL_GLASS,        CELL_REFINED_COAL,   false,
    CONVERTER_FUEL,     0,                   CELL_ANY_STONE,           CELL_REFINED_COAL, CELL_LAVA,           false,

    CONVERTER_MATERIAL, CELL_ANY_FUEL,       CELL_STONE,            0,           CELL_MARBLE,    false,
    CONVERTER_MATERIAL, CELL_UNREFINED_COAL, CELL_STONE,            CELL_SAND,   CELL_SANDSTONE, false,
    CONVERTER_MATERIAL, CELL_REFINED_COAL,   CELL_SANDSTONE,        CELL_MARBLE, CELL_QUARTZ,    false,
    CONVERTER_MATERIAL, CELL_UNREFINED_COAL, CELL_SAND,             0,           CELL_GLASS,     false,

    CONVERTER_MATERIAL, CELL_ANY_NONE_OR_UNREFINED_COAL, CELL_ANY_STEAM_OR_ICE, 0, CELL_WATER, true,
    CONVERTER_MATERIAL, 0,                               CELL_WATER,            0, CELL_ICE,   false,

    CONVERTER_MATERIAL, CELL_ANY_COAL,       CELL_DIRT,             0,         CELL_STONE,     false,
    CONVERTER_MATERIAL, CELL_ANY_COAL,       CELL_ANY_WATER_OR_ICE, 0,         CELL_STEAM,     true,

    CONVERTER_MATERIAL, CELL_LAVA, CELL_ANY_COAL, 0,            CELL_BASALT,  false,
    CONVERTER_MATERIAL, CELL_LAVA, CELL_QUARTZ,   CELL_MARBLE,  CELL_GRANITE, false,
    CONVERTER_MATERIAL, CELL_LAVA, CELL_BASALT,   CELL_GRANITE, CELL_DIAMOND, false,
};

#define CONVERSIONS_GUI_COUNT (sizeof(conversions)/(6*sizeof(int)))

bool can_conversions_gui_be_active(void);
void converter_gui_draw(int target);
