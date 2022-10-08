#include "gui.h"

#include "grid.h"

#include "util.h"
#include "placer.h"
#include "chisel.h"
#include "chisel_blocker.h"
#include "cursor.h"
#include "game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void gui_init() {
    struct GUI *gui = &gs->gui;

    *gui = (struct GUI){ .popup_y = (f32) (gs->gh*gs->S), .popup_y_vel = 0, .popup_h = GUI_POPUP_H, .popup = 0 };
    gui->popup_texture = gs->textures.popup;

    tooltip_reset(&gui->tooltip);
    gui->is_placer_active = false;

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);

        gui->tool_buttons[i] = button_allocate(BUTTON_TYPE_TOOL_BAR, gs->textures.tool_buttons[i], name, click_gui_tool_button);
        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->activated = i == gs->current_tool;

        cum += gui->tool_buttons[i]->w;
    }
}

void gui_tick() {
    struct GUI *gui = &gs->gui;
    struct Input *input = &gs->input;

    if (input->keys_pressed[SDL_SCANCODE_TAB]) {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        tooltip_reset(&gui->tooltip);

        gs->current_tool = gs->previous_tool;

        // Just in case the player had reset it.
        if (gs->current_placer == -1)
            gs->current_placer = 0;
    }

    const f32 speed = 3.0f;

    all_converters_tick();

    if (gui->popup) {
        if (SDL_GetCursor() != gs->grabber_cursor) {
            /* SDL_SetCursor(gs->grabber_cursor); */
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
            tooltip_reset(&gui->tooltip);
        } else if (!was_placer_active && gui->is_placer_active) {
            struct Placer *p = converter_get_current_placer();
            p->x = input->mx;
            p->y = input->my;
        }
    } else if (gui->popup_y < gs->S*gs->gh) {
        gui->popup_y_vel += speed;
    } else {
        gui->popup_y = (f32) (gs->S*gs->gh);
        gui->popup_y_vel = 0;
    }

    if (!gui->popup) {
        /* SDL_SetCursor(normal_cursor); */
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui->tool_buttons[i], &i);
        }
        if (input->real_my >= GUI_H) {
            tooltip_reset(&gui->tooltip);
        }
    }

    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = (f32) clamp((int) gui->popup_y, (int) (gs->S*gs->gh - gui->popup_h), gs->window_height);
}

void gui_draw() {
    struct GUI *gui = &gs->gui;

    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR));

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
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR), NULL, &dst);
    SDL_SetRenderTarget(gs->renderer, old);
}

void gui_popup_draw() {
    struct GUI *gui = &gs->gui;

    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)gui->popup_h
    };

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gs->renderer, &popup);

    all_converters_draw();
}

internal void tooltip_draw_box(struct Tooltip *tooltip, int w, int h) {
    tooltip->w = w;
    tooltip->h = h;

    SDL_Rect r = {
        (int) (tooltip->x * gs->S),
        (int) (tooltip->y * gs->S + GUI_H),
        w,
        h
    };

    SDL_SetRenderDrawColor(gs->renderer, 12, 12, 12, 255);
    SDL_RenderFillRect(gs->renderer, &r);
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(gs->renderer, &r);
}

void tooltip_reset(struct Tooltip *tooltip) {
    memset(tooltip, 0, sizeof(struct Tooltip));
    tooltip->x = tooltip->y = -1;
}

void tooltip_set_position_to_cursor(struct Tooltip *tooltip, int type) {
    struct Input *input = &gs->input;
    tooltip->x = (f32)input->real_mx/gs->S;
    tooltip->y = (f32)input->real_my/gs->S - GUI_H/gs->S;
    tooltip->type = type;
}

void tooltip_set_position(struct Tooltip *tooltip, int x, int y, int type) {
    tooltip->x = (f32) x;
    tooltip->y = (f32) y;
    tooltip->type = type;
}

