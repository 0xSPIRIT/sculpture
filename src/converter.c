#include "converter.h"

#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>

#include "globals.h"

#include "grid.h"
#include "gui.h"
#include "placer.h"
#include "util.h"
#include "game.h"

struct Placer *converter_get_current_placer() {
    struct Placer *placers = gs->placers;
    if (gs->current_placer == -1 || gs->current_tool != TOOL_PLACER) return NULL;
    return &placers[gs->current_placer];
}

internal bool can_place_item_in_slot(int type, int slot) {
    bool can_put_fuel = false;

    if (slot == SLOT_FUEL) {
        switch (type) {
        case CELL_COAL: case CELL_WOOD_LOG:
            can_put_fuel = true;
            break;
        }
    }
    
    return
        slot == SLOT_INPUT1 ||
        slot == SLOT_INPUT2 ||
        can_put_fuel;
}

void item_init() {
}

void item_draw(struct Item *item, int x, int y, int w, int h) {
    if (!item->type) return;
    if (!item->amount) {
        item->type = 0;
        return;
    }

    SDL_Rect r = {
        x, y,
        w, h
    };
    SDL_RenderCopy(gs->renderer, gs->textures.items[item->type], NULL, &r);

    char number[32] = {0};
    sprintf(number, "%d", item->amount);

    SDL_Color color = (SDL_Color){255, 255, 255, 255};
    
    SDL_Surface *surf = TTF_RenderText_Solid(gs->bold_small_font, number, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);

    SDL_Rect dst = {
        x + w - surf->w - 1,
        y + h - surf->h - 1,
        surf->w,
        surf->h
    };

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(gs->renderer, &dst);
    
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);
}

// Slot may be NULL if the item doesn't belong to any slot.
// This function mostly just handles interactions with items and the mouse.
void item_tick(struct Item *item, struct Slot *slot, int x, int y, int w, int h) {
    struct Input *input = &gs->input;

    if (item == &gs->item_holding) return;
    if (!is_point_in_rect((SDL_Point){input->real_mx, input->real_my-GUI_H}, (SDL_Rect){x, y, w, h})) return;

    // From this point onwards, we know the mouse is in this item,
    // and this item is not currently being held.

    bool can_place_item = false;

    if (!gs->gui.is_placer_active) {
        can_place_item = can_place_item_in_slot(gs->item_holding.type, slot->type);
        if (!gs->item_holding.type) can_place_item = true;

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {

            // If they're the same type, just add their amounts.
            if (item->type && gs->item_holding.type == item->type) {
                // Add the amount from holding to the item.
                item->amount += gs->item_holding.amount;

                gs->item_holding.type = 0;
                gs->item_holding.amount = 0;
            }

            // Otherwise if we're either holding an item or have an item in slot,
            // swap them since they're different types.
            else if ((gs->item_holding.type || item->type) && can_place_item) {
                struct Item a = *item;

                item->type = gs->item_holding.type;
                item->amount = gs->item_holding.amount;

                gs->item_holding.type = a.type;
                gs->item_holding.amount = a.amount;

                overlay_reset(&gs->gui.overlay);
            } 

        } else if (input->mouse_pressed[SDL_BUTTON_RIGHT] && item->type && gs->item_holding.type == 0) { // If holdingd nothing
            // Split the item into two like minecraft.
            SDL_assert(gs->item_holding.amount == 0);

            const int half = item->amount/2;

            if (half) {
                item->amount -= half;
                gs->item_holding.type = item->type;
                gs->item_holding.amount += half;

                overlay_reset(&gs->gui.overlay);
            }
        }
    } else if (gs->gui.is_placer_active) {
        struct Placer *p = converter_get_current_placer();
        struct Converter *converter = NULL;

        can_place_item = can_place_item_in_slot(p->contains_type, slot->type);

        if (is_mouse_in_converter(gs->material_converter)) {
            converter = gs->material_converter;
        } else if (is_mouse_in_converter(gs->fuel_converter)) {
            converter = gs->fuel_converter;
        }

        if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
            if (p->contains_type == 0 || p->contains_type == item->type) {
                p->contains_type = item->type;
                p->contains_amount += item->amount;

                item->type = 0;
                item->amount = 0;
            }
        } else if (can_place_item && input->mouse_pressed[SDL_BUTTON_LEFT]) {
            int amt = 0;
            const int place_speed = 6;

            if (p->contains_amount && (!item->type || !item->amount)) {
                item->type = p->contains_type;
                amt = place_speed;
            } else if (item->type == p->contains_type) {
                amt = place_speed;
            }

            if (p->contains_amount - converter->speed < 0) {
                amt = p->contains_amount;
            }

            item->amount += amt;
            p->contains_amount -= amt;
        }
    }
}

