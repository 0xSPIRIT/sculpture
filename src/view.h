struct View {
    int x, y, w, h;
};

void view_tick(struct View *view, struct Input *input) {
    int move_x = input->keys[SDL_SCANCODE_D] - input->keys[SDL_SCANCODE_A];
    int move_y = input->keys[SDL_SCANCODE_S] - input->keys[SDL_SCANCODE_W];
    
    const int speed = 9;
    
    move_x = 0;
    move_y = 0;
    
    view->x += move_x * speed;
    view->y += move_y * speed;
}