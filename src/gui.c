#include "gui.h"

#include "grid.h"
#include "globals.h"
#include "util.h"
#include "placer.h"
#include "chisel.h"
#include "chisel_blocker.h"
#include "cursor.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct GUI gui;
SDL_Texture *gui_texture;

void gui_init() {
    SDL_Surface *surf = IMG_Load("../res/popup.png");
    gui = (struct GUI){ .popup_y = gh*S, .popup_y_vel = 0, .popup_h = GUI_POPUP_H, .popup = 0 };
    gui.popup_texture = SDL_CreateTextureFromSurface(renderer, surf);
    gui.overlay.x = gui.overlay.y = -1;
    gui.is_placer_active = false;
    SDL_FreeSurface(surf);

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        char filename[128] = {0};
        char path[128] = {0};

        get_name_from_tool(i, name);
        get_file_from_tool(i, filename);

        sprintf(path, "../res/buttons/%s", filename);
        gui.tool_buttons[i] = button_allocate(path, name, click_gui_tool_button);
        gui.tool_buttons[i]->x = cum;
        gui.tool_buttons[i]->y = 0;
        gui.tool_buttons[i]->index = i;
        gui.tool_buttons[i]->activated = i == current_tool;

        cum += gui.tool_buttons[i]->w;
    }

    gui_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw*S, GUI_H);

    /* all_converters_init(); */
}

void gui_deinit() {
    for (int i = 0; i < 5; i++) {
        button_deallocate(gui.tool_buttons[i]);
    }
    SDL_DestroyTexture(gui.popup_texture);
    SDL_DestroyTexture(gui_texture);
}

void gui_tick() {
    if (keys_pressed[SDL_SCANCODE_TAB]) {
        gui.popup = !gui.popup;
        gui.popup_y_vel = 0;
        gui.overlay.x = -1;
        gui.overlay.y = -1;
        /* all_converters_reset(); */
        // Just in case the player had reset it.
        if (current_placer == -1)
            current_placer = 0;
    }

    const float speed = 3.0f;

    all_converters_tick();

    if (gui.popup) {
        if (SDL_GetCursor() != grabber_cursor) {
            set_cursor(grabber_cursor);
            /* SDL_ShowCursor(1); */
        }

        if (gui.popup_y > S*gh-gui.popup_h) {
            gui.popup_y_vel -= speed;
        } else {
            gui.popup_y_vel = 0;
            gui.popup_y = S*gh-gui.popup_h;
        }

        int was_placer_active = gui.is_placer_active;

        gui.is_placer_active = keys[SDL_SCANCODE_LSHIFT];

        if (was_placer_active && !gui.is_placer_active) {
            gui.overlay.x = -1;
            gui.overlay.y = -1;
        } else if (!was_placer_active && gui.is_placer_active) {
            struct Placer *p = converter_get_current_placer();
            p->x = mx;
            p->y = my;
        }
    } else if (gui.popup_y < S*gh) {
        gui.popup_y_vel += speed;
    } else {
        gui.popup_y = S*gh;
        gui.popup_y_vel = 0;
    }

    if (!gui.popup) {
        /* set_cursor(normal_cursor); */
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui.tool_buttons[i]);
        }
        if (real_my >= GUI_H && gui.overlay.is_gui) {
            gui.overlay.x = gui.overlay.y = -1;
            gui.overlay.is_gui = 0;
        }
    }

    gui.popup_y += gui.popup_y_vel;
    gui.popup_y = clamp(gui.popup_y, S*gh - gui.popup_h, window_height);
}

void gui_draw() {
    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(renderer);

    SDL_SetTextureBlendMode(gui_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, gui_texture);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_Rect r = { 0, 0, gw, GUI_H/S };
    SDL_RenderFillRect(renderer, &r);

    for (int i = 0; i < TOOL_COUNT; i++) {
        button_draw(gui.tool_buttons[i]);
    }

    SDL_Rect dst = {
        0, 0,
        gw*S, GUI_H
    };

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, gui_texture, NULL, &dst);
    SDL_SetRenderTarget(renderer, old);
}

void gui_popup_draw() {
    SDL_Rect popup = {
        0, GUI_H + gui.popup_y,
        gw*S, gui.popup_h
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &popup);

    all_converters_draw();
}

static void overlay_draw_box(struct Overlay *overlay, int w, int h) {
    overlay->w = w;
    overlay->h = h;

    SDL_Rect r = { overlay->x * S, overlay->y * S + GUI_H, w, h };
    SDL_SetRenderDrawColor(renderer, 12, 12, 12, 255);
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &r);
}