void item_deinit() {
    for (int i = 0; i < CELL_COUNT; i++) {
        SDL_DestroyTexture(gs->textures.items[i]);
    }
}

void all_converters_init() {
    gs->material_converter = converter_init(CONVERTER_MATERIAL);
    gs->fuel_converter = converter_init(CONVERTER_FUEL);
}

void all_converters_deinit() {
    converter_deinit(gs->material_converter);
    converter_deinit(gs->fuel_converter);
}

void all_converters_tick() {
    if (gs->gui.popup)
        gs->current_tool = TOOL_PLACER;

    if (gs->gui.popup && gs->gui.is_placer_active) {
        placer_tick(converter_get_current_placer());
    }

    converter_tick(gs->material_converter);
    converter_tick(gs->fuel_converter);

    if (!gs->gui.is_placer_active) {
        bool set_overlay_this_frame = false;

        for (int i = 0; i < 2; i++) {
            struct Converter *c = i == 0 ? gs->material_converter : gs->fuel_converter;

            for (int j = 0; j < c->slot_count; j++) {
                if (!c->slots[j].item.type) continue;

                if (is_mouse_in_slot(&c->slots[j])) {
                    overlay_set_position_to_cursor(&gs->gui.overlay, OVERLAY_TYPE_ITEM);

                    char type[64] = {0};
                    char type_name[64] = {0};
                    char amount[64] = {0};

                    get_name_from_type(c->slots[j].item.type, type_name);
                    sprintf(type, "Type: %s", type_name);
                    sprintf(amount, "Amount: %d", c->slots[j].item.amount);

                    strcpy(gs->gui.overlay.str[0], type);
                    strcpy(gs->gui.overlay.str[1], amount);

                    set_overlay_this_frame = true;
                } else if (!set_overlay_this_frame && gs->gui.overlay.type == OVERLAY_TYPE_ITEM) {
                    overlay_reset(&gs->gui.overlay);
                }
            }
        }
    }

    struct Input *input = &gs->input;
    item_tick(&gs->item_holding, NULL, input->real_mx, input->real_my, ITEM_SIZE, ITEM_SIZE);
}

void all_converters_draw() {
    converter_draw(gs->material_converter);
    converter_draw(gs->fuel_converter);

    if (gs->gui.popup && gs->gui.is_placer_active) {
        placer_draw(converter_get_current_placer(), true);
    }

    struct Input *input = &gs->input;
    item_draw(&gs->item_holding, input->real_mx - ITEM_SIZE/2, input->real_my - ITEM_SIZE/2, ITEM_SIZE, ITEM_SIZE);
}

bool is_mouse_in_converter(struct Converter *converter) {
    struct Input *input = &gs->input;

    return is_point_in_rect((SDL_Point){input->real_mx, input->real_my-GUI_H}, (SDL_Rect){converter->x, converter->y, converter->w, converter->h});
}

bool is_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;

    return is_point_in_rect((SDL_Point){input->real_mx, input->real_my-GUI_H},
                            (SDL_Rect){
                                slot->converter->x + slot->x - slot->w/2,
                                slot->converter->y + slot->y - slot->h/2,
                                slot->w,
                                slot->h
                            });
}

