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
// Note that the DLL is not allowed to allocate memory
// because we're statically linking against the C Runtime
// Library. So the DLL and the platform layer do not share
// the same heap. The solution is to simply allocate a huge
// block of memory at the start of the program in this file
// using VirtualAlloc() and give it pointers into that
// memory block through functions defined in shared.h
//
// This method also means you can't store "local persistent"
// (static) variables inside functions nor global variables
// because they reset every time you reload the DLL. The only
// global variable I have is "gs", which is the GameState.
// It's value gets reset every frame in case the dll reloads.
// Globals are probably not very good practice in a lot of
// uses anyways.
//
// This game uses a jumbo/unity build, so we don't need
// function prototypes or includes anywhere except headers.h,
// shared.h, main.c and game.c. That way, headers are all
// just struct definitions or #defines!
//

#include <windows.h>

#include "shared.h"
#include "util.c"
#include "win32_SetProcessDpiAware.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "assets.c"
#include "input.c"

#define GAME_DLL_NAME "sculpture.dll"
#define TEMP_DLL_NAME "sculpture_temp.dll"
#define LOCK_NAME     "lock.tmp"

#define ALASKA_START_FULLSCREEN 0

typedef void (*GameInitProc)(Game_State *state, int start_level);
typedef bool (*GameTickEventProc)(Game_State *state, SDL_Event *event);
typedef void (*GameRunProc)(Game_State *state);

typedef struct Game_Code {
    HMODULE dll;
    FILETIME last_write_time;
    
    GameInitProc game_init;
    GameTickEventProc game_tick_event;
    GameRunProc game_run;
} Game_Code;

