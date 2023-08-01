//
// Basically, the game is split up into two compilation units.
// This one is compiled to an .exe file that simply loads
// a DLL, intializes SDL, loads the game's assets, and
// calls two functions from the DLL to run the game.
//
// The app itself is, as I said, compiled to a .dll and can
// be found in game.c
//
// The benefit of doing this is you get live code reloading;
// you can compile your code while the app is running and
// the "platform" layer (this file here) will just reload
// the DLL and continue calling the updated functions with
// the preexisting memory.
//
// The way memory works here is we simply allocate a huge
// block of memory at the start of the program in this file
// using VirtualAlloc() and give the game pointers into that
// memory block through functions defined in shared.h
//
// This method also means you can't store "local persistent"
// (static) variables inside functions nor global variables
// because they reset every time you reload the DLL. The only
// global variable I have is "gs", which is the GameState,
// for convenience, so I don't have to pass it into every
// function. It's value gets reset every frame in case the
// dll reloads.
//


// Includes

#include <windows.h>

#ifndef ALASKA_RELEASE_MODE // Debug mode
  #include "shared.h"
  #include "render.c"
  #include "util.c"
#else                       // Release mode
  #include "game.c"
#endif

#include "assets.c"
#include "input.c"

// Defines

#define GAME_DLL_NAME "sculpture.dll"
#define TEMP_DLL_NAME "sculpture_temp.dll"
#define LOCK_NAME     "lock.tmp"

#define ALASKA_START_FULLSCREEN 0

// Some function pointers for the hot-loading.

typedef void (*GameInitProc)(Game_State *state);
typedef bool (*GameTickEventProc)(Game_State *state, SDL_Event *event);
typedef void (*GameRunProc)(Game_State *state);

typedef struct Game_Code {
    HMODULE dll;
    FILETIME last_write_time;

    GameInitProc game_init;
    GameTickEventProc game_tick_event;
    GameRunProc game_run;
} Game_Code;

static void fail(int code) {
    char message[256];
    sprintf(message, "An error occurred when initializing. Code: %d", code);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error!", message, null);
#ifndef ALASKA_RELEASE_MODE
    __debugbreak();
#else
    exit(1);
    #endif
}

static void game_init_sdl(Game_State *state, const char *window_title, int w, int h, bool use_software_renderer) {
    bool ok = true;

    ok = (SDL_Init(SDL_INIT_VIDEO) == 0);
    if (!ok) fail(1);

    ok = (Mix_Init(MIX_INIT_OGG) != 0);
    if (!ok) fail(2);

    int x = 200;
    int y = 125;

#ifdef ALASKA_RELEASE_MODE
    x = SDL_WINDOWPOS_CENTERED;
    y = SDL_WINDOWPOS_CENTERED;
#endif

    state->window = SDL_CreateWindow(window_title,
                                     x,
                                     y,
                                     w,
                                     h,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!state->window) fail(3);

    SDL_Surface *window_icon = RenderLoadSurface("icon.png");
    SDL_SetWindowIcon(state->window, window_icon);

    SDL_FreeSurface(window_icon);

    ok = (IMG_Init(IMG_INIT_PNG) != 0);
    if (!ok) fail(4);

    ok = (TTF_Init() == 0);
    if (!ok) fail(5);

    ok = (Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) >= 0);
    if (!ok) fail(6);

    int flags = 0;
    if (use_software_renderer) {
        flags = SDL_RENDERER_SOFTWARE;
    } else {
        flags = SDL_RENDERER_ACCELERATED;
    }

    flags |= SDL_RENDERER_PRESENTVSYNC;

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

    Assert(persistent_memory->size >= sizeof(Game_State));

    void *base_address = 0;
#ifndef ALASKA_RELEASE_MODE
    base_address = (void*)Terabytes(2);
#endif

    persistent_memory->data = VirtualAlloc(base_address,
                                           persistent_memory->size + transient_memory->size,
                                           MEM_COMMIT | MEM_RESERVE,
                                           PAGE_READWRITE);
    if (!persistent_memory->data) fail(10);

    persistent_memory->cursor = persistent_memory->data;

    // Set the transient memory as an offset into persistent memory.
    transient_memory->data = persistent_memory->data + persistent_memory->size;
    transient_memory->cursor = transient_memory->data;
}

