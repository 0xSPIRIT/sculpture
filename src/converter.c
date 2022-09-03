#include "converter.h"

#include <SDL2/SDL_image.h>

#include "util.h"
#include "globals.h"
#include "gui.h"
#include "placer.h"

struct Converter *converters[MAX_CONVERTERS];
struct Converter *furnace = NULL,
                 *blast_furnace = NULL,
                 *wood_processor = NULL,
                 *stonecutter = NULL,
                 *cooler = NULL;

struct Placer *get_current_placer() {
    if (current_placer == -1 || current_tool != TOOL_PLACER) return NULL;
    return placers[current_placer];
}

static bool is_holding_placer(struct Converter *converter) {
    struct Placer *placer = get_current_placer();
    if (!placer) return false;

    // If our placer pointer is not anything in the converter, we'll know
    // it has to be something different.
    for (int i = 0; i < converter->placer_count; i++) {
        if (placer == converter->placers[i]) return false;
    }
    return true;
}

static bool is_mouse_in_converter(struct Converter *converter) {
    return is_point_in_rect((SDL_Point){real_mx, real_my - GUI_H}, (SDL_Rect){converter->x, converter->y, converter->w, converter->h});
}

void all_converters_init() {
    for (int i = 0; i < CONVERTER_COUNT; i++) {
        converters[i] = converter_init(i);
    }

    furnace = converters[CONVERTER_FURNACE];
    blast_furnace = converters[CONVERTER_BLAST_FURNACE];
    wood_processor = converters[CONVERTER_WOOD_PROCESSOR];
    stonecutter = converters[CONVERTER_STONECUTTER];
    cooler = converters[CONVERTER_COOLER];
}

void all_converters_reset() {
    for (int i = 0; i < CONVERTER_COUNT; i++) {
        for (int j = 0; j < converters[i]->placer_count; j++) {
            converters[i]->placers[j] = NULL;
        }
        converters[i]->state = STATE_OFF;
    }
}

struct Converter *converter_init(int type) {
    struct Converter *converter = calloc(1, sizeof(struct Converter));

    {
        SDL_Surface *surf = IMG_Load("../res/converter_off.png");
        converter->off_w = surf->w;
        converter->off_h = surf->h;
        converter->off_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_on.png");
        converter->on_w = surf->w;
        converter->on_h = surf->h;
        converter->on_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_output.png");
        converter->output_w = surf->w;
        converter->output_h = surf->h;
        converter->working_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    converter->w = converter->on_w;
    converter->h = converter->on_h;
    converter->texture = converter->off_texture;
    converter->state = STATE_OFF;
    converter->speed = 16;

    converter->placers = (struct Placer **) calloc(converter->placer_count, sizeof(struct Placer*));
    converter->rel_placer_attachment = (SDL_Point *) calloc(converter->placer_count, sizeof(SDL_Point));

    // Note: rel_placer_attachment are in in-game coords.
    //       when drawing the placer, it knows to upscale it
    //       to real coords.
    switch (type) {
    case CONVERTER_FURNACE:
        converter->placer_count = 3;
        converter->rel_placer_attachment[PLACER_INPUT] = (SDL_Point){5, 1};
        converter->rel_placer_attachment[PLACER_OUTPUT] = (SDL_Point){10, converter->h/S - 1};
        break;
    case CONVERTER_BLAST_FURNACE:
        converter->placer_count = 3;
        converter->rel_placer_attachment[PLACER_INPUT] = (SDL_Point){0, 0};
        break;
    case CONVERTER_WOOD_PROCESSOR:
        converter->placer_count = 2;
        converter->rel_placer_attachment[PLACER_INPUT] = (SDL_Point){0, 0};
        break;
    case CONVERTER_STONECUTTER:
        converter->placer_count = 3;
        converter->rel_placer_attachment[PLACER_INPUT] = (SDL_Point){0, 0};
        break;
    case CONVERTER_COOLER:
        converter->placer_count = 2;
        converter->rel_placer_attachment[PLACER_INPUT] = (SDL_Point){0, 0};
        break;
    }

    for (int i = 0; i < converter->placer_count; i++) {
        converter->placers[i] = NULL;
    }

    switch (type) {
    case CONVERTER_FURNACE:
        converter->convert_hook = furnace_convert;
        break;
    case CONVERTER_BLAST_FURNACE:
        converter->convert_hook = blast_furnace_convert;
        break;
    case CONVERTER_STONECUTTER:
        converter->convert_hook = furnace_convert;
        break;
    }

