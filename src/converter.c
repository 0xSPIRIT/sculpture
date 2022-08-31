#include "converter.h"

#include <SDL2/SDL_image.h>

#include "util.h"
#include "globals.h"
#include "gui.h"
#include "placer.h"

struct Converter converter;

static int converter_can_overheat() {
    return (converter.heat_state == HEAT_HOT || converter.heat_state == HEAT_OVERHEATING || converter.heat_state == HEAT_TOO_HOT) && converter.placer_top && converter.placer_top->contains_type == CELL_COAL;
}

static struct Placer *get_current_placer() {
    if (current_placer == -1 || current_tool != TOOL_PLACER) return NULL;
    return placers[current_placer];
}

static int is_holding_placer() {
    struct Placer *placer = get_current_placer();
    if (!placer) return 0;
    return placer != converter.placer_top && placer != converter.placer_bottom;
}

void converter_init() {
    {
        SDL_Surface *surf = IMG_Load("../res/converter_off.png");
        converter.off_w = surf->w;
        converter.off_h = surf->h;
        converter.off_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_on.png");
        converter.on_w = surf->w;
        converter.on_h = surf->h;
        converter.on_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_output.png");
        converter.output_w = surf->w;
        converter.output_h = surf->h;
        converter.output_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    converter.w = converter.on_w;
    converter.h = converter.on_h;
    converter.texture = converter.off_texture;
    converter.state = STATE_OFF;
    converter.placer_top = converter.placer_bottom = NULL;
    converter.heat_time_max = 240; // Frames
    converter.heat_state = HEAT_HOT;
    converter.cooldown_time_max = 160;
    converter.cooldown_time_current = -1;
    converter.spd = 16;
}

void converter_deinit() {
    SDL_DestroyTexture(converter.texture);
}

// Affixes the placer to the converter for input or output
static void converter_attempt_attach_to_placer() {
    struct Placer *placer = get_current_placer();
    const int give = 5; // px of leeway

    // If we're still holding a placer, but clicking outside...
    if (is_holding_placer(converter) && !is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.x-give, gui.popup_y, converter.w+give, gh-gui.popup_y})) {
        current_placer = -1;
        return;
    }

    // Top placer
    if (my < converter.y+converter.h/2) {
        if (placer && !converter.placer_top) {
            converter.placer_top = placer;
        } else if (is_holding_placer(converter)) {
            // Swap the holding placer and the one in the converter.
            int i = converter.placer_top->index;
            converter.placer_top = get_current_placer();
            current_placer = i;
        } else if (converter.placer_top) {
            current_placer = converter.placer_top->index;
            converter.placer_top = NULL;
        }
    } else { // Bottom placer
        if (placer && !converter.placer_bottom) {
            // Set the bottom one if the conditions are right.
            if (placer->contains_amount == 0 || placer->contains_type == 0 || placer->contains_type == converter.contains_type) {
                converter.placer_bottom = placer;
            }
        } else if (is_holding_placer(converter) && converter.placer_bottom) {
            // Swap 'em
            int i = converter.placer_bottom->index;
            converter.placer_bottom = get_current_placer();
            current_placer = i;
        } else if (converter.placer_bottom) {
            // Otherwise we take the one from the bottom.
            current_placer = converter.placer_bottom->index;
            converter.placer_bottom = NULL;
        }
    }
}

