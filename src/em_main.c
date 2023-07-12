// EMSCRIPTEN MAIN.

#define ALASKA_RELEASE_MODE

#include "game.c"

#include "assets.c"
#include "input.c"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <dirent.h>
#include <errno.h>

typedef struct GameLoopData {
    Memory_Arena persistent_memory, transient_memory;
    Game_State *game_state;
} GameLoopData;

static void game_init_sdl(Game_State *state, const char *window_title, int w, int h, bool use_software_renderer) {
    SDL_Init(SDL_INIT_VIDEO);
    Assert(Mix_Init(MIX_INIT_OGG) != 0);

    state->window = SDL_CreateWindow(window_title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     w,
                                     h,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    Assert(state->window);

    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    Assert(Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) >= 0);

    int flags = 0;
    if (use_software_renderer) {
        flags = SDL_RENDERER_SOFTWARE;
    } else {
        flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    }

    state->renderer = SDL_CreateRenderer(state->window, -1, flags);
    Assert(state->renderer);

    if (state->fullscreen) {
        SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void make_memory_arena(Memory_Arena *persistent_memory, Memory_Arena *transient_memory) {
    persistent_memory->size = Megabytes(512);
    transient_memory->size = Megabytes(8);

    AssertNW(persistent_memory->size >= sizeof(Game_State));

    persistent_memory->data = malloc(persistent_memory->size + transient_memory->size);
    AssertNW(persistent_memory->data);
    persistent_memory->cursor = persistent_memory->data;

    // Set the transient memory as an offset into persistent memory.
    transient_memory->data = persistent_memory->data + persistent_memory->size;
    transient_memory->cursor = transient_memory->data;
}

static f64 calculate_scale(bool fullscreen) {
    return 7;
}

static bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void game_init_a(Game_State *state) {
    srand((unsigned int) time(0));

    if (state->S == 0)
        state->S = calculate_scale(false);

    state->window_width = 128*state->S;
    state->window_height = 128*state->S + GUI_H;

    state->real_width = state->window_width;
    state->real_height = state->window_height;

    game_init_sdl(state,
                  "Alaska",
                  state->window_width,
                  state->window_height,
                  state->use_software_renderer);

    // Load all assets... except for render targets.
    // We can't create render targets until levels
    // are initialized.
    audio_init(&state->audio);
    textures_init(state->renderer, &state->textures);
    surfaces_init(&state->surfaces);

    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);

    state->normal_cursor = SDL_GetCursor();
    state->grabber_cursor = init_system_cursor(arrow_cursor_data);
    state->placer_cursor = init_system_cursor(placer_cursor_data);

    fonts_init(&state->fonts);
}

static void game_deinit(Game_State *state) {
    //textures_deinit(&state->textures);
    surfaces_deinit(&state->surfaces);
    fonts_deinit(&state->fonts);
    audio_deinit(&state->audio);

    SDL_Quit();
}

static void em_mainloop(void *arg) {
    GameLoopData *data = (GameLoopData*) arg;

    input_tick(data->game_state);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        bool should_continue = game_tick_event(data->game_state, &event);
        if (!should_continue) {
            emscripten_cancel_main_loop();
        }
    }
    game_run(data->game_state);
    // Zero out the transient memory for next frame!
    memset(data->transient_memory.data, 0, data->transient_memory.size);
    data->transient_memory.cursor = data->transient_memory.data;
}

static int main(int argc, char **argv)
{
    GameLoopData data;

    DIR *d;
    dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != null) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }

    make_memory_arena(&data.persistent_memory, &data.transient_memory);
    data.game_state = PushSize(&data.persistent_memory, sizeof(Game_State));
    gs = data.game_state;

    gs->use_software_renderer = false;
    gs->S = 5;
    gs->fullscreen = false;

    gs->persistent_memory = &data.persistent_memory;
    gs->transient_memory = &data.transient_memory;

    game_init_a(gs);
    game_init(gs, 0);

    render_targets_init(gs->renderer,
                        gs->window_width,
                        gs->levels,
                        &gs->textures);

    emscripten_set_main_loop_arg(em_mainloop, &data, 0, 1);

    free(data.persistent_memory.data);
}