bool was_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;

    return is_point_in_rect((SDL_Point){input->real_pmx,input->real_pmy-GUI_H},
                            (SDL_Rect){
                                slot->converter->x + slot->x - slot->w/2,
                                slot->converter->y + slot->y - slot->h/2,
                                slot->w,
                                slot->h
                            });
}

struct Converter *converter_init(int type) {
    struct Converter *converter = persist_allocate(1, sizeof(struct Converter));
    converter->type = type;
    converter->w = gs->window_width/2;
    converter->h = GUI_POPUP_H;

    converter->timer_max = 1;
    converter->timer_current = 0;

    switch (type) {
    case CONVERTER_MATERIAL:
        converter->slot_count = 4;
        converter->slots = persist_allocate(converter->slot_count, sizeof(struct Slot));

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
        converter->slots[SLOT_OUTPUT].y = 4.0*converter->h/5.0;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");

        converter->name = persist_allocate(CONVERTER_NAME_LEN, 1);
        strcpy(converter->name, "Material Converter");
        break;
    case CONVERTER_FUEL:
        converter->slot_count = 3;
        converter->slots = persist_allocate(converter->slot_count, sizeof(struct Slot));

        converter->slots[SLOT_INPUT1].x = converter->w/3.0;
        converter->slots[SLOT_INPUT1].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");

        converter->slots[SLOT_INPUT2].x = 2.0*converter->w/3.0;
        converter->slots[SLOT_INPUT2].y = converter->h/4.0;
        strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");

        converter->slots[SLOT_OUTPUT].x = converter->w/2.0;
        converter->slots[SLOT_OUTPUT].y = 4.0*converter->h/5.0;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");

        converter->name = persist_allocate(CONVERTER_NAME_LEN, 1);
        strcpy(converter->name, "Fuel Converter");
        break;
    }

    // Indices line up with Slot_Type enum
    // even though slot_count is variable.
    for (int i = 0; i < converter->slot_count; i++) {
        converter->slots[i].type = i;
        converter->slots[i].w = ITEM_SIZE;
        converter->slots[i].h = ITEM_SIZE;
        converter->slots[i].converter = converter;
    }

    SDL_Surface *surf = IMG_Load("../res/arrow.png");
    converter->arrow.texture = gs->textures.converter_arrow;
    converter->arrow.x = converter->w/2;
    converter->arrow.y = converter->h/2 + 24;
    converter->arrow.w = surf->w;
    converter->arrow.h = surf->h;
    converter->speed = 1;

    // Both X and Y-coordinates are updated in converter_tick.
    struct Button *b;
    b = button_allocate(gs->textures.convert_button, "Convert", converter_begin_converting);
    b->w = 48;
    b->h = 48;

    converter->go_button = b;
    
    return converter;
}

void converter_deinit(struct Converter *converter) {
    SDL_DestroyTexture(converter->arrow.texture);
    button_deallocate(converter->go_button);
    /* free(converter->name); */
    /* free(converter->slots); */
    /* free(converter); */
}