    return converter;
}

void converters_deinit() {
    converter_deinit(furnace);
    converter_deinit(blast_furnace);
    converter_deinit(stonecutter);
}

void converter_deinit(struct Converter *converter) {
    free(converter->placers);
    free(converter->rel_placer_attachment);
    SDL_DestroyTexture(converter->texture);
}

// Affixes the placer to the converter for input or output
static void converter_attempt_attach_to_placer(struct Converter *converter) {
    struct Placer *placer = get_current_placer();
    if (!placer) return;

    if (is_holding_placer(converter) && !is_mouse_in_converter(converter)) {
        return;
    }

    // Input placer
    if (real_my-GUI_H < converter->y+converter->h/2) {
        printf("A\n"); fflush(stdout);
        if (placer && !converter->placers[PLACER_INPUT]) {
            converter->placers[PLACER_INPUT] = placer;
        } else if (is_holding_placer(converter)) {
            // Swap the holding placer and the one in the converter.
            int i = converter->placers[PLACER_INPUT]->index;
            converter->placers[PLACER_INPUT] = get_current_placer();
            current_placer = i;
        } else if (converter->placers[PLACER_INPUT]) {
            current_placer = converter->placers[PLACER_INPUT]->index;
            converter->placers[PLACER_INPUT] = NULL;
        }
    } else { // Output placer
        if (placer && !converter->placers[PLACER_OUTPUT]) {
            // Set the bottom one if the conditions are right.
            if (placer->contains_amount == 0 || placer->contains_type == 0 || placer->contains_type == converter->contains_type) {
                converter->placers[PLACER_OUTPUT] = placer;
            }
        } else if (is_holding_placer(converter) && converter->placers[PLACER_OUTPUT]) {
            // Swap 'em
            int i = converter->placers[PLACER_OUTPUT]->index;
            converter->placers[PLACER_OUTPUT] = get_current_placer();
            current_placer = i;
        } else if (converter->placers[PLACER_OUTPUT]) {
            // Otherwise we take the one from the bottom.
            current_placer = converter->placers[PLACER_OUTPUT]->index;
            converter->placers[PLACER_OUTPUT] = NULL;
        }
    }
}

void converter_tick(struct Converter *converter) {
    struct Placer *placer = get_current_placer();
    if (!placer) return;

    // Locking the placers in place which are connected to the converter.
    // the placer values must be in in-game coords, but the converter
    // is in real coordinates (without GUI_H though).

    bool set_a_placer = false;
    for (int i = 0; i < converter->placer_count; i++) {
        if (converter->placers[i]) {
            converter->placers[i]->x = converter->x/S + converter->rel_placer_attachment[i].x;
            converter->placers[i]->y = converter->y/S + converter->rel_placer_attachment[i].y;

            SDL_RendererFlip flip = get_placer_flip(converter, i);
            switch (flip) {
            case SDL_FLIP_VERTICAL:
                converter->placers[i]->y += converter->placers[i]->h;
                break;
            default:
                break;
            }

            set_a_placer = true;
        }
    }

    if (!set_a_placer && placer) {
        placer->x = mx;
        placer->y = my;
    }

    // Checking if the mouse is over input placer, output placer, or the converter, and
    // setting the overlay accordingly.
    if (converter->placers[PLACER_INPUT] && is_mouse_in_placer(converter->placers[PLACER_INPUT])) {
        struct Placer *p = converter->placers[PLACER_INPUT];

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);

        strcpy(gui.overlay.str[0], "Input");
        strcpy(gui.overlay.str[1], string);

        /* SDL_ShowCursor(0); */
    } else if (converter->placers[PLACER_OUTPUT] && is_mouse_in_placer(converter->placers[PLACER_OUTPUT])) {
        struct Placer *p = converter->placers[PLACER_OUTPUT];

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);

