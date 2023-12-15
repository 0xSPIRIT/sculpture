void setup_winds(Wind *wind) {
    wind->streams[0].point_count = 25;
    SDL_Point points[] = {
        { 15, 5 },
        { 14, 4 },
        { 13, 4 },
        { 12, 4 },
        { 11, 3 },
        { 10, 3 },
        { 9, 4 },
        { 8, 4 },
        { 7, 4 },
        { 6, 5 },
        { 5, 5 },
        { 4, 5 },
        { 3, 5 },
        { 2, 5 },
        { 1, 4 },
        { 0, 3 },
        { 0, 2 },
        { 0, 1 },
        { 1, 0 },
        { 2, 0 },
        { 3, 0 },
        { 4, 1 },
        { 5, 2 },
        { 4, 3 },
        { 3, 3 }
    };
    memcpy(wind->streams[0].points, points, wind->streams[0].point_count * sizeof(SDL_Point));
}

SDL_Color wind_get_pixel(SDL_Surface *surf, int x, int y) {
    SDL_Color result = {0};

    int bpp = surf->format->BytesPerPixel;
    int w = surf->w;

    if (bpp == 1) {
        u8 *pixels = (u8*)surf->pixels;
        u8 a = pixels[x+y*w];
        SDL_GetRGBA(a,
                    surf->format,
                    &result.r,
                    &result.g,
                    &result.b,
                    &result.a);
    } else if (bpp == 4) {
        u32 *pixels = (u32*)surf->pixels;
        u32 a = pixels[x+y*w];
        SDL_GetRGBA(a,
                    surf->format,
                    &result.r,
                    &result.g,
                    &result.b,
                    &result.a);
    }

    return result;
}

Wind_Stream load_wind_stream(const char *fp) {
    Wind_Stream stream = {0};

    SDL_Surface *surf = IMG_Load(fp);

    int w = surf->w;
    int h = surf->h;

    bool found = false;
    do {
        for (int i = 0; i < w*h; i++) {
            SDL_Color c = wind_get_pixel(surf, i%w, i/w);
            if (c.a == 255 && c.r == stream.point_count) {
                stream.points[stream.point_count++] = (SDL_Point){i%w, i/w};
                found = true;
            }
        }
    } while (found);

    SDL_FreeSurface(surf);

    return stream;
}

void wind_stream_activate(Wind *wind) {
    Wind_Stream *stream = &wind->streams[0];
    wind->instances[wind->instance_count++] = (Wind_Stream_Instance){
        .stream = stream,
        .range_start = 0,
        .range_start = 0,
        .state = WIND_STREAM_GROW,
        .active = true,
        .x = rand()%(gs->gw-16),
        .y = rand()%(gs->gh-8),
        .speed = 0
    };
}

void wind_streams_draw(int target, Wind *wind) {
    if (gs->should_update) {
        if (!gs->paused || gs->step_one) {
            if (wind->timer == 0) {
                wind_stream_activate(&gs->wind);
                wind->timer = 10+rand()%20;
            } else {
                wind->timer--;
            }
        }
    }

    for (int i = 0; i < wind->instance_count; i++) {
        bool done = wind_stream_draw(target, &wind->instances[i]);
        if (done) {
            wind->instance_count--;
            for (int j = i; j < wind->instance_count; j++) {
                wind->instances[j] = wind->instances[j+1];
            }
            i--;
        }
    }
}

// Returns when finished.
bool wind_stream_draw(int target, Wind_Stream_Instance *instance) {
    Wind_Stream *stream = instance->stream;

    if (gs->should_update) {
        if (!gs->paused || gs->step_one) {
            // Tick
            switch (instance->state) {
                case WIND_STREAM_IDLE: {
                } break;
                case WIND_STREAM_GROW: {
                    instance->range_end += instance->speed;
                    instance->speed += 0.03;
                    if (instance->range_end >= stream->point_count-1) {
                        instance->range_end = stream->point_count-1;
                        instance->state = WIND_STREAM_SHRINK;
                        instance->speed = 0.4;
                    }
                } break;
                case WIND_STREAM_SHRINK: {
                    instance->range_start += instance->speed;
                    instance->speed += 0.02;
                    if (instance->range_start >= stream->point_count-1+1) { // We want to display the one last pixel before stopping.
                        memset(instance, 0, sizeof(Wind_Stream_Instance));
                        return true;
                    } else if (instance->range_start >= stream->point_count-1) {
                        instance->range_start = stream->point_count-1;
                    }
                } break;
            }

            instance->time += 1.0/60.0;
        }
    }

    Assert(instance->range_end < stream->point_count);
    Assert(instance->range_start < stream->point_count);

    for (int i = (int)instance->range_start; i <= (int)instance->range_end; i++) {
        RenderColor(255, 255, 255, (100+i*i*i*i+(int)instance->x)%50);
        RenderPointRelative(target,
                            round(stream->points[i].x+instance->x),
                            round(stream->points[i].y+instance->y));
    }

    if (gs->should_update) {
        f32 k = sin(instance->time*2.5);
        k *= k;
        instance->x -= k;
    }

    return false;
}