void converter_tick(struct Converter *converter) {
    switch (converter->type) {
    case CONVERTER_MATERIAL:
        converter->x = 0;
        converter->y = gs->gui.popup_y;
        break;
    case CONVERTER_FUEL:
        converter->x = gs->S*gs->gw/2;
        converter->y = gs->gui.popup_y;
        break;
    }

    converter->go_button->x = converter->x + converter->arrow.x - 128;
    converter->go_button->y = gs->gui.popup_y + converter->arrow.y + converter->arrow.h/2 + converter->go_button->h/2;

    for (int i = 0; i < converter->slot_count; i++) {
        struct Slot *s = &converter->slots[i];
        slot_tick(s);
    }

    button_tick(converter->go_button, converter);
    
    if (converter->state == CONVERTER_ON) {
        if (converter->timer_current < converter->timer_max) {
            converter->timer_current += 1;
        } else {
            converter->timer_current = 0;
            switch (converter->type) {
            case CONVERTER_MATERIAL: {
                bool did_convert = converter_convert(gs->material_converter);
                if (!did_convert) {
                    converter_set_state(gs->material_converter, CONVERTER_OFF);
                }
                break;
            }
            case CONVERTER_FUEL: {
                bool did_convert = converter_convert(gs->fuel_converter);
                if (!did_convert) {
                    converter_set_state(gs->fuel_converter, CONVERTER_OFF);
                }
                break;
            }
            }
        }
    }

    if (!gs->gui.popup) return;
}

void converter_draw(struct Converter *converter) {
    SDL_Rect converter_bounds = {
        converter->x, converter->y + GUI_H,
        converter->w, converter->h
    };

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(gs->renderer, &converter_bounds);

    for (int i = 0; i < converter->slot_count; i++) {
        slot_draw(&converter->slots[i]);
    }

    SDL_Rect arrow_dst = {
        converter->x + converter->arrow.x - converter->arrow.w / 2.0,
        converter->y + converter->arrow.y + converter->arrow.h / 2.0,
        converter->arrow.w,
        converter->arrow.h
    };

    // Flashing the arrow itself.
    if (converter->state == CONVERTER_ON) {
        const int period = 500; // Milliseconds.
        Uint8 a = (SDL_GetTicks()/period) % 2 == 0;
        a = a ? 255 : 190;
        SDL_SetTextureColorMod(converter->arrow.texture, a, a, a);
    }

    SDL_RenderCopy(gs->renderer, converter->arrow.texture, NULL, &arrow_dst);

    SDL_Surface *surf = TTF_RenderText_Blended(gs->font_courier, converter->name, (SDL_Color){0, 0, 0, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);

    int margin = 8;
    SDL_Rect r = {
        converter->x + margin,
        converter->y + margin + GUI_H,
        surf->w,
        surf->h
    };
    SDL_RenderCopy(gs->renderer, texture, NULL, &r);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);

    button_draw(converter->go_button);
}

void slot_tick(struct Slot *slot) {
    item_tick(&slot->item,
              slot,
              slot->converter->x + slot->x - slot->w/2,
              slot->converter->y + slot->y - slot->h/2,
              slot->w,
              slot->h);
}

void slot_draw(struct Slot *slot) {
    struct Converter *c = slot->converter;
    SDL_Rect bounds = {
        c->x + slot->x - slot->w/2,
        c->y + slot->y - slot->h/2 + GUI_H,
        slot->w, slot->h
    };

    int col = 200;
    SDL_SetRenderDrawColor(gs->renderer, col, col, col, 255);
    SDL_RenderFillRect(gs->renderer, &bounds);

    col = 0;
    SDL_SetRenderDrawColor(gs->renderer, col, col, col, 255);

    bounds.x--;
    bounds.y--;
    bounds.w += 2;
    bounds.h += 2;

    SDL_RenderDrawRect(gs->renderer, &bounds);

    bounds.x++;
    bounds.y++;
    bounds.w -= 2;
    bounds.h -= 2;

    if (*slot->name) {
        SDL_Surface *surf = TTF_RenderText_Blended(gs->small_font, slot->name, (SDL_Color){0, 0, 0, 255});
        SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);

        SDL_Rect dst = {
            bounds.x + slot->w/2 - surf->w/2,
            bounds.y - surf->h - 2,
            surf->w,
            surf->h
        }; 
        SDL_RenderCopy(gs->renderer, texture, NULL, &dst);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surf);
    }

    item_draw(&slot->item, bounds.x, bounds.y, bounds.w, bounds.h);
}

