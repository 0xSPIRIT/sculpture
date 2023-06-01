// A wrapper for calls to SDL's draw functions.
// We use this for things like global offsets,
// and to manage render targets better.

typedef enum {
    VIEW_STATE_UNDEFINED,   // Invalid
    VIEW_STATE_SCREENSPACE, // Size: gs->window_width, gs->window_height
    VIEW_STATE_PIXELS,      // Size: gs->gw, gs->gh
} View_State;

typedef struct {
    int x, y, width, height;
} Rect;

typedef struct {
    int x, y;
} Point;

typedef struct {
    SDL_Texture *handle;
    int full_width;         // These full size parameters
    int full_height;        // refer to the actual size of the texture...
    int working_width;      // Whereas the working size parameters are less,
    int working_height;     // enabling the use of negative texture positions.
    Point top_left;
    View_State view; // Is this in screenspace or pixel art?
} Render_Target;

typedef struct {
    SDL_Texture *handle;
    int width, height;
} Texture;

typedef struct {
    TTF_Font *handle;
    int char_width, char_height;
} Font;

typedef enum {
    ALIGNMENT_TOP_LEFT,
    ALIGNMENT_TOP_RIGHT,
    ALIGNMENT_BOTTOM_LEFT,
    ALIGNEMNT_BOTTOM_RIGHT,
    ALIGNMENT_CENTER
} Alignment;

// The structure used when drawing text to the screen.
typedef struct {
    Font *font;
    const char *str;
    SDL_Color foreground;
    SDL_Color background;
    Alignment alignment;
    bool force_update;    // Should we force redraw?
    
    Texture *texture;     // OUT: The texture generated
} Render_Text_Data;

// TODO: Move things that are in Game_State, such as
//       the texture struct into here.
typedef struct {
    SDL_Renderer *sdl;
    View_State view;
    Render_Target *current_target;
    
    int unfreed_objects;
} Render;

static Render RenderInit(SDL_Renderer *sdl_renderer);

static SDL_Surface *RenderLoadSurface(const char *fp);
static Texture *RenderLoadTexture(const char *fp);
static Texture *RenderCreateTextureFromSurface(SDL_Surface *surf);
static Font *RenderLoadFont(const char *fp, int size);

static Render_Target *RenderMakeRenderTarget(View_State view, bool use_negative_coords, int width, int height);

static void RenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
static void RenderColorStruct(SDL_Color rgba);
static void RenderLine(Render_Target *target, int x1, int y1, int x2, int y2);
static void RenderPoint(Render_Target *target, int x1, int y1);
static void RenderDrawRect(Render_Target *target, Rect rect);
static void RenderFillRect(Render_Target *target, Rect rect);
static void RenderClear(Render_Target *target);
static void RenderPresent(Render_Target *target);
static void RenderTexture(Render_Target *target, Texture *texture, Rect src, Rect dst);
static void RenderTextureEx(Render_Target *target, Texture *texture, Rect src, Rect dst, f64 angle, Point center, SDL_RendererFlip flip);
static void RenderDrawText(Render_Target *target, Render_Text_Data *text_data);
static void RenderSetFontSize(Font *font, int size);
static void RenderReadPixels(Uint8 *pixels, int pitch);