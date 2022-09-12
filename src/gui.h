#ifndef GUI_H_
#define GUI_H_

#include <SDL2/SDL.h>

#include "converter.h"
#include "grid.h"

#define MAX_OVERLAY_LEN 128
#define MAX_OVERLAY_LINE_LEN 128

#define GUI_POPUP_H 336 // window_height/2
#define GUI_H 96

#define ITEM_SIZE 48

enum Overlay_Type {
    OVERLAY_TYPE_OFF,    // Off state
    OVERLAY_TYPE_BUTTON, // From GUI Buttons
    OVERLAY_TYPE_ITEM,   // From hovering over items
    OVERLAY_TYPE_PLACER  // From the placer
};

struct Overlay {
    enum Overlay_Type type;

    float x, y;
    char str[MAX_OVERLAY_LEN][MAX_OVERLAY_LINE_LEN];
    int w, h;
};

struct GUI {
    int popup;
    bool is_placer_active;
    float popup_y, popup_y_vel, popup_h;
    SDL_Texture *popup_texture;
    struct Overlay overlay;

    struct Button *tool_buttons[TOOL_COUNT];
};

struct Button {
    int x, y, w, h;
    int index;
    SDL_Texture *texture;
    char overlay_text[128];
    bool activated;
    bool just_had_overlay; // Used to disable the GUI overlay when the mouse goes off me.
    void (*on_pressed)(void*);
};

void gui_init();
void gui_deinit();
void gui_tick();
void gui_draw();

void overlay_reset(struct Overlay *overlay);
void overlay_set_position_to_cursor(struct Overlay *overlay, int type);
void overlay_set_position(struct Overlay *overlay, int x, int y, int type);
void overlay_draw(struct Overlay *overlay);

void overlay_get_string(int type, int amt, char *out_str);

struct Button *button_allocate(SDL_Texture *texture, const char *overlay_text, void (*on_pressed)(void*));
void button_tick(struct Button *b, void *data);
void button_draw(struct Button *b);
void gui_popup_draw();
void button_deallocate(struct Button *b);

void click_gui_tool_button(void *type_ptr);

#endif  /* GUI_H_ */
