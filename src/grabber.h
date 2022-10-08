struct Grabber {
    f32 x, y;
    int w, h;
    SDL_Texture *texture;

    int cell_holding_id;
    int object_holding;
};
