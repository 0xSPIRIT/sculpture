#include "gui.h"

#include "grid.h"
#include "globals.h"
#include "util.h"
#include "placer.h"
#include "chisel.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct GUI gui;
SDL_Texture *gui_texture;

void gui_init() {
    SDL_Surface *surf = IMG_Load("../res/popup.png");
    gui = (struct GUI){ .popup_y = gh, .popup_y_vel = 0, .popup_h = surf->h, .popup = 0 };
    gui.popup_texture = SDL_CreateTextureFromSurface(renderer, surf);
    gui.overlay.x = gui.overlay.y = -1;
    SDL_FreeSurface(surf);

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        char path[128] = {0};
        get_name_from_tool(i, name);
        get_path_from_tool(i, path);
        gui.tool_buttons[i] = button_allocate(path, name, click_gui_tool_button);
        gui.tool_buttons[i]->x = cum;
        gui.tool_buttons[i]->y = 0;
        gui.tool_buttons[i]->index = i;

        cum += gui.tool_buttons[i]->w;
    }

    gui_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, GUI_H/S);

    converter_init();
}

void gui_deinit() {
    for (int i = 0; i < 5; i++) {
        button_deallocate(gui.tool_buttons[i]);
    }
    SDL_DestroyTexture(gui.popup_texture);
    SDL_DestroyTexture(gui_texture);
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

    if (!gui.popup) {
        SDL_SetCursor(normal_cursor);
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui.tool_buttons[i]);
        }
    }

    gui.popup_y += gui.popup_y_vel;
    gui.popup_y = clamp(gui.popup_y, gh-gui.popup_h, gh);
}

void gui_draw() {
    {
        SDL_Texture *old = SDL_GetRenderTarget(renderer);

        SDL_SetRenderTarget(renderer, gui_texture);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

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

    SDL_Rect popup = {
        0, gui.popup_y,
        gw, gui.popup_h
    };
    SDL_RenderCopy(renderer, gui.popup_texture, NULL, &popup);

    converter_draw();
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
        dsts[i].y += GUI_H;

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

struct Button *button_allocate(char *image, const char *overlay_text, void (*on_pressed)(int)) {
    struct Button *b = calloc(1, sizeof(struct Button));
    SDL_Surface *surf = IMG_Load(image);
    printf("%s\n", image); fflush(stdout);
    b->w = surf->w;
    b->h = surf->h;
    b->texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    if (b->overlay_text) {
        b->overlay_text = calloc(strlen(overlay_text), 1);
        strcpy(b->overlay_text, overlay_text);
    }
    b->on_pressed = on_pressed;
    return b;
}

void button_tick(struct Button *b) {
    int gui_mx = real_mx / S;
    int gui_my = real_my / S;

    if (gui_mx >= b->x && gui_mx <= b->x+b->w &&
        gui_my >= b->y && gui_my <= b->y+b->h) {

        static int p = 0;
        if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (!p) {
                b->on_pressed(b->index);
                p = 1;
            }
        } else p = 0;
    }
}

void button_draw(struct Button *b) {
    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };
    SDL_RenderCopy(renderer, b->texture, NULL, &dst);
}

void button_deallocate(struct Button *b) {
    if (b->overlay_text)
        free(b->overlay_text);
    SDL_DestroyTexture(b->texture);
}

void click_gui_tool_button(int type) {
    printf("Type %d\n", type); fflush(stdout);
    current_tool = type;
    switch (current_tool) {
    case TOOL_CHISEL_SMALL:
        chisel = &chisel_small;
        for (int i = 0; i < object_count; i++)
            object_set_blobs(i, 0);
        hammer.normal_dist = hammer.dist = chisel->w+4;
        break;
    case TOOL_CHISEL_MEDIUM:
        chisel = &chisel_medium;
        for (int i = 0; i < object_count; i++)
            object_set_blobs(i, 1);
        hammer.normal_dist = hammer.dist = chisel->w+4;
        break;
    case TOOL_CHISEL_LARGE:
        current_tool = TOOL_CHISEL_LARGE;
        chisel = &chisel_large;
        for (int i = 0; i < object_count; i++)
            object_set_blobs(i, 2);
        hammer.normal_dist = hammer.dist = chisel->w+4;
        break;
    }
}
