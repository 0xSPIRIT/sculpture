struct View {
    f64 x, y, w, h;
};

#if 0
void view_tick(struct View *view, struct Input *input) {
    f64 desired_x = 0;
    
    (void)input;
    
    if (input->keys[SDL_SCANCODE_D]) {
        desired_x = 100;
    } else if (input->keys[SDL_SCANCODE_A]) {
        desired_x = -100;
    }
    
    f64 desired_y = 0;
    
    if (input->keys[SDL_SCANCODE_S]) {
        desired_y = 100;
    } else if (input->keys[SDL_SCANCODE_W]) {
        desired_y = -100;
    }
    
    view->x = lerp64(view->x, desired_x, 0.35);
    view->y = lerp64(view->y, desired_y, 0.35);
}
#endif