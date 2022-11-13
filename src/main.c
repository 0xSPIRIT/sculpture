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
#include "win32_SetProcessDpiAware.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "util.c"
#include "assets.c"
#include "input.c"

#define GAME_DLL_NAME "sculpture.dll"
#define TEMP_DLL_NAME "sculpture_temp.dll"
#define LOCK_NAME     "lock.tmp"

#define ALASKA_USE_SOFTWARE_RENDERER

typedef void (*GameInitProc)(struct Game_State *state, int start_level);
typedef bool (*GameTickEventProc)(struct Game_State *state, SDL_Event *event);
typedef void (*GameRunProc)(struct Game_State *state);

struct Game_Code {
    HMODULE dll;
    FILETIME last_write_time;
    
    GameInitProc game_init;
    GameTickEventProc game_tick_event;
    GameRunProc game_run;
};

void game_init_sdl(struct Game_State *state, const char *window_title, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    
    state->window = SDL_CreateWindow(window_title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     w,
                                     h,
                                     SDL_WINDOW_SHOWN);
    Assert(state->window);
    
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    
#ifdef ALASKA_USE_SOFTWARE_RENDERER
    const int flags = SDL_RENDERER_SOFTWARE;
#else
    const int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
#endif
    
    state->renderer = SDL_CreateRenderer(state->window, -1, flags);
    Assert(state->renderer);
}

void make_memory_arena(struct Memory_Arena *persistent_memory, struct Memory_Arena *transient_memory) {
    persistent_memory->size = Megabytes(1024);
    transient_memory->size = Megabytes(8);
    
    AssertNW(persistent_memory->size >= sizeof(struct Game_State));
    
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

void game_init(struct Game_State *state) {
    srand((unsigned int) time(0));
    
    state->S = 6;
    state->window_width = 128*state->S;
    state->window_height = 128*state->S + GUI_H;
    
    // Taken from https://github.com/kumar8600/win32_SetProcessDpiAware
    win32_SetProcessDpiAware();
    
    game_init_sdl(state,
                  "Alaska", 
                  state->window_width, 
                  state->window_height);
    
    // Load all assets... except for render targets.
    // We can't create render targets until levels
    // are initialized.
    textures_init(state->renderer, &state->textures);
    surfaces_init(&state->surfaces);
    
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
    
    state->normal_cursor = SDL_GetCursor();
    state->grabber_cursor = init_system_cursor(arrow_cursor_data);
    state->placer_cursor = init_system_cursor(placer_cursor_data);
    
    fonts_init(&state->fonts);
}

void game_deinit(struct Game_State *state) {
    // Close the window first so it'll feel snappy.
    SDL_DestroyWindow(state->window);
    
    textures_deinit(&state->textures);
    surfaces_deinit(&state->surfaces);
    fonts_deinit(&state->fonts);
    
    SDL_DestroyRenderer(state->renderer);
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// @Performance
inline FILETIME get_last_write_time(char *filename) {
    FILETIME result = {0};
    WIN32_FILE_ATTRIBUTE_DATA data;
    
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        result = data.ftLastWriteTime;
    }
    
    return result;
}

void load_game_code(struct Game_Code *code) {
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
    
    if (!code->game_run) {
        Error("Error finding the functions in the DLL!\n");
        exit(1);
    }
}

void reload_game_code(struct Game_Code *code) {
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
    if (argc == 2) {
        start_level = atoi(argv[1]);
    }
    
    struct Game_Code game_code = {0};
    load_game_code(&game_code);
    
    struct Memory_Arena persistent_memory, transient_memory;
    make_memory_arena(&persistent_memory, &transient_memory);
    
    // *1.5 in case we add more values at runtime.
    struct Game_State *game_state = PushSize(&persistent_memory, sizeof(struct Game_State));
    
    gs = game_state; // This is so that our macros can pick up "gs" instead of game_state.
    
    game_state->persistent_memory = &persistent_memory;
    game_state->transient_memory = &transient_memory;
    
    game_init(game_state);
    game_code.game_init(game_state, start_level);
    
    // Only now, since the levels have been instantiated,
    // can we initialize render targets (since they depend
    // on each level's width/height)
    
    render_targets_init(game_state->renderer, game_state->window_width, game_state->levels, &game_state->textures);
    
    bool running = true;
    
    LARGE_INTEGER time_start, time_elapsed;
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time_start);
    
#ifdef ALASKA_USE_SOFTWARE_RENDERER
    const f64 target_seconds_per_frame = 1.0/60.0;
    f64 time_passed = 0.0;
#endif
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
        
        // if (should_stop) {
            // running = false;
        // }
        
        QueryPerformanceCounter(&time_elapsed);
        
#ifdef ALASKA_USE_SOFTWARE_RENDERER
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
#endif
        
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
    }
    
    game_deinit(game_state);
    
    VirtualFree(persistent_memory.data, 0, MEM_RELEASE);
    
    return 0;
}