void converter_tick() {
    struct Placer *placer = get_current_placer();
    
    // Locking the placers in place which are connected to the converter.
    if (converter.placer_top) {
        converter.placer_top->x = 15;
        converter.placer_top->y = converter.y + 1;
    }
    if (converter.placer_bottom) {
        converter.placer_bottom->x = 20;
        converter.placer_bottom->y = converter.y + converter.h + converter.placer_bottom->h - 1;
    } else if (!converter.placer_top && placer) {
        placer->x = mx;
        placer->y = my;
    }
    
    // Checking if the mouse is over input placer, output placer, or the converter, and
    // setting the overlay accordingly.
    if (converter.placer_top &&
        is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.placer_top->x-converter.placer_top->w/2, converter.placer_top->y-converter.placer_top->h, converter.placer_top->w, converter.placer_top->h})) {
        struct Placer *p = converter.placer_top;

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);

        strcpy(gui.overlay.str[0], "Input");
        strcpy(gui.overlay.str[1], string);
        /* SDL_ShowCursor(0); */
    } else if (converter.placer_bottom && is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.placer_bottom->x-converter.placer_bottom->w/2, converter.placer_bottom->y-converter.placer_bottom->h, converter.placer_bottom->w, converter.placer_bottom->h})) {
        struct Placer *p = converter.placer_bottom;

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);

        strcpy(gui.overlay.str[0], "Output");
        strcpy(gui.overlay.str[1], string);
        /* SDL_ShowCursor(0); */
    } else if (is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.x, converter.y, converter.w, converter.w})) {
        char string[256] = {0};
        char str_b[256] = {0};

        overlay_get_string(converter.contains_type, converter.contains_amount, string);

        overlay_reset(&gui.overlay);
        overlay_set_position(&gui.overlay);

        strcpy(gui.overlay.str[0], "Converter");

        switch (converter.heat_state) {
        case HEAT_TOO_HOT:
            sprintf(str_b, "Mode: [TOO HOT] %.2f seconds left", (float)converter.cooldown_time_current/60.0);
            strcpy(gui.overlay.str[1], str_b);
            break;
        case HEAT_OVERHEATING:;
            sprintf(str_b, "Mode: [OVERHEATING] %.2f seconds left", (float)converter.cooldown_time_current/60.0);
            strcpy(gui.overlay.str[1], str_b);
            break;
        case HEAT_HOT:
            strcpy(gui.overlay.str[1], "Mode: [HOT]");
            break;
        case HEAT_COLD:
            strcpy(gui.overlay.str[1], "Mode: [COLD]");
            break;
        }
        strcpy(gui.overlay.str[2], string);
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
    
    // Code that assigns the placers to the converter.
    if (mouse_pressed[SDL_BUTTON_LEFT] && current_tool == TOOL_PLACER)
        converter_attempt_attach_to_placer();

    if (mouse_pressed[SDL_BUTTON_RIGHT]) {
        if (converter.state == STATE_OFF && converter.placer_top) {
            converter.state = STATE_ON;
        } else {
            converter.state = STATE_OFF;
        }
    }

    if (mouse_pressed[SDL_BUTTON_MIDDLE]) {
        if (converter.state != STATE_OUT && converter.contains_amount > 0 && converter.placer_bottom && (converter.placer_bottom->contains_type == converter.contains_type || converter.placer_bottom->contains_amount == 0)) {
            converter.state = STATE_OUT;
            converter.placer_bottom->contains_type = converter.contains_type;
        } else {
            converter.state = STATE_OFF;
	}
    }    

    struct Placer *top = converter.placer_top;
    struct Placer *bottom = converter.placer_bottom;
    
    if (converter.state == STATE_ON && !top) {
        converter.state = STATE_OFF;
    }
    
    float amount_add;
    if (converter.heat_state == HEAT_HOT || converter.heat_state == HEAT_COLD) {
        amount_add = converter.spd;
    } else if (converter.heat_state == HEAT_OVERHEATING) {
        amount_add = 0.25;
    }

    // Actual conversion.
    if (converter.state == STATE_ON && top->contains_amount > 0) {
        // Convert the type.
        int output = converter_convert(top->contains_type);

        if (converter.contains_type != output && converter.contains_amount > 0) {
            converter.state = STATE_OFF; // We cannot continue since we can't output two types.
        } else {
            converter.contains_type = output;

            // We don't want to take out more than we have.
            if (top->contains_amount - amount_add < 0) {
                amount_add = top->contains_amount;
            }

            // Move the amount from the top placer to the converter.
            converter.contains_amount += amount_add;
            top->contains_amount -= amount_add;
            if (top->contains_amount <= 0) { // Stop the converter when there's no more in the top placer.
                converter.state = STATE_OFF;
                top->contains_amount = 0;
                top->contains_type = 0;
            }
        }

        // Overheating code
        if (converter_can_overheat(converter)) {
            converter.heat_time_current -= 1;
            if (converter.heat_state == HEAT_HOT && converter.heat_time_current <= converter.heat_time_max/2) {
                converter.heat_state = HEAT_OVERHEATING;
                if (converter.cooldown_time_current == -1) {
                    converter.cooldown_time_current = converter.cooldown_time_max;
                }
            } else if (converter.heat_state == HEAT_OVERHEATING && converter.heat_time_current <= 0) {
                converter.heat_state = HEAT_TOO_HOT;
                converter.state = STATE_OFF;
                converter.cooldown_time_current = converter.cooldown_time_max;
            }    
        }
    } else if (converter.state == STATE_OUT && converter.contains_amount > 0) {
        // Output the amount to the converter to the bottom placer.
        
        if (!bottom) converter.state = STATE_OFF;
        if (bottom->contains_type == 0) {
            bottom->contains_amount = 0;
            bottom->contains_type = converter.contains_type;
        } else if (converter.contains_type != bottom->contains_type) {
            converter.state = STATE_OFF;
        }
        
        // We don't want to take out more than we have.
        if (converter.contains_amount - amount_add < 0) {
            amount_add = converter.contains_amount;
        }

        converter.contains_amount -= amount_add;
        bottom->contains_amount += amount_add;
        
        if (converter.contains_amount <= 0) {
            converter.state = STATE_OFF;
            converter.contains_type = 0;
        }
    }
    
    // Cooldown code.

    if (converter_can_overheat(converter) && converter.cooldown_time_current > 0) {
        converter.cooldown_time_current--;

        if (converter.cooldown_time_current == 0) {
            switch (converter.heat_state) {
            case HEAT_OVERHEATING:
                converter.heat_state = HEAT_HOT;
                converter.cooldown_time_current = -1;
                break; 
            case HEAT_TOO_HOT:
                converter.heat_state = HEAT_OVERHEATING;
                converter.cooldown_time_current = converter.cooldown_time_max;
                break; 
            }
        }
    }
    
    // Set textures for each state.
    if (converter.state == STATE_ON) {
        converter.texture = converter.on_texture;
        converter.w = converter.on_w;
        converter.h = converter.on_h;
    } else if (converter.state == STATE_OFF) {
        converter.texture = converter.off_texture;
        converter.w = converter.off_w;
        converter.h = converter.off_h;
        if (converter_can_overheat(converter))
            converter.heat_time_current = converter.heat_time_max;
    } else if (converter.state == STATE_OUT) {
        converter.texture = converter.output_texture;
        converter.w = converter.output_w;
        converter.h = converter.output_h;
    }
}

