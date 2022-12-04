void deleter_init(void) {
    struct Deleter *deleter = &gs->deleter;
    deleter->texture = gs->textures.deleter;
    deleter->counter = 0;
    deleter->timer = 0;
    SDL_QueryTexture(deleter->texture, NULL, NULL, &deleter->w, &deleter->h);
}

void deleter_delete(void) {
    struct Deleter *deleter = &gs->deleter;
    
    save_state_to_next();
    
    add_item_to_inventory_slot(gs->grid[(int)deleter->x + (int)deleter->y*gs->gw].type, 1);
    emit_dust(gs->grid[(int)deleter->x + (int)deleter->y*gs->gw].type, deleter->x, deleter->y, randf(2)-1, randf(2)-1);
    set(deleter->x, deleter->y, 0, -1);
    
    objects_reevaluate();
}

void deleter_tick(void) {
    struct Deleter *deleter = &gs->deleter;
    struct Input *input = &gs->input;
    
    if (gs->is_mouse_over_any_button) return;
    
    deleter->x = (f32) input->mx;
    deleter->y = (f32) input->my;
    
    if (deleter->timer) {
        if (deleter->counter == 1) {
            SDL_SetTextureColorMod(deleter->texture, 0, 255, 0);
        } else {
            SDL_SetTextureColorMod(deleter->texture, 255, 0, 0);
        }
        deleter->timer--;
    } else {
        SDL_SetTextureColorMod(deleter->texture, 255, 255, 255);
    }
    
    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        deleter->timer = 3;
        
        deleter->counter++;
        if (deleter->counter == 2) {
            deleter_delete();
            deleter->counter = 0;
        }
    }
}

void deleter_draw(void) {
    struct Deleter *deleter = &gs->deleter;

    const SDL_Rect dst = {
        (int) deleter->x, (int) deleter->y,
        deleter->w, deleter->h
    };

    SDL_RenderCopy(gs->renderer, deleter->texture, NULL, &dst);
}
