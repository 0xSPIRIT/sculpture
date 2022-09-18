#include "gui.h"

#include "grid.h"

#include "util.h"
#include "placer.h"
#include "chisel.h"
#include "chisel_blocker.h"
#include "boot/cursor.h"
#include "game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void gui_init() {
    struct GUI *gui = &gs->gui;

    *gui = (struct GUI){ .popup_y = gs->gh*gs->S, .popup_y_vel = 0, .popup_h = GUI_POPUP_H, .popup = 0 };
    gui->popup_texture = gs->textures.popup;

    overlay_reset(&gui->overlay);
    gui->is_placer_active = false;

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);

        gui->tool_buttons[i] = button_allocate(gs->textures.tool_buttons[i], name, click_gui_tool_button);
        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->activated = i == gs->current_tool;

        cum += gui->tool_buttons[i]->w;
    }

    /* all_converters_init(); */
}

/* void gui_deinit() { */
/*     struct GUI *gui = &gs->gui; */

/*     for (int i = 0; i < 5; i++) { */
/*         button_deallocate(gui->tool_buttons[i]); */
/*     } */
/*     SDL_DestroyTexture(gui->popup_texture); */
/*     SDL_DestroyTexture(RenderTarget(gs, TARGET_GUI_TOOLBAR)); */
/* } */

void gui_tick() {
    struct GUI *gui = &gs->gui;
    struct Input *input = &gs->input;

    if (input->keys_pressed[SDL_SCANCODE_TAB]) {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        overlay_reset(&gui->overlay);
        /* all_converters_reset(); */
        // Just in case the player had reset it.
        if (gs->current_placer == -1)
            gs->current_placer = 0;
    }

    const float speed = 3.0f;

    all_converters_tick();

    if (gui->popup) {
        if (SDL_GetCursor() != gs->grabber_cursor) {
            SDL_SetCursor(gs->grabber_cursor);
            /* SDL_ShowCursor(1); */
        }

        if (gui->popup_y > gs->S*gs->gh-gui->popup_h) {
            gui->popup_y_vel -= speed;
        } else {
            gui->popup_y_vel = 0;
            gui->popup_y = gs->S*gs->gh-gui->popup_h;
        }

        int was_placer_active = gui->is_placer_active;

        gui->is_placer_active = input->keys[SDL_SCANCODE_LCTRL];

        if (was_placer_active && !gui->is_placer_active) {
            overlay_reset(&gui->overlay);
        } else if (!was_placer_active && gui->is_placer_active) {
            struct Placer *p = converter_get_current_placer();
            p->x = input->mx;
            p->y = input->my;
        }
    } else if (gui->popup_y < gs->S*gs->gh) {
        gui->popup_y_vel += speed;
    } else {
        gui->popup_y = gs->S*gs->gh;
        gui->popup_y_vel = 0;
    }

    if (!gui->popup) {
        /* SDL_SetCursor(normal_cursor); */
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui->tool_buttons[i], &i);
        }
        if (input->real_my >= GUI_H) {
            overlay_reset(&gui->overlay);
        }
    }

    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = clamp(gui->popup_y, gs->S*gs->gh - gui->popup_h, gs->window_height);
}

void gui_draw() {
    struct GUI *gui = &gs->gui;

    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);

    SDL_SetTextureBlendMode(RenderTarget(gs, TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);

    Assert(gs->window, RenderTarget(gs, TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(gs, TARGET_GUI_TOOLBAR));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    SDL_SetRenderDrawColor(gs->renderer, 64, 64, 64, 255);
    SDL_Rect r = { 0, 0, gs->gw, GUI_H/gs->S };
    SDL_RenderFillRect(gs->renderer, &r);

    for (int i = 0; i < TOOL_COUNT; i++) {
        button_draw(gui->tool_buttons[i]);
    }

    SDL_Rect dst = {
        0, 0,
        gs->gw*gs->S, GUI_H
    };

    SDL_SetRenderTarget(gs->renderer, NULL);
    SDL_RenderCopy(gs->renderer, RenderTarget(gs, TARGET_GUI_TOOLBAR), NULL, &dst);
    SDL_SetRenderTarget(gs->renderer, old);
}

void gui_popup_draw() {
    struct GUI *gui = &gs->gui;

    SDL_Rect popup = {
        0, GUI_H + gui->popup_y,
        gs->gw*gs->S, gui->popup_h
    };

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gs->renderer, &popup);

    all_converters_draw();
}