void game_init_sdl(Game_State *state, const char *window_title, int w, int h, bool use_software_renderer) {
    SDL_Init(SDL_INIT_VIDEO);

    Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
    
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

void make_memory_arena(Memory_Arena *persistent_memory, Memory_Arena *transient_memory) {
    persistent_memory->size = Megabytes(1024);
    transient_memory->size = Megabytes(8);
    
    AssertNW(persistent_memory->size >= sizeof(Game_State));
    
    LPVOID base_address = (LPVOID) Terabytes(2);
    
    persistent_memory->data = VirtualAlloc(base_address,
                                           persistent_memory->size + transient_memory->size,
                                           MEM_COMMIT | MEM_RESERVE,
                                           PAGE_READWRITE);
    AssertNW(persistent_memory->data);
    persistent_memory->cursor = persistent_memory->data;
    
    // Set the transient memory as an offset into persistent memory.
    transient_memory->data = persistent_memory->data + persistent_memory->size;
    transient_memory->cursor = transient_memory->data;
}

f64 calculate_scale(bool fullscreen, int *dw, int *dh) {
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

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

void game_init(Game_State *state) {
    srand((unsigned int) time(0));
    
    if (state->S == 0)
        state->S = calculate_scale(false, &state->desktop_w, &state->desktop_h);
    
    state->window_width = (int)(64*state->S);
    state->window_height = (int)(64*state->S + GUI_H);
    
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

void game_deinit(Game_State *state) {
    //textures_deinit(&state->textures);
    surfaces_deinit(&state->surfaces);
    fonts_deinit(&state->fonts);
    audio_deinit(&state->audio);
    
    SDL_Quit();
}

FILETIME get_last_write_time(char *filename) {
    FILETIME result = {0};
    WIN32_FILE_ATTRIBUTE_DATA data;
    
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        result = data.ftLastWriteTime;
    }
    
    return result;
}

void load_game_code(Game_Code *code) {
    code->last_write_time = get_last_write_time(GAME_DLL_NAME);
    
    // Copy File may fail the first few times ..?
    int copy_counter = 0;
    while (1) {
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

void reload_game_code(Game_Code *code) {
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

#ifdef ALASKA_RELEASE_MODE
int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    // Taken from https://github.com/kumar8600/win32_SetProcessDpiAware
    win32_SetProcessDpiAware();
    
#ifndef ALASKA_RELEASE_MODE
    
    // Make sure we're running in the right folder.
    {
        char cwd[1024] = {0};
        size_t length = 0;
        char *final_three_chars = NULL;
        
        GetCurrentDirectory(1024, cwd);
        length = strlen(cwd);
        
        final_three_chars = cwd+length-3;
        
        AssertNW(0 == strcmp(final_three_chars, "bin"));
    }
#endif
    
    // The level to start on
    int start_level = 0;
#ifndef ALASKA_RELEASE_MODE
    if (argc == 2) {
        start_level = atoi(argv[1])-1;
        if (start_level < 0) start_level = 0;
        if (start_level >= 10) start_level = 9;
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
    
    Game_Code game_code = {0};
    load_game_code(&game_code);
    
    Memory_Arena persistent_memory, transient_memory;
    make_memory_arena(&persistent_memory, &transient_memory);
    
    // *1.5 in case we add more values at runtime.
    Game_State *game_state = PushSize(&persistent_memory, sizeof(Game_State));
    gs = game_state; // This is so that our macros can pick up "gs" instead of game_state.
    
    //scale = scale_popup();
    
    game_state->desktop_w = dw;
    game_state->desktop_h = dh;
    
    game_state->use_software_renderer = use_software_renderer;
    game_state->S = scale;
    
    game_state->fullscreen = fullscreen;
    
    game_state->persistent_memory = &persistent_memory;
    game_state->transient_memory = &transient_memory;
    
    game_init(game_state);
    game_code.game_init(game_state, start_level);
    
    // Only now, since the levels have been instantiated,
    // can we initialize render targets (since they depend
    // on each level's width/height)
    
    render_targets_init(game_state->renderer,
                        game_state->levels,
                        &game_state->textures);
    
    
    bool running = true;
    
    LARGE_INTEGER time_start, time_elapsed;
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time_start);
    
    const f64 target_seconds_per_frame = 1.0/60.0;
    f64 time_passed = 0.0;
    f32 fps = 0.f;
    
    while (running) {
        LARGE_INTEGER time_elapsed_for_frame;
        QueryPerformanceCounter(&time_elapsed_for_frame);
        
        // We push the DLL reload to a delay because if we lock the file too fast
        // the compiler doesn't have enough time to write to it... or something.
        // It's only a 5-frame delay so it doesn't matter compared to compilation
        // times anyways.
        FILETIME new_dll_write_time = get_last_write_time(GAME_DLL_NAME);
        
        if (CompareFileTime(&new_dll_write_time, &game_code.last_write_time) != 0) {
            reload_game_code(&game_code);
        }
        
        input_tick(game_state);
        
        SDL_Event event;
        
        game_state->resized = false;
        while (SDL_PollEvent(&event)) {
            bool should_continue = game_code.game_tick_event(game_state, &event);
            if (!should_continue) {
                running = false;
            }
        }
        
        game_code.game_run(game_state);
        
        // Zero out the transient memory for next frame!
        memset(transient_memory.data, 0, transient_memory.size);
        transient_memory.cursor = transient_memory.data;
        
        QueryPerformanceCounter(&time_elapsed);
        
        if (use_software_renderer) {
            Uint64 delta = time_elapsed.QuadPart - time_elapsed_for_frame.QuadPart;
            f64 d = (f64)delta / (f64)frequency.QuadPart;
            
            // Sleep until 16.6ms has passed.
            while (d < target_seconds_per_frame) {
                LARGE_INTEGER end;
                QueryPerformanceCounter(&end);
                d = (f64)(end.QuadPart - time_elapsed_for_frame.QuadPart) / (f64)frequency.QuadPart;
            }
            
            time_passed += d;
            
            // Only update FPS counter every 0.25s.
            if (time_passed > 0.1) {
                fps = 1.0/d;
                time_passed = 0;
            }
            
            {
                Uint64 size_current = persistent_memory.cursor - persistent_memory.data;
                Uint64 size_max = persistent_memory.size;
                f32 percentage = (f32)size_current / (f32)size_max;
                percentage *= 100.f;
                
                char title[128] = {0};
                sprintf(title,
                        "Alaska | Memory Used: %.2f/%.2f MB [%.2f%%] | FPS: %.2f",
                        size_current/1024.0/1024.0,
                        size_max/1024.0/1024.0,
                        percentage,
                        fps);
                
                SDL_SetWindowTitle(game_state->window, title);
            }
        } else {
            Uint64 size_current = persistent_memory.cursor - persistent_memory.data;
            Uint64 size_max = persistent_memory.size;
            f32 percentage = (f32)size_current / (f32)size_max;
            percentage *= 100.f;
            
            char title[128] = {0};
            sprintf(title,
                    "Alaska | Memory Used: %.2f/%.2f MB [%.2f%%]",
                    size_current/1024.0/1024.0,
                    size_max/1024.0/1024.0,
                    percentage);
            
            SDL_SetWindowTitle(game_state->window, title);
        }
    }
    
    game_deinit(game_state);
    
    VirtualFree(persistent_memory.data, 0, MEM_RELEASE);
    
    return 0;
}
