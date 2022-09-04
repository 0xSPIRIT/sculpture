#include "converter.h"

#include <SDL2/SDL_image.h>
#include <stdlib.h>

#include "globals.h"
#include "grid.h"
#include "gui.h"
#include "placer.h"
#include "util.h"

struct Converter *material_converter = NULL,
                 *fuel_converter = NULL;
struct Item *item_holding = NULL;

static struct Placer *get_current_placer() {
    if (current_placer == -1 || current_tool != TOOL_PLACER) return NULL;
    return placers[current_placer];
}

void item_init() {
    for (int i = 0; i < CELL_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);
        SDL_Surface *surf = IMG_Load(file);
        SDL_assert(surf);
        item_textures[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

void item_deinit() {
    for (int i = 0; i < CELL_COUNT; i++) {
        SDL_DestroyTexture(item_textures[i]);
    }
}

void all_converters_init() {
    material_converter = converter_init(CONVERTER_MATERIAL);
    fuel_converter = converter_init(CONVERTER_FUEL);
}

void all_converters_deinit() {
    converter_deinit(material_converter);
    converter_deinit(fuel_converter);
}

void all_converters_tick() {
    current_tool = TOOL_PLACER;

    if (gui.popup) {
        placer_tick(get_current_placer());
    }

    converter_tick(material_converter);
    converter_tick(fuel_converter);
}

void all_converters_draw() {
    converter_draw(material_converter);
    converter_draw(fuel_converter);

    if (gui.popup) {
        placer_draw(get_current_placer(), true);
    }
}

bool is_mouse_in_converter(struct Converter *converter) {
    return is_point_in_rect((SDL_Point){real_mx, real_my-GUI_H}, (SDL_Rect){converter->x, converter->y, converter->w, converter->h});
}

bool is_mouse_in_slot(struct Slot *slot) {
    return is_point_in_rect((SDL_Point){real_mx, real_my-GUI_H},
                            (SDL_Rect){
                                slot->converter->x + slot->x - slot->w/2,
                                slot->converter->y + slot->y - slot->h/2,
                                slot->w,
                                slot->h
                            });
}

struct Converter *converter_init(int type) {
    struct Converter *converter = calloc(1, sizeof(struct Converter));
    converter->type = type;
    converter->w = window_width/2;
    converter->h = GUI_POPUP_H;

    switch (type) {
    case CONVERTER_MATERIAL:
        converter->slot_count = 4;
        converter->slots = calloc(converter->slot_count, sizeof(struct Slot));

        converter->slots[SLOT_INPUT1].x = converter->w/3.0;
        converter->slots[SLOT_INPUT1].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");

        converter->slots[SLOT_INPUT2].x = 2.0*converter->w/3.0;
        converter->slots[SLOT_INPUT2].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");

        converter->slots[SLOT_FUEL].x = 3.0*converter->w/4.0;
        converter->slots[SLOT_FUEL].y = converter->h/2.0;
        strcpy(converter->slots[SLOT_FUEL].name, "Fuel");

        converter->slots[SLOT_OUTPUT].x = converter->w/2.0;
        converter->slots[SLOT_OUTPUT].y = 3.0*converter->h/4.0;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");

        converter->name = malloc(CONVERTER_NAME_LEN);
        strcpy(converter->name, "Material Converter");
        break;
    case CONVERTER_FUEL:
        converter->slot_count = 3;
        converter->slots = calloc(converter->slot_count, sizeof(struct Slot));

        converter->slots[SLOT_INPUT1].x = converter->w/3.0;
        converter->slots[SLOT_INPUT1].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");

        converter->slots[SLOT_INPUT2].x = 2.0*converter->w/3.0;
        converter->slots[SLOT_INPUT2].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");

        converter->slots[SLOT_OUTPUT].x = converter->w/2.0;
        converter->slots[SLOT_OUTPUT].y = 3.0*converter->h/4.0;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");

        converter->name = malloc(CONVERTER_NAME_LEN);
        strcpy(converter->name, "Fuel Converter");
        break;
    }

    for (int i = 0; i < converter->slot_count; i++) {
        converter->slots[i].w = 48;
        converter->slots[i].h = 48;
        converter->slots[i].converter = converter;
    }

    SDL_Surface *surf = IMG_Load("../res/arrow.png");
    converter->arrow.texture = SDL_CreateTextureFromSurface(renderer, surf);
    converter->arrow.x = converter->w/2;
    converter->arrow.y = converter->h/2 + 16;
    converter->arrow.w = surf->w;
    converter->arrow.h = surf->h;
    converter->speed = 6;
    
    return converter;
}

void converter_deinit(struct Converter *converter) {
    SDL_DestroyTexture(converter->arrow.texture);
    free(converter->name);
    free(converter->slots);
    free(converter);
}

void converter_tick(struct Converter *converter) {
    switch (converter->type) {
    case CONVERTER_MATERIAL:
        converter->x = 0;
        converter->y = gui.popup_y;
        break;
    case CONVERTER_FUEL:
        converter->x = S*gw/2;
        converter->y = gui.popup_y;
        break;
    }

    if (!gui.popup) return;
}

void converter_draw(struct Converter *converter) {
    SDL_Rect converter_bounds = {
        converter->x, converter->y + GUI_H,
        converter->w, converter->h
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &converter_bounds);

    for (int i = 0; i < converter->slot_count; i++) {
        slot_draw(&converter->slots[i]);
    }

    SDL_Rect arrow_dst = {
        converter->x + converter->arrow.x - converter->arrow.w / 2.0,
        converter->y + converter->arrow.y + converter->arrow.h / 2.0,
        converter->arrow.w,
        converter->arrow.h
    };
    SDL_RenderCopy(renderer, converter->arrow.texture, NULL, &arrow_dst);

    SDL_Surface *surf = TTF_RenderText_Blended(font_consolas, converter->name, (SDL_Color){0, 0, 0, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);

    int margin = 8;
    SDL_Rect r = {
        converter->x + margin,
        converter->y + margin + GUI_H,
        surf->w,
        surf->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &r);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

void slot_tick(struct Slot *slot) {
}

void slot_draw(struct Slot *slot) {
    struct Converter *c = slot->converter;
    SDL_Rect bounds = {
        c->x + slot->x - slot->w/2,
        c->y + slot->y - slot->h/2 + GUI_H,
        slot->w, slot->h
    };

    int col = 200;
    SDL_SetRenderDrawColor(renderer, col, col, col, 255);
    SDL_RenderFillRect(renderer, &bounds);

    col = 0;
    SDL_SetRenderDrawColor(renderer, col, col, col, 255);

    bounds.x--;
    bounds.y--;
    bounds.w += 2;
    bounds.h += 2;

    SDL_RenderDrawRect(renderer, &bounds);

    bounds.x++;
    bounds.y++;
    bounds.w -= 2;
    bounds.h -= 2;

    if (*slot->name) {
        SDL_Surface *surf = TTF_RenderText_Blended(small_font, slot->name, (SDL_Color){0, 0, 0, 255});
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);

        SDL_Rect dst = {
            bounds.x, bounds.y - surf->h - 2,
            surf->w, surf->h
        };
        SDL_RenderCopy(renderer, texture, NULL, &dst);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surf);
    }

    item_draw(&slot->item, bounds.x, bounds.y, bounds.w, bounds.h);
}

void item_draw(struct Item *item, int x, int y, int w, int h) {
    if (item->type) {
        SDL_Rect r = {
            x, y,
            w, h
        };
        SDL_RenderCopy(renderer, item_textures[item->type], NULL, &r);
    }

    char number[32] = {0};
    sprintf(number, "%d", item->amount);
    
    SDL_Surface *surf = TTF_RenderText_Blended(small_font, number, (SDL_Color){0, 0, 0, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_Rect dst = {
        x + w - surf->w - 1,
        y + h - surf->h - 1,
        surf->w,
        surf->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}
