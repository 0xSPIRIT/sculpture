#ifndef GUI_H_
#define GUI_H_

#include <SDL2/SDL.h>

#include "converter.h"

#define MAX_OVERLAY_LEN 128
#define MAX_OVERLAY_LINE_LEN 128

struct Overlay {
    float x, y;
    char str[MAX_OVERLAY_LEN][MAX_OVERLAY_LINE_LEN];
};

struct GUI {
    int popup, popup_y, popup_y_vel, popup_h;
    SDL_Texture *popup_texture;
    struct Overlay overlay;
};

extern struct GUI gui;

void gui_init();
void gui_deinit();
void gui_tick();
void gui_draw();

void overlay_draw(struct Overlay *overlay);

void overlay_get_string(int type, int amt, char *out_str);

#endif  /* GUI_H_ */
