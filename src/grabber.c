void grabber_init(void) {
    gs->grabber.texture = 0;
    gs->grabber.object_holding = -1;
}

void grabber_tick(void) {
    Grabber *grabber = &gs->grabber;
    
    if (gs->tutorial.active) return;

    f32 px = grabber->x, py = grabber->y;

    grabber->x = (f32) gs->input.mx;
    grabber->y = (f32) gs->input.my;

    if (!is_in_bounds((int)grabber->x, (int)grabber->y)) return;
    if (gs->is_mouse_over_any_button) return;
    
    if (grabber->object_holding != -1) {
        int dx = (int) (grabber->x-px);
        int dy = (int) (grabber->y-py);
        object_attempt_move(grabber->object_holding, dx, dy);
    }
    
    int gx = (int)grabber->x;
    int gy = (int)grabber->y;
    
    if (gs->overlay.show && gs->overlay.grid[gx+gy*gs->gw]) {
        tooltip_set_position_to_cursor(&gs->gui.tooltip, TOOLTIP_TYPE_ITEM);
        
        gs->overlay.current_material = gs->overlay.grid[gx+gy*gs->gw];
        
        gs->is_mouse_over_any_button = true;
        
        char tooltip_text[64];
        char type_name[64];
        
        get_name_from_type(gs->overlay.grid[gx+gy*gs->gw], type_name);
        
        if (gs->level_current == 0 || gs->level_current+1 > 7)
            sprintf(tooltip_text, "Material: %s", type_name);
        else
            sprintf(tooltip_text, "Preferred Material: %s", type_name);
        
        memset(gs->gui.tooltip.str, 0, MAX_TOOLTIP_LINE_LEN * MAX_TOOLTIP_LEN);
        
        strcpy(gs->gui.tooltip.str[0], tooltip_text);
    } else {
        gs->overlay.current_material = -1;
        tooltip_reset(&gs->gui.tooltip);
    }
    
    if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
            save_state_to_next();
        }
        // Find object at point, and attempt move to cursor pixel by pixel.
        int object = gs->grid[(int)grabber->x+(int)grabber->y*gs->gw].object;
        if (object != -1) grabber->object_holding = object;
    } else {
        grabber->object_holding = -1;
    }
}
