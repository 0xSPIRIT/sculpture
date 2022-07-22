#include "gui.h"

#include "grid.h"
#include "globals.h"
#include "util.h"
#include "placer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct GUI gui;

void gui_init() {
    SDL_Surface *surf = IMG_Load("../res/popup.png");
    gui = (struct GUI){ .popup_y = gh, .popup_y_vel = 0, .popup_h = surf->h, .popup = 0 };
    gui.popup_texture = SDL_CreateTextureFromSurface(renderer, surf);
    gui.overlay.x = gui.overlay.y = -1;
    SDL_FreeSurface(surf);

    converter_init();
}

void gui_deinit() {
    SDL_DestroyTexture(gui.popup_texture);
}

void gui_tick() {
    static int pressed = 0;
    if (keys[SDL_SCANCODE_TAB]) {
        if (!pressed) {
            pressed = 1;
            gui.popup = !gui.popup;
            gui.popup_y_vel = 0;
            gui.overlay.x = -1;
            gui.overlay.y = -1;
            converter.placer_top = converter.placer_bottom = NULL;
            converter.state = STATE_OFF;
            // Just in case the player had reset it.
            if (current_placer == -1)
                current_placer = 0;
        }
    } else {
        pressed = 0;
    }

    if (gui.popup) {
        if (SDL_GetCursor() != grabber_cursor) {
            SDL_SetCursor(grabber_cursor);
            SDL_ShowCursor(1);
        }

        if (gui.popup_y > gh-gui.popup_h) {
            gui.popup_y_vel -= 1;
        } else {
            gui.popup_y_vel = 0;
            gui.popup_y = gh-gui.popup_h;
        }

        converter_tick();
    } else if (gui.popup_y < gh) {
        gui.popup_y_vel += 1;
    } else {
        gui.popup_y = gh;
        gui.popup_y_vel = 0;
    }

    if (!gui.popup) SDL_SetCursor(normal_cursor);

    gui.popup_y += gui.popup_y_vel;
    gui.popup_y = clamp(gui.popup_y, gh-gui.popup_h, gh);
}

void gui_draw() {
    SDL_Rect popup = {
        0, gui.popup_y,
        gw, gui.popup_h
    };
    SDL_RenderCopy(renderer, gui.popup_texture, NULL, &popup);

    converter_draw();
}

static void overlay_draw_box(struct Overlay *overlay, int w, int h) {
    SDL_Rect r = { overlay->x * S, overlay->y * S, w, h };
    SDL_SetRenderDrawColor(renderer, 12, 12, 12, 255);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &r);
}

// This happens outside of the pixel-art texture, so we must
// multiply all values by scale.
void overlay_draw(struct Overlay *overlay) {
    if (overlay->x == -1 && overlay->y == -1) return;

    const int margin = 8; // In real pixels.

    SDL_Surface *surfaces[MAX_OVERLAY_LINE_LEN];
    SDL_Texture *textures[MAX_OVERLAY_LINE_LEN];
    SDL_Rect dsts[MAX_OVERLAY_LINE_LEN];
    int count = 0;

    int highest_w = 0;
    int height = 0;

    for (int i = 0; i < MAX_OVERLAY_LINE_LEN; i++) {
        if (!strlen(overlay->str[i])) continue;
        count++;
        
        SDL_Color color;

        if (i == 0)
            color = (SDL_Color){200,200,200,255};
        else
            color = (SDL_Color){255,255,255,255};

        surfaces[i] = TTF_RenderText_Solid(font, overlay->str[i], color);
        textures[i] = SDL_CreateTextureFromSurface(renderer, surfaces[i]);
        dsts[i] = (SDL_Rect){ S * overlay->x + margin, height + S * overlay->y + margin, surfaces[i]->w, surfaces[i]->h };

        height += surfaces[i]->h;

        if (surfaces[i]->w > highest_w) highest_w = surfaces[i]->w;
    }

    overlay_draw_box(overlay, highest_w + margin*2, height + margin*2);

    for (int i = 0; i < count; i++) {
        SDL_RenderCopy(renderer, textures[i], NULL, &dsts[i]);
    }

    for (int i = 0; i < count; i++) {
        SDL_FreeSurface(surfaces[i]);
        SDL_DestroyTexture(textures[i]);
    }
}

void overlay_get_string(int type, int amt, char *out_str) {
    gui.overlay = (struct Overlay){
        (float)real_mx/S, (float)real_my/S
    };
    char name[256] = {0};
    if (amt == 0) {
        strcpy(out_str, "Contains nothing");
        return;
    }
    get_name_from_type(type, name);
    sprintf(out_str, "Contains %s", name);
    if (amt) {
        char s[256] = {0};
        sprintf(s, " amt: %d", amt);
        strcat(out_str, s);
    }
}