internal void overlay_draw_box(struct Overlay *overlay, int w, int h) {
    overlay->w = w;
    overlay->h = h;

    SDL_Rect r = { overlay->x * gs->S, overlay->y * gs->S + GUI_H, w, h };
    SDL_SetRenderDrawColor(gs->renderer, 12, 12, 12, 255);
    SDL_RenderFillRect(gs->renderer, &r);
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(gs->renderer, &r);
}

void overlay_reset(struct Overlay *overlay) {
    memset(overlay, 0, sizeof(struct Overlay));
    overlay->x = overlay->y = -1;
}

void overlay_set_position_to_cursor(struct Overlay *overlay, int type) {
    struct Input *input = &gs->input;
    overlay->x = (float)input->real_mx/gs->S;
    overlay->y = (float)input->real_my/gs->S - GUI_H/gs->S;
    overlay->type = type;
}

void overlay_set_position(struct Overlay *overlay, int x, int y, int type) {
    overlay->x = x;
    overlay->y = y;
    overlay->type = type;
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

        // @Performance
        surfaces[i] = TTF_RenderText_Solid(gs->fonts.font, overlay->str[i], color);
        Assert(gs->window, surfaces[i]);
        textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surfaces[i]);
        Assert(gs->window, textures[i]);
        dsts[i] = (SDL_Rect){ gs->S * overlay->x + margin, height + gs->S * overlay->y + margin, surfaces[i]->w, surfaces[i]->h };
        dsts[i].y += GUI_H;

        height += surfaces[i]->h;

        if (surfaces[i]->w > highest_w) highest_w = surfaces[i]->w;
    }

    bool clamped = false;

    // Clamp the overlays if it goes outside the gs->window.
    if (overlay->x*gs->S + highest_w >= gs->S*gs->gw) {
        overlay->x -= highest_w/gs->S;
        for (int i = 0; i < count; i++)
            dsts[i].x -= highest_w;
        clamped = true;
    }

    overlay_draw_box(overlay, highest_w + margin*2, height + margin*2);

    if (clamped) {
        overlay->x += highest_w;
    }

    for (int i = 0; i < count; i++) {
        SDL_RenderCopy(gs->renderer, textures[i], NULL, &dsts[i]);
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

struct Button *button_allocate(SDL_Texture *texture, const char *overlay_text, void (*on_pressed)(void*)) {
    struct Button *b = persist_alloc(gs->memory, 1, sizeof(struct Button));
    b->texture = texture;
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);

    strcpy(b->overlay_text, overlay_text);
    b->on_pressed = on_pressed;
    return b;
}

void button_tick(struct Button *b, void *data) {
    struct Input *input = &gs->input;
    struct GUI *gui = &gs->gui;

    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;

    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {

        overlay_set_position_to_cursor(&gui->overlay, OVERLAY_TYPE_BUTTON);
        b->just_had_overlay = true;

        if (strlen(b->overlay_text))
            strcpy(gui->overlay.str[0], b->overlay_text);

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            b->on_pressed(data);
            b->activated = true;
        }
    } else if (b->just_had_overlay) {
        b->just_had_overlay = false;
        overlay_reset(&gui->overlay);
    }

    if (!(input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        b->activated = false;
    }
}

void button_draw(struct Button *b) {
    struct Input *input = &gs->input;

    int gui_input_mx = input->real_mx;// / gs->S;
    int gui_input_my = input->real_my;// / gs->S;

    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };
    if (b->activated) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
               gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        SDL_SetTextureColorMod(b->texture, 230, 230, 230);
    } else {
        SDL_SetTextureColorMod(b->texture, 255, 255, 255);
    }
    SDL_RenderCopy(gs->renderer, b->texture, NULL, &dst);
}

void click_gui_tool_button(void *type_ptr) {
    int type = *((int*)type_ptr);
    
    struct GUI *gui = &gs->gui;

    gs->current_tool = type;
    gs->chisel_blocker_mode = 0;
    switch (gs->current_tool) {
    case TOOL_CHISEL_SMALL:
        gs->chisel = &gs->chisel_small;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 0);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = gs->chisel->w+2;
        break;
    case TOOL_CHISEL_MEDIUM:
        gs->chisel = &gs->chisel_medium;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 1);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = gs->chisel->w+4;
        break;
    case TOOL_CHISEL_LARGE:
        gs->current_tool = TOOL_CHISEL_LARGE;
        gs->chisel = &gs->chisel_large;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 2);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = gs->chisel->w+4;
        break;
    }

    for (int i = 0; i < TOOL_COUNT; i++) {
        gui->tool_buttons[i]->activated = 0;
    }
    overlay_reset(&gui->overlay);
}
