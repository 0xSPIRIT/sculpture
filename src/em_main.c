#define ALASKA_RELEASE_MODE

#define MAX_PATH 260
#define M_PI 3.14159265358979323

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

#include <math.h>

#include "game.c"

#include "assets.c"
#include "input.c"

#include <emscripten.h>

typedef struct GameLoopData {
    Memory_Arena persistent_memory, transient_memory;
    Game_State *game_state;
} GameLoopData;

static void fail(int code) {
    char message[256];
    sprintf(message, "An error occurred when initializing. Code: %d", code);
    puts(message);
#ifndef ALASKA_RELEASE_MODE
    __debugbreak();
#else
    exit(1);
    #endif
}

// Separate because audio is initted upon player clicking
static void game_init_sdl_audio(Game_State *state) {
    bool ok = true;
    
    ok = (Mix_Init(MIX_INIT_OGG) != 0);
    if (!ok) fail(2);
    
    ok = Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) >= 0;
    if (!ok) fail(6);
}

static void game_init_sdl_em(Game_State *state, const char *window_title, int w, int h) {
    bool ok = true;

    ok = (SDL_Init(SDL_INIT_VIDEO) == 0);
    if (!ok) fail(1);
    
    {
        SDL_DisplayMode dm;
        
        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            printf("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        }
        
        state->desktop_w = dm.w;
        state->desktop_h = dm.h;
        
        printf("Desktop width: %d, Desktop height: %d\n", dm.w, dm.h);
    }

    state->window = SDL_CreateWindow(window_title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     w,
                                     h,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!state->window) fail(3);

    ok = (IMG_Init(IMG_INIT_PNG) != 0);
    if (!ok) fail(4);

    ok = (TTF_Init() == 0);
    if (!ok) fail(5);

    int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    state->renderer = SDL_CreateRenderer(state->window, -1, flags);
    if (!state->renderer) fail(7);
    
    state->render = RenderInit(state->renderer);

    if (state->fullscreen) {
        SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void make_memory_arena(Memory_Arena *persistent_memory, Memory_Arena *transient_memory) {
    persistent_memory->size = Megabytes(512);
    transient_memory->size = Megabytes(8);

    persistent_memory->data = calloc(persistent_memory->size + transient_memory->size, 1);
    if (!persistent_memory->data) {
        fail(100);
    }
    printf("Base Memory: %p\n", persistent_memory->data);
    
    if (!persistent_memory->data) fail(8);
    persistent_memory->cursor = persistent_memory->data;

    // Set the transient memory as an offset into persistent memory.
    transient_memory->data = persistent_memory->data + persistent_memory->size;
    transient_memory->cursor = transient_memory->data;
}


static bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void game_init_emscripten(Game_State *state) {
    srand((unsigned int) time(0));
    
    state->S = 9;

    state->game_width = 64*state->S;
    state->game_height = 64*state->S + GUI_H;

    state->real_width = state->game_width;
    state->real_height = state->game_height;

    game_init_sdl_em(state,
                     "Alaska",
                     state->game_width,
                     state->game_height);

    // Load all assets... except for render targets.
    // We can't create render targets until levels
    // are initialized.
    audio_init(&state->audio);
    textures_init(&state->textures);
    surfaces_init(&state->surfaces);

    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);

    fonts_init(&state->fonts);
}

static void game_deinit(Game_State *state) {
    //textures_deinit(&state->textures);
    surfaces_deinit(&state->surfaces);
    fonts_deinit(&state->fonts);
    audio_deinit(&state->audio);
    
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);

    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
}

static void em_mainloop(void *arg) {
    GameLoopData *data = (GameLoopData*) arg;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        bool should_continue = game_tick_event(data->game_state, &event);
        if (!should_continue) {
            emscripten_cancel_main_loop();
        }
    }
    
    input_tick(data->game_state);

    game_run(data->game_state);
    
    memset(data->transient_memory.data, 0, data->transient_memory.size);
    data->transient_memory.cursor = data->transient_memory.data;
}

int main(int argc, char **argv) {
    GameLoopData data;

    make_memory_arena(&data.persistent_memory, &data.transient_memory);
    data.game_state = PushSize(&data.persistent_memory, sizeof(Game_State));
    gs = data.game_state;

    gs->S = 7;
    gs->fullscreen = false;

    gs->persistent_memory = &data.persistent_memory;
    gs->transient_memory = &data.transient_memory;

    game_init_emscripten(gs);
    game_init(gs);

    render_targets_init();

    emscripten_set_main_loop_arg(em_mainloop, &data, 0, 1);

    free(data.persistent_memory.data);
}
