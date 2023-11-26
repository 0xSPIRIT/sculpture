static Light *lighting_add_light(Lighting *lighting, Light l) {
    lighting->lights[lighting->light_count++] = l;
    return &lighting->lights[lighting->light_count-1];
}

static void lighting_init(Lighting *lighting) {
    memset(lighting, 0, sizeof(Lighting));
    lighting->main_light   = lighting_add_light(lighting, (Light){ gs->gw/2, gs->gh/3, 1.0, 1.0, 80, true });
    lighting->chisel_light = lighting_add_light(lighting, (Light){ 0, 0, 0.2, 0.2, 38, false });
}

static void lighting_tick(Lighting *lighting) {
    Light *c = lighting->chisel_light;
    
    if (is_tool_chisel() && gs->chisel) {
        c->active = true;
        c->x = gs->chisel->x;
        c->y = gs->chisel->y;
    } else {
        c->active = false;
    }
}

static f32 get_light_strength_at_position(Light light, int x, int y) {
    f32 dx = x - light.x;
    f32 dy = y - light.y;

    f32 length = sqrt(dx*dx + dy*dy);

    f32 max_length = light.radius;

    f32 strength = 1 - length / max_length;
    strength *= light.strength;

    if (strength < 0) strength = 0;

    return strength;
}

static SDL_Color modify_color_based_on_light_strength(SDL_Color in, f64 strength) {
    SDL_Color result;

    int r = clamp((int)((f64)in.r * strength), 0, 255);
    int g = clamp((int)((f64)in.g * strength), 0, 255);
    int b = clamp((int)((f64)in.b * strength), 0, 255);

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = in.a;

    return result;
}

// Modifies color `c` based on the position of `light` and the x, y.
static void apply_light_to_color(SDL_Color *c, Light light, int x, int y) {
    if (!c) return;

    f64 strength = get_light_strength_at_position(light, x, y);

    *c = modify_color_based_on_light_strength(*c, strength);
}

// Applies all lights
static void apply_lighting_to_color(Lighting *lighting, SDL_Color *c, int x, int y) {
    f64 cum_strength = 0;

    for (int i = 0; i < lighting->light_count; i++) {
        if (lighting->lights[i].active)
            cum_strength += get_light_strength_at_position(lighting->lights[i], x, y);
    }

    *c = modify_color_based_on_light_strength(*c, cum_strength);
}

static void apply_lighting_to_alpha(Lighting *lighting, u8 *alpha, int x, int y) {
    f64 cum_strength = 0;

    for (int i = 0; i < lighting->light_count; i++) {
        if (lighting->lights[i].active)
            cum_strength += get_light_strength_at_position(lighting->lights[i], x, y);
    }

    *alpha = clamp((int)((f64)(*alpha) * cum_strength), 0, 255);
}

// Applies all lights to an entire render target.
static void apply_lighting_to_target(int target, Lighting *lighting) {
    int w = gs->gw*2;
    int h = gs->gh*2;

    u8 *pixels = PushArray(gs->transient_memory, 4*w*h, 1);
    RenderReadPixels(target, pixels, w*4);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = (x+y*w)*4;
            if (pixels[index+3]) {
                SDL_Color c = { pixels[index], pixels[index+1], pixels[index+2], pixels[index+3] };

                // These x and y values are offset,
                // since they're based on the actual texture
                // itself, which goes from (0, 0) to (gs->gw*2, gs->gh*2)
                // But, the actual grid space is a section through this,
                // of (64, 32) to (64+128, 32+64) which is the center of the screen.

                // So, we must modify the x and y values for the vignette calculation

                apply_lighting_to_color(lighting, &c, x-64, y-32);

                RenderColor(c.r, c.g, c.b, c.a);
                RenderPoint(target, x, y);
            }
        }
    }
}
