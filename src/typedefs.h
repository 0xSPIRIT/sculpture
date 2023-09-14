#define Error(...) do{fprintf(stderr, __VA_ARGS__), fflush(stderr);}while(0);

#ifndef ALASKA_RELEASE_MODE
  #define Log(...) do{char msg[8192]; sprintf(msg, __VA_ARGS__), OutputDebugString(msg); }while(0);
#else
  #define Log(...) ((void)0)
#endif

#define BLACK ((SDL_Color){0,0,0,255})
#define WHITE ((SDL_Color){255,255,255,255})

#define Radians(x) ((x) * (M_PI / 180.0))
#define Degrees(x) ((x) * (180.0 / M_PI))

#define null 0

typedef float  f32;
typedef double f64;

// Just my little convention
typedef Uint8  u8;
typedef Uint16 u16;
typedef Uint32 u32;
typedef Uint64 u64;
typedef Sint16 s16;
typedef Sint32 s32;
typedef Sint64 s64;

#ifndef ALASKA_RELEASE_MODE
  #define export    __declspec(dllexport)
#else
  #define export
#endif