void overlay_reset(struct Overlay *overlay) {
    memset(overlay, 0, sizeof(struct Overlay));
}

void overlay_set_position(struct Overlay *overlay) {
    overlay->x = (float)real_mx/S;
    overlay->y = (float)real_my/S - GUI_H/S;
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
        SDL_assert(surfaces[i]);
        textures[i] = SDL_CreateTextureFromSurface(renderer, surfaces[i]);
        SDL_assert(textures[i]);
        dsts[i] = (SDL_Rect){ S * overlay->x + margin, height + S * overlay->y + margin, surfaces[i]->w, surfaces[i]->h };
        dsts[i].y += GUI_H;

        height += surfaces[i]->h;

        if (surfaces[i]->w > highest_w) highest_w = surfaces[i]->w;
    }

    bool clamped = false;

    // Clamp the overlays if it goes outside the window.
    if (overlay->x*S + highest_w >= S*gw) {
        overlay->x -= highest_w/S;
        for (int i = 0; i < count; i++)
            dsts[i].x -= highest_w;
        clamped = true;
    }

    overlay_draw_box(overlay, highest_w + margin*2, height + margin*2);

    if (clamped) {
        overlay->x += highest_w;
    }

    for (int i = 0; i < count; i++) {
        SDL_RenderCopy(renderer, textures[i], NULL, &dsts[i]);
    }

    for (int i = 0; i < count; i++) {
        SDL_FreeSurface(surfaces[i]);
        SDL_DestroyTexture(textures[i]);
    }
}

void overlay_get_string(int type, int amt, char *out_str) {
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

struct Button *button_allocate(char *image, const char *overlay_text, void (*on_pressed)(void*)) {
    struct Button *b = calloc(1, sizeof(struct Button));
    SDL_Surface *surf = IMG_Load(image);
    b->w = surf->w;
    b->h = surf->h;
    b->texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    strcpy(b->overlay_text, overlay_text);
    b->on_pressed = on_pressed;
    return b;
}

void button_tick(struct Button *b) {
    int gui_mx = real_mx;
    int gui_my = real_my;

    if (gui_mx >= b->x && gui_mx < b->x+b->w &&
        gui_my >= b->y && gui_my < b->y+b->h) {
        gui.overlay = (struct Overlay){
            (float)real_mx/S, (float)real_my/S - GUI_H/S
        };
        gui.overlay.is_gui = 1;
        b->just_had_overlay = true;

        if (strlen(b->overlay_text))
            strcpy(gui.overlay.str[0], b->overlay_text);

        if (mouse_pressed[SDL_BUTTON_LEFT]) {
            b->on_pressed(&b->index);
            b->activated = true;
        }
    } else if (b->just_had_overlay) {
        b->just_had_overlay = false;
        gui.overlay.x = gui.overlay.y = -1;
    }

    if (!(mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        b->activated = false;
    }
}

void button_draw(struct Button *b) {
    int gui_mx = real_mx;// / S;
    int gui_my = real_my;// / S;

    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };
    if (b->activated) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (gui_mx >= b->x && gui_mx < b->x+b->w &&
               gui_my >= b->y && gui_my < b->y+b->h) {
        SDL_SetTextureColorMod(b->texture, 230, 230, 230);
    } else {
        SDL_SetTextureColorMod(b->texture, 255, 255, 255);
    }
    SDL_RenderCopy(renderer, b->texture, NULL, &dst);
}

void button_deallocate(struct Button *b) {
    SDL_DestroyTexture(b->texture);
}

void click_gui_tool_button(void *type_ptr) {
    int type = *((int*)type_ptr);

    current_tool = type;
    chisel_blocker_mode = 0;
    switch (current_tool) {
    case TOOL_CHISEL_SMALL:
        chisel = &chisel_small;
        for (int i = 0; i < object_count; i++)
            object_generate_blobs(i, 0);
        chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+2;
        break;
    case TOOL_CHISEL_MEDIUM:
        chisel = &chisel_medium;
        for (int i = 0; i < object_count; i++)
            object_generate_blobs(i, 1);
        chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+4;
        break;
    case TOOL_CHISEL_LARGE:
        current_tool = TOOL_CHISEL_LARGE;
        chisel = &chisel_large;
        for (int i = 0; i < object_count; i++)
            object_generate_blobs(i, 2);
        chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+4;
        break;
    }

    for (int i = 0; i < TOOL_COUNT; i++) {
        gui.tool_buttons[i]->activated = 0;
    }
    gui.overlay.x = gui.overlay.y = -1;
}
