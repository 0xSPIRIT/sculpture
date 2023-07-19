#define Error(...) do{fprintf(stderr, __VA_ARGS__), fflush(stderr);}while(0);
#define Log(...) do{printf(__VA_ARGS__), fflush(stdout);}while(0);

#define BLACK ((SDL_Color){0,0,0,255})
#define WHITE ((SDL_Color){255,255,255,255})

#define Radians(x) ((x) * (M_PI / 180.0))
#define Degrees(x) ((x) * (180.0 / M_PI))

#define null 0

typedef float  f32;
typedef double f64;

#define export    __declspec(dllexport)
#define allocator __declspec(allocator)