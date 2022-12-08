struct View {
    f64 x, y, w, h;
};

f64 lerp64(f64 a, f64 b, f64 t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
}

void view_tick(struct View *view, struct Input *input) {
    float desired_x = 0;
    
    if (input->keys[SDL_SCANCODE_D]) {
        desired_x = 100;
    } else if (input->keys[SDL_SCANCODE_A]) {
        desired_x = -100;
    }
    
    float desired_y = 0;
    
    if (input->keys[SDL_SCANCODE_S]) {
        desired_y = 100;
    } else if (input->keys[SDL_SCANCODE_W]) {
        desired_y = -100;
    }
    
    view->x = lerp64(view->x, desired_x, 0.35);
    view->y = lerp64(view->y, desired_y, 0.35);
}