bool converter_is_layout_valid(struct Converter *converter) {
    bool is_empty = true;
    for (int i = 0; i < converter->slot_count; i++) {
        if (converter->slots[i].item.type) {
            is_empty = false;
            break;
        }
    }

    if (is_empty) return false;

    return true;
}

void converter_begin_converting(void *converter_ptr) {
    struct Converter *converter = (struct Converter *) converter_ptr;

    if (!converter_is_layout_valid(converter))
        return;

    converter_set_state(converter, converter->state == CONVERTER_ON ? CONVERTER_OFF : CONVERTER_ON);
}

void converter_set_state(struct Converter *converter, enum Converter_State state) {
    converter->state = state;
    if (state == CONVERTER_OFF) {
        converter->state = CONVERTER_OFF;
        SDL_SetTextureColorMod(converter->arrow.texture, 255, 255, 255);
    }
}

// Returns if the conversion went succesfully.
bool converter_convert(struct Converter *converter) {
    bool did_convert = false;

    struct Item *input1 = &converter->slots[SLOT_INPUT1].item;
    struct Item *input2 = &converter->slots[SLOT_INPUT2].item;
    struct Item *output = &converter->slots[SLOT_OUTPUT].item;
    struct Item *fuel = NULL;

    if (converter->type == CONVERTER_MATERIAL) {
        fuel = &converter->slots[SLOT_FUEL].item;
    }

    int number_inputs = (input1->type != 0) + (input2->type != 0);

    if (!number_inputs) return false;
    if (fuel && !fuel->type && fuel->amount == 0) return false;

    int output_type = 0;
    int output_ratio = 0;

    struct Item *input = NULL;

    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
    } else if (number_inputs == 2) {
        input = input1;
    }

    if (converter->type == CONVERTER_FUEL) {
        switch (input->type) {
        case CELL_COBBLESTONE: case CELL_MARBLE: case CELL_DIRT: case CELL_SAND: case CELL_WOOD_PLANK:
            output_type = CELL_WOOD_LOG;
            break;
        case CELL_GLASS: case CELL_WOOD_LOG: case CELL_QUARTZ:
            output_type = CELL_COAL;
            break;
        }
    } else if (converter->type == CONVERTER_MATERIAL) {
        switch (input->type) {
        case CELL_SAND:
            output_type = CELL_GLASS;
            break;
        }
    }

    int number_unique_inputs = 0;

    if (number_inputs == 1) {
        number_unique_inputs = 1;
    } else if (number_inputs == 2) {
        if (input1->type != input2->type) {
            number_unique_inputs = 2;
        } else {
            number_unique_inputs = 1;
        }
    }

    output_ratio = number_unique_inputs * number_unique_inputs;

    if (!output_type) return false;

    bool final_conversion = false;
    if (output_type == output->type || output->type == 0) {
        output->type = output_type;

        int amt = converter->speed * output_ratio;

        // Check if any input, when reduced by the amount,
        // gives negative amount. If so, lock the amount
        // we're reducing to by the correct amount.
        if (input1->type && input1->amount-amt < 0) {
            amt = input1->amount;
            final_conversion = true;
        }
        if (input2->type && input2->amount-amt < 0) {
            amt = input2->amount;
            final_conversion = true;
        }

        // The same for fuel, but make sure to multiple by the
        // number of inputs to compensate otherwise you'd be able
        // to use half the amount of fuel for the same output if
        // you just split the input in two.
        if (fuel && fuel->type && fuel->amount-amt < 0) {
            amt = fuel->amount * number_inputs; // *ing to compensate.
            final_conversion = true;
        }

        int amt_add = 0;
        if (input1->type) {
            input1->amount -= amt;
            amt_add += amt;
        }
        if (input2->type) {
            input2->amount -= amt;
            amt_add += amt;
        }

        if (fuel && fuel->type)
            fuel->amount -= amt_add;
        output->amount += amt_add;

        did_convert = true;
    }

    return !final_conversion || !did_convert;
}
