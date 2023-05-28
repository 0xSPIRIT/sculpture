Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

void background_draw(Background *background) {
    int w = background->surface->w, h = background->surface->h;
    
    background->time += gs->dt;
    
    const double speed = 0.1;
    
    for (int i = 0; i < w*h; i++) {
        double r, g, b;
        
        double t = background->time;
        if (i%2==0)
            t += ((double)i/(double)(2*w*h));
        else
            t += ((double)i/(double)(1.8*w*h));
        
        t *= speed;
        
        double coeff = 0.25;
        
        r = sin(1.5*t);
        r *= r;
        r *= coeff*50;
        
        g = 2*cos(t)*sin(t);
        g *= g;
        g *= coeff*10;
            
        b = 2*cos(t);
        b *= b;
        b *= coeff*25;
        
        const int flicker = 6;
        
        int amt = rand()%(flicker*2)-flicker;
        
        r += amt;
        g += amt;
        b += amt;
        
        if (r<0) r = 0;
        if (g<0) g = 0;
        if (b<0) b = 0;
        
        Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
        set_pixel(background->surface, i%w, i/w, pixel);
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer,
                                                        background->surface);
    SDL_RenderCopy(gs->renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
}