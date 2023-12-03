#define ALASKA_RELEASE_MODE

#define MAX_PATH 260
#define M_PI 3.14159265358979323

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

#define MessageBox(fmt, ...) do{char msg[512]={0}; sprintf(msg, "alert('" fmt "')", __VA_ARGS__); emscripten_run_script(msg); }while(0);


#include <emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, focuscanvas, (), {
          var canvas = document.getElementById("canvas");
          canvas.focus();
      });

EM_JS(void, canvas_set_size, (int desired_width, int desired_height, double device_pixel_ratio), {
          var canvas = document.getElementById('canvas');

          var new_w = desired_width / device_pixel_ratio;
          var new_h = desired_height / device_pixel_ratio;

          canvas.style.width = new_w + "px"; // 460.8 @ scale=9
          canvas.style.height = new_h + "px"; // 518.4 @ scale=9
          canvas.width = new_w;
          canvas.height = new_h;
      });

EM_JS(int, canvas_get_width, (), {
          var canvas = document.getElementById('canvas');
          return parseInt(canvas.style.width);
      });

EM_JS(int, canvas_get_height, (), {
          var canvas = document.getElementById('canvas');
          return parseInt(canvas.style.height);
      });

#include "headers.h"

#include "game.c"
#include "assets.c"

typedef struct GameLoopData {
    Memory_Arena persistent_memory, transient_memory;
    Game_State *game_state;
} GameLoopData;

static void fail(const char *msg) {
    char message[256];
    sprintf(message, "alert('An error occurred when initializing. Error message:\n%s')", msg);
    emscripten_run_script(message);
    exit(1);
}

// Separate because audio is initted upon player clicking
static void game_init_sdl_audio(Game_State *state) {
    bool ok = true;

    ok = (Mix_Init(MIX_INIT_OGG) != 0);
    if (!ok) fail("Failed to init SDL mixer");

    ok = Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) >= 0;
    if (!ok) fail("Failed to open audio device");
}

static void game_init_sdl_em(Game_State *state, const char *window_title, int w, int h, f64 device_pixel_ratio) {
    bool ok = true;

    ok = (SDL_Init(SDL_INIT_VIDEO) == 0);
    if (!ok) fail("Failed to init SDL");

    {
        SDL_DisplayMode dm;

        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            fail(SDL_GetError());
        }

        state->desktop_w = dm.w;
        state->desktop_h = dm.h;
    }

    state->window = SDL_CreateWindow(window_title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     w,
                                     h,
                                     0);
    if (!state->window) fail("Failed to create window");

    state->device_pixel_ratio = device_pixel_ratio;

    canvas_set_size(state->real_width, state->real_height, device_pixel_ratio);
    
    game_init_sdl_audio(state);

    ok = (IMG_Init(IMG_INIT_PNG) != 0);
    if (!ok) fail("Failed to init SDL image");

    ok = (TTF_Init() == 0);
    if (!ok) fail("Failed to init SDL ttf");

    int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    state->renderer = SDL_CreateRenderer(state->window, -1, flags);
    if (!state->renderer) fail("Failed to create the renderer");

    state->render = RenderInit(state->renderer);

    input_set_locked(true);
}

static void make_memory_arena(Memory_Arena *persistent_memory, Memory_Arena *transient_memory) {
    persistent_memory->size = Megabytes(512);
    transient_memory->size = Megabytes(8);

    persistent_memory->data = calloc(persistent_memory->size + transient_memory->size, 1);
    if (!persistent_memory->data) {
        fail("Failed to allocate memory for the game!\nTry closing other applications/tabs then try again.");
    }

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

    state->S = 12;
    // 768 x 864

    f64 device_pixel_ratio = emscripten_get_device_pixel_ratio();

    state->game_width = 64*state->S;
    state->game_height = 64*state->S + GUI_H;

    state->real_width = state->game_width;
    state->real_height = state->game_height;

    game_init_sdl_em(state,
                     "Alaska",
                     state->game_width,
                     state->game_height,
                     device_pixel_ratio);

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
    
    if (SDL_GetMouseFocus() != null) {
        focuscanvas();
        gs->test = false;
    }
    
    data->game_state->input.em_dx = 0;
    data->game_state->input.em_dy = 0;

    while (SDL_PollEvent(&event)) {
        bool should_continue = game_handle_event(data->game_state, &event);
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

    gs->fullscreen = false;

    gs->persistent_memory = &data.persistent_memory;
    gs->transient_memory = &data.transient_memory;

    game_init_emscripten(gs);
    game_init(gs);

    render_targets_init();

    // TODO: How do I make this ensure 60fps?
    emscripten_set_main_loop_arg(em_mainloop, &data, 0, 1);

    free(data.persistent_memory.data);
}