#ifndef GUI_H_
#define GUI_H_

#include "converter.h"
#include "grid.h"

#define MAX_TOOLTIP_LEN 128
#define MAX_TOOLTIP_LINE_LEN 128

#define GUI_POPUP_H 336 // window_height/2
#define GUI_H 96

#define ITEM_SIZE 48

enum Tooltip_Type {
    TOOLTIP_TYPE_OFF,    // Off state
    TOOLTIP_TYPE_BUTTON, // From GUI Buttons
    TOOLTIP_TYPE_ITEM,   // From hovering over items
    TOOLTIP_TYPE_PLACER  // From the placer
};

enum Button_Type {
    BUTTON_TYPE_TOOL_BAR,
    BUTTON_TYPE_CONVERTER
};

struct Tooltip {
    enum Tooltip_Type type;

    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;
};

struct GUI {
    int popup;
    bool is_placer_active;
    f32 popup_y, popup_y_vel, popup_h;
    SDL_Texture *popup_texture;
    struct Tooltip tooltip;

    struct Button *tool_buttons[TOOL_COUNT];
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

#endif  /* GUI_H_ */
