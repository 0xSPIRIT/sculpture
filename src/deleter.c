void deleter_init(void) {
    Deleter *deleter = &gs->deleter;
    deleter->texture = gs->textures.deleter;
    SDL_QueryTexture(deleter->texture, NULL, NULL, &deleter->w, &deleter->h);
}

void deleter_delete(void) {
    Deleter *deleter = &gs->deleter;
    
    save_state_to_next();
    
    add_item_to_inventory_slot(gs->grid[(int)deleter->x + (int)deleter->y*gs->gw].type, 1);
    emit_dust(gs->grid[(int)deleter->x + (int)deleter->y*gs->gw].type, deleter->x, deleter->y, randf(2)-1, randf(2)-1);
    set(deleter->x, deleter->y, 0, -1);
    
    objects_reevaluate();
}

void deleter_tick(void) {
    Deleter *deleter = &gs->deleter;
    Input *input = &gs->input;
    
    if (gs->is_mouse_over_any_button) return;
    
    if (!deleter->is_rotating) {
        deleter->x = (f32) input->mx;
        deleter->y = (f32) input->my;
    }
    
    deleter->is_rotating = input->keys[SDL_SCANCODE_LSHIFT] != 0;
    
    if (deleter->is_rotating) {
        f32 rmx = (f32)input->real_mx / (f32)gs->S;
        f32 rmy = (f32)(input->real_my-GUI_H) / (f32)gs->S;
        deleter->angle = 180 + 360 * atan2f(rmy - deleter->y, rmx - deleter->x) / (f32)(2*M_PI);
        
        f32 step = 22.5;
        deleter->angle /= step;
        deleter->angle = ((int)deleter->angle) * step;
    }
    
    
    if (input->mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        deleter->is_rotating = false;
    }
    
    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        deleter_delete();
    }
}

void deleter_draw(void) {
    Deleter *deleter = &gs->deleter;
    
    const SDL_Rect dst = {
        (int) deleter->x, (int) deleter->y,
        deleter->w, deleter->h
    };
    
    SDL_SetTextureAlphaMod(deleter->texture, 128);
    
    const SDL_Point center = { 0, 0 };
    SDL_RenderCopyEx(gs->renderer,
                     deleter->texture,
                     NULL,
                     &dst,
                     deleter->angle,
                     &center,
                     SDL_FLIP_NONE);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 64);
    SDL_RenderDrawPoint(gs->renderer, (int)deleter->x, (int)deleter->y);
}
