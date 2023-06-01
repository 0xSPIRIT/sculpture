static void view_update(void) {
    gs->view.x = 0;
    if (gs->input.keys[SDL_SCANCODE_A])
        gs->view.x -= Scale(120);
    if (gs->input.keys[SDL_SCANCODE_D])
        gs->view.x += Scale(120);
}