        strcpy(gui.overlay.str[0], "Output");
        strcpy(gui.overlay.str[1], string);
        /* SDL_ShowCursor(0); */
    } else if (is_mouse_in_converter(converter)) {
        char string[256] = {0};

        overlay_get_string(converter->contains_type, converter->contains_amount, string);

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        strcpy(gui.overlay.str[0], "Converter");
        strcpy(gui.overlay.str[1], string);
        /* SDL_ShowCursor(0); */
    } else { // We don't have anything to show in the overlay.
        gui.overlay.x = -1;
        gui.overlay.y = -1;
        /* SDL_ShowCursor(1); */
    }
    
    if (is_holding_placer(converter) && placer) {
        // We have a placer that we're holding that is not on the top / bottom.
        placer_tick(placer);
    }
    
    // Assigns the placers to the converter.
    if (mouse_pressed[SDL_BUTTON_LEFT] && current_tool == TOOL_PLACER)
        converter_attempt_attach_to_placer(converter);

    // TODO: Bounds check.
    if (mouse_pressed[SDL_BUTTON_RIGHT]) {
        if (converter->state == STATE_OFF && converter->placers[PLACER_INPUT]) {
            converter->state = STATE_ON;
        } else {
            converter->state = STATE_OFF;
        }
    }

    if (mouse_pressed[SDL_BUTTON_MIDDLE]) {
        if (converter->state != STATE_OUT && converter->contains_amount > 0 && converter->placers[PLACER_OUTPUT] && (converter->placers[PLACER_OUTPUT]->contains_type == converter->contains_type || converter->placers[PLACER_OUTPUT]->contains_amount == 0)) {
            converter->state = STATE_OUT;
            converter->placers[PLACER_OUTPUT]->contains_type = converter->contains_type;
        } else {
            converter->state = STATE_OFF;
	}
    }    

    struct Placer *input = converter->placers[PLACER_INPUT];
    struct Placer *output = converter->placers[PLACER_OUTPUT];
    
    if (converter->state == STATE_ON && !input) {
        converter->state = STATE_OFF;
    }
    
    float amount_add = converter->speed;

    // Actual conversion.

    if (converter->state == STATE_ON && input->contains_amount > 0) {
        // Convert the type.
        int output = converter->convert_hook(0, input->contains_type);

        if (converter->contains_type != output && converter->contains_amount > 0) {
            converter->state = STATE_OFF; // We cannot continue since we can't output two types.
        } else {
            converter->contains_type = output;

            // We don't want to take out more than we have.
            if (input->contains_amount - amount_add < 0) {
                amount_add = input->contains_amount;
            }

            // Move the amount from the input placer to the converter.
            converter->contains_amount += amount_add;
            input->contains_amount -= amount_add;
            if (input->contains_amount <= 0) { // Stop the converter when there's no more in the input placer.
                converter->state = STATE_OFF;
                input->contains_amount = 0;
                input->contains_type = 0;
            }
        }
    } else if (converter->state == STATE_OUT && converter->contains_amount > 0) {
        // Output the amount to the converter to the output placer.
        
        if (!output) converter->state = STATE_OFF;
        if (output->contains_type == 0) {
            output->contains_amount = 0;
            output->contains_type = converter->contains_type;
        } else if (converter->contains_type != output->contains_type) {
            converter->state = STATE_OFF;
        }
        
        // We don't want to take out more than we have.
        if (converter->contains_amount - amount_add < 0) {
            amount_add = converter->contains_amount;
        }

        converter->contains_amount -= amount_add;
        output->contains_amount += amount_add;
        
        if (converter->contains_amount <= 0) {
            converter->state = STATE_OFF;
            converter->contains_type = 0;
        }
    }
    
    // Set textures for each state.

    if (converter->state == STATE_ON) {
        converter->texture = converter->on_texture;
        converter->w = converter->on_w;
        converter->h = converter->on_h;
    } else if (converter->state == STATE_OFF) {
        converter->texture = converter->off_texture;
        converter->w = converter->off_w;
        converter->h = converter->off_h;
    } else if (converter->state == STATE_OUT) {
        converter->texture = converter->working_texture;
        converter->w = converter->output_w;
        converter->h = converter->output_h;
    }
}

void converter_draw(struct Converter *converter) {
    converter->x = 3*S;
    converter->y = gui.popup_y + 3*S;
    
    SDL_Rect rect = {
        converter->x, GUI_H + converter->y,
        converter->w, converter->h
    };

    SDL_RenderCopy(renderer, converter->texture, NULL, &rect);

    if (gui.popup && current_tool == TOOL_PLACER) {
        for (int i = 0; i < converter->placer_count; i++) {
            if (converter->placers[i]) {
                placer_draw(converter->placers[i], true, get_placer_flip(converter, i));
            }
        }
        if (is_holding_placer(converter)) {
            placer_draw(get_current_placer(), true, false);
        }
    }
}

// These functions convert an input type to an output type,
// for different converters.

// Since this is a generic function, there's an unused
// state variable. Furnaces requires no external state
// information to generate a converted type.
int furnace_convert(int unused_state, int input) {
    switch (input) {
    case CELL_SAND:   return CELL_GLASS;
    case CELL_GLASS:  return CELL_MARBLE;
    case CELL_DIRT:   return CELL_COAL;
    case CELL_ICE:    return CELL_WATER;
    case CELL_WATER:  return CELL_STEAM;
    }

    return CELL_NONE;
}

int blast_furnace_convert(int unused_state, int input) {
    switch (input) {
    case CELL_MARBLE: return CELL_QUARTZ;
    case CELL_QUARTZ: return CELL_SAND;
    case CELL_COAL:   return CELL_DIAMOND;
    case CELL_ICE:    return CELL_STEAM;
    }

    return CELL_NONE;
}

int cooler_convert(int unused_state, int input) {
    switch (input) {
    case CELL_QUARTZ: return CELL_MARBLE;
    case CELL_MARBLE: return CELL_GLASS;
    case CELL_GLASS:  return CELL_SAND;
    case CELL_STEAM:  return CELL_WATER;
    case CELL_WATER:  return CELL_ICE;
    }

    return CELL_NONE;
}

// TODO: Add more cell types for different cut stones.
int stonecutter_convert(int state, int input) {
    switch (input) {
    case CELL_COBBLESTONE:
        switch (state) {
        }
        break;
    case CELL_MARBLE:
        switch (state) {
        }
        break;
    case CELL_QUARTZ:
        switch (state) {
        }
        break;
    }

    return CELL_NONE;
}