// This happens outside of the pixel-art texture, so we must
// multiply all values by scale.
void tooltip_draw(struct Tooltip *tooltip) {
    if (tooltip->x == -1 && tooltip->y == -1) return;

    const int margin = 8; // In real pixels.

    SDL_Surface *surfaces[MAX_TOOLTIP_LINE_LEN];
    SDL_Texture *textures[MAX_TOOLTIP_LINE_LEN];
    SDL_Rect dsts[MAX_TOOLTIP_LINE_LEN];
    int count = 0;

    int highest_w = 0;
    int height = 0;

    for (int i = 0; i < MAX_TOOLTIP_LINE_LEN; i++) {
        if (!strlen(tooltip->str[i])) continue;
        count++;
        
        SDL_Color color;

        if (i == 0)
            color = (SDL_Color){200,200,200,255};
        else
            color = (SDL_Color){255,255,255,255};

        // @Performance
        surfaces[i] = TTF_RenderText_Solid(gs->fonts.font, tooltip->str[i], color);
        Assert(surfaces[i]);
        textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surfaces[i]);
        Assert(textures[i]);
        dsts[i] = (SDL_Rect){
            (int) (gs->S * tooltip->x + margin),
            (int) (height + gs->S * tooltip->y + margin),
            surfaces[i]->w,
            surfaces[i]->h };
        dsts[i].y += GUI_H;

        height += surfaces[i]->h;

        if (surfaces[i]->w > highest_w) highest_w = surfaces[i]->w;
    }

    bool clamped = false;

    // Clamp the tooltips if it goes outside the gs->window.
    if (tooltip->x*gs->S + highest_w >= gs->S*gs->gw) {
        tooltip->x -= highest_w/gs->S;
        for (int i = 0; i < count; i++)
            dsts[i].x -= highest_w - margin/2;
        clamped = true;
    }

    tooltip_draw_box(tooltip, highest_w + margin*2, height + margin*2);

    if (clamped) {
        tooltip->x += highest_w;
    }

    for (int i = 0; i < count; i++) {
        SDL_RenderCopy(gs->renderer, textures[i], NULL, &dsts[i]);
    }

    for (int i = 0; i < count; i++) {
        SDL_FreeSurface(surfaces[i]);
        SDL_DestroyTexture(textures[i]);
    }
}

void tooltip_get_string(int type, int amt, char *out_str) {
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

struct Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    struct Button *b = arena_alloc(gs->persistent_memory, 1, sizeof(struct Button));
    b->type = type;
    b->texture = texture;
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);

    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

void button_tick(struct Button *b, void *data) {
    struct Input *input = &gs->input;
    struct GUI *gui = &gs->gui;

    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;

    // TODO: This is a temporary hack so that function pointers won't stop working
    //       upon reloading the DLL.
    switch (b->type) {
    case BUTTON_TYPE_CONVERTER:
        b->on_pressed = converter_begin_converting;
        break;
    case BUTTON_TYPE_TOOL_BAR:
        b->on_pressed = click_gui_tool_button;
        break;
    }

    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {

        tooltip_set_position_to_cursor(&gui->tooltip, TOOLTIP_TYPE_BUTTON);
        b->just_had_tooltip = true;

        if (strlen(b->tooltip_text))
            strcpy(gui->tooltip.str[0], b->tooltip_text);

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            b->on_pressed(data);
            b->activated = true;
        }
    } else if (b->just_had_tooltip) {
        b->just_had_tooltip = false;
        tooltip_reset(&gui->tooltip);
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

    if (gui->popup) return;

    gs->current_tool = type;
    gs->chisel_blocker_mode = 0;
    switch (gs->current_tool) {
    case TOOL_CHISEL_SMALL:
        gs->chisel = &gs->chisel_small;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 0);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+2);
        break;
    case TOOL_CHISEL_MEDIUM:
        gs->chisel = &gs->chisel_medium;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 1);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    case TOOL_CHISEL_LARGE:
        gs->current_tool = TOOL_CHISEL_LARGE;
        gs->chisel = &gs->chisel_large;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 2);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    }

    for (int i = 0; i < TOOL_COUNT; i++) {
        gui->tool_buttons[i]->activated = 0;
    }
    tooltip_reset(&gui->tooltip);
}
