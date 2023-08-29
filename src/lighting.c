void apply_vignette(SDL_Color *c, int x, int y) {
    Light light = { gs->gw/2, gs->gh/3 };
    
    int dx = x - light.x;
    int dy = y - light.y;
    
    f64 length = sqrt(dx*dx + dy*dy);
    
    f64 max_length = 80;
    if (0) {
        int _dx = gs->gw - light.x;
        int _dy = 0 - light.y;
        
        max_length = sqrt(_dx*_dx + _dy*_dy);
    }
    
    f64 strength = 1 - length / max_length;
    //strength *= max_darkness;
    
    c->r *= strength;
    c->g *= strength;
    c->b *= strength;
}