static f64 calculate_scale(bool fullscreen, int *dw, int *dh) {
    RECT desktop;
    HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    int w = desktop.right;
    int h = desktop.bottom;

    if (dw) *dw=w;
    if (dh) *dh=h;

    if (fullscreen) {
        return h/72.0;
        //return (int)round(7.0 * h/1080.0);
    } else {
        return (int)round(12.0 * h/1080.0);
    }
}

static bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void win32_game_init(Game_State *state) {
    srand((unsigned int) time(0));

    if (state->S == 0)
        state->S = calculate_scale(false, &state->desktop_w, &state->desktop_h);

    state->game_width = (int)(64*state->S);
    state->game_height = (int)(64*state->S + GUI_H);

    state->real_width = state->game_width;
    state->real_height = state->game_height;

    game_init_sdl(state,
                  "Alaska",
                  state->game_width,
                  state->game_height,
                  state->use_software_renderer);

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
    RenderCleanup(&state->render);
    fonts_deinit(&state->fonts);
    audio_deinit(&state->audio);

    SDL_Quit();
}

static FILETIME get_last_write_time(char *filename) {
    FILETIME result = {0};
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        result = data.ftLastWriteTime;
    }

    return result;
}

static void load_game_code(Game_Code *code) {
    code->last_write_time = get_last_write_time(GAME_DLL_NAME);

    // Copy File may fail the first few times ..?
    int copy_counter = 0;
    while (true) {
        copy_counter++;
        Assert(copy_counter <= 10);

        if (CopyFileA(GAME_DLL_NAME, TEMP_DLL_NAME, FALSE)) {
            break;
        }
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            break;
        }
    }

    code->dll = LoadLibraryA(TEMP_DLL_NAME);
    if (!code->dll) {
        Error("Error loading the DLL!\n");
        exit(1);
    }

    code->game_init = (GameInitProc)
        GetProcAddress(code->dll, "game_init");
    code->game_tick_event = (GameTickEventProc)
        GetProcAddress(code->dll, "game_tick_event");
    code->game_run = (GameRunProc)
        GetProcAddress(code->dll, "game_run");

    if (!code->game_run || !code->game_tick_event || !code->game_run) {
        Error("Error finding the functions in the DLL!\n");
        exit(1);
    }
}

static void reload_game_code(Game_Code *code) {
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (GetFileAttributesExA(LOCK_NAME, GetFileExInfoStandard, &ignored)) {
        return;
    }

    if (code->dll) {
        FreeLibrary(code->dll);
        code->game_init = 0;
        code->game_run = 0;
        code->game_tick_event = 0;
        code->dll = 0;
    }

    load_game_code(code);
}

// Taken from https://github.com/kumar8600/win32_SetProcessDpiAware/blob/master/win32_SetProcessDpiAware.c
//
// Created by kumar on 2016/03/29.
//

typedef enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT (WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

inline bool win32_SetProcessDpiAware(void) {
    HMODULE shcore = LoadLibraryA("Shcore.dll");
    SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
    if (shcore) {
        SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T) GetProcAddress(shcore, "SetProcessDpiAwareness");
    }
    HMODULE user32 = LoadLibraryA("User32.dll");
    SETPROCESSDPIAWARE_T SetProcessDPIAware = NULL;
    if (user32) {
        SetProcessDPIAware = (SETPROCESSDPIAWARE_T) GetProcAddress(user32, "SetProcessDPIAware");
    }

    bool ret = false;
    if (SetProcessDpiAwareness) {
        ret = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK;
    } else if (SetProcessDPIAware) {
        ret = SetProcessDPIAware() != 0;
    }

    if (user32) {
        FreeLibrary(user32);
    }
    if (shcore) {
        FreeLibrary(shcore);
    }
    return ret;
}

