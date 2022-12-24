void deleter_init(void) {
    struct Deleter *deleter = &gs->deleter;
    deleter->texture = gs->textures.deleter;
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
    
    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        deleter_delete();
    }
}

void deleter_draw(void) {
    struct Deleter *deleter = &gs->deleter;
    
    const SDL_Rect dst = {
        (int) deleter->x, (int) deleter->y,
        deleter->w, deleter->h
    };
    
    SDL_SetTextureAlphaMod(deleter->texture, 128);
    SDL_RenderCopy(gs->renderer, deleter->texture, NULL, &dst);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 64);
    SDL_RenderDrawPoint(gs->renderer, (int)deleter->x, (int)deleter->y);
}
