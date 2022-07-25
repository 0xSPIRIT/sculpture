#ifndef GUI_H_
#define GUI_H_

#include <SDL2/SDL.h>

#include "converter.h"
#include "globals.h"

#define MAX_OVERLAY_LEN 128
#define MAX_OVERLAY_LINE_LEN 128

struct Overlay {
    float x, y;
    char str[MAX_OVERLAY_LEN][MAX_OVERLAY_LINE_LEN];
    int w, h;
};

struct GUI {
    int popup, popup_y, popup_y_vel, popup_h;
    SDL_Texture *popup_texture;
    struct Overlay overlay;

    struct Button *tool_buttons[TOOL_COUNT];
};

struct Button {
    int x, y, w, h;
    int index;
    SDL_Texture *texture;
    char *overlay_text;
    void (*on_pressed)();
};

extern struct GUI gui;
extern SDL_Texture *gui_texture;

void gui_init();
void gui_deinit();
void gui_tick();
void gui_draw();

void overlay_draw(struct Overlay *overlay);

void overlay_get_string(int type, int amt, char *out_str);

struct Button *button_allocate(char *image, const char *overlay_text, void (*on_pressed)(int));
void button_tick(struct Button *b);
void button_draw(struct Button *b);
void button_deallocate(struct Button *b);

void click_gui_tool_button(int type);

#endif  /* GUI_H_ */