int win32_main(int argc, char **argv) {
    win32_SetProcessDpiAware();

#ifndef ALASKA_RELEASE_MODE
    // Make sure we're running in the right folder.
    {
        char cwd[1024] = {0};
        size_t length = 0;
        char *final_three_chars = null;

        GetCurrentDirectory(1024, cwd);
        length = strlen(cwd);

        final_three_chars = cwd+length-3;

        Assert(0 == strcmp(final_three_chars, "bin"));
    }
#endif

    f64 scale = 0;

    int fullscreen = ALASKA_START_FULLSCREEN;

    int dw=0, dh=0;

    if (fullscreen) {
        scale = calculate_scale(true, &dw, &dh);
    }

    bool use_software_renderer = false;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (0==strcmp(argv[i], "-renderer=gpu")) {
                use_software_renderer = false;
            } else if (0==strcmp(argv[i], "-renderer=cpu")) {
                use_software_renderer = true;
            } else if (prefix("-scale=", argv[i])) {
                scale = atoi(argv[i]+7);
            } else if (0==strcmp(argv[i], "-fullscreen")) {
                fullscreen = 1;
            }
        }
    }

#ifndef ALASKA_RELEASE_MODE
    Game_Code game_code = {0};
    load_game_code(&game_code);
#endif

    Memory_Arena persistent_memory, transient_memory;
    make_memory_arena(&persistent_memory, &transient_memory);

    gs = PushSize(&persistent_memory, sizeof(Game_State));

    gs->desktop_w = dw;
    gs->desktop_h = dh;

    gs->use_software_renderer = use_software_renderer;
    gs->S = scale;

    gs->fullscreen = fullscreen;

    gs->persistent_memory = &persistent_memory;
    gs->transient_memory = &transient_memory;

    win32_game_init(gs);

#ifdef ALASKA_RELEASE_MODE
    game_init(gs);
#else
    game_code.game_init(gs);
#endif

    // Only now, since the levels have been instantiated,
    // can we initialize render targets (since they depend
    // on each level's width/height)

    render_targets_init();

    bool running = true;

    LARGE_INTEGER time_start, time_elapsed;

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time_start);

    f64 time_passed = 0.0;
    int fps = 0, fps_draw = 0;

    while (running) {
        LARGE_INTEGER time_elapsed_for_frame;
        QueryPerformanceCounter(&time_elapsed_for_frame);

#ifndef ALASKA_RELEASE_MODE
        FILETIME new_dll_write_time = get_last_write_time(GAME_DLL_NAME);

        if (CompareFileTime(&new_dll_write_time, &game_code.last_write_time) != 0) {
            reload_game_code(&game_code);
        }
#endif

        input_tick(gs);

        SDL_Event event;

        gs->resized = false;
        while (SDL_PollEvent(&event)) {
#ifndef ALASKA_RELEASE_MODE
            bool should_continue = game_code.game_tick_event(gs, &event);
#else
            bool should_continue = game_tick_event(gs, &event);
#endif
            if (!should_continue) {
                running = false;
            }
        }

#ifndef ALASKA_RELEASE_MODE
        game_code.game_run(gs);
#else
        game_run(gs);
#endif

        // Zero out the transient memory for next frame!
        memset(transient_memory.data, 0, transient_memory.size);
        transient_memory.cursor = transient_memory.data;

        QueryPerformanceCounter(&time_elapsed);

        Uint64 delta = time_elapsed.QuadPart - time_elapsed_for_frame.QuadPart;
        f64 d = (f64)delta / (f64)frequency.QuadPart; // ~16.67 ms due to SDL_RenderPresent.

        // NOTE: d != gs->dt.
        //  d = Time taken with the vsync sleep taken into account.
        //  gs->dt = Time taken for frame alone.

        time_passed += d;

        if (time_passed >= 1) {
            fps_draw = fps;
            time_passed = 0;
            fps = 0;
        } else {
            fps++;
        }

#ifndef ALASKA_RELEASE_MODE
        Uint64 size_current = persistent_memory.cursor - persistent_memory.data;
        Uint64 size_max = persistent_memory.size;
        f32 percentage = (f32)size_current / (f32)size_max;
        percentage *= 100.f;

        char title[128] = {0};
        sprintf(title,
                "Alaska | Frametime: %.2fms | Memory Used: %.2f%%",
                gs->dt*1000,
                percentage);

        SDL_SetWindowTitle(gs->window, title);
#endif
    }

    game_deinit(gs);

    VirtualFree(persistent_memory.data, 0, MEM_RELEASE);

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nShowCmd)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    win32_main(0, null);
}