void converter_draw() {
    converter.x = 10;
    converter.y = gui.popup_y + 10;
    
    SDL_Rect rect = {
        converter.x, converter.y,
        converter.w, converter.h
    };
    
    if (converter.heat_state == HEAT_TOO_HOT) {
        SDL_SetTextureColorMod(converter.texture, 255, 0, 0);
    } else if (converter.heat_state == HEAT_OVERHEATING) {
        SDL_SetTextureColorMod(converter.texture, 255, 200, 200);
    } else if (converter.heat_state == HEAT_HOT) {
        SDL_SetTextureColorMod(converter.texture, 255, 255, 255);
    } else if (converter.heat_state == HEAT_COLD) {
        SDL_SetTextureColorMod(converter.texture, 0, 0, 255);
    }
    SDL_RenderCopy(renderer, converter.texture, NULL, &rect);
    
    if (current_tool == TOOL_PLACER) {
        if (converter.placer_top)
            placer_draw(converter.placer_top);
        if (converter.placer_bottom)
            placer_draw(converter.placer_bottom);
        
        if (get_current_placer() && get_current_placer() != converter.placer_top && get_current_placer() != converter.placer_bottom)
            placer_draw(get_current_placer());
    }
}

// Converts cell type "input" to another babsed on the converter's current
// heat level.
int converter_convert(int input) {
    if (converter.heat_state == HEAT_HOT || converter.heat_state == HEAT_OVERHEATING) { // Heat
        switch (input) {
        case CELL_SAND:   return CELL_GLASS;
        case CELL_GLASS:  return CELL_MARBLE;
        case CELL_MARBLE: return CELL_QUARTZ;
        case CELL_QUARTZ: return CELL_SAND;
            
        case CELL_DIRT:   return CELL_COAL;
        case CELL_COAL:   return CELL_DIAMOND;
            
        // TODO: Remove this from the converter and make a wood converter.
        case CELL_WOOD_LOG:   return CELL_WOOD_PLANK;
        case CELL_WOOD_PLANK: return CELL_COAL;
            
        case CELL_ICE:   return CELL_WATER;
        case CELL_WATER: return CELL_STEAM;
        }
    } else if (converter.heat_state == HEAT_COLD) { // Cold
        switch (input) {
        case CELL_QUARTZ: return CELL_MARBLE;
        case CELL_MARBLE: return CELL_GLASS;
        case CELL_GLASS:  return CELL_SAND;
        case CELL_SAND:   return CELL_DIRT;
            
        case CELL_COAL:   return CELL_DIRT;
            
        case CELL_STEAM:  return CELL_WATER;
        case CELL_WATER:  return CELL_ICE;
        }
    }
    return input;
}
