#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#ifndef __EMSCRIPTEN__
#include <windows.h>
#endif

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "typedefs.h"
#include "audio.h"
#include "render.h"
#include "colors.h"
#include "cursor.h"
#include "fades.h"
#include "preview.h"
#include "credits.h"
#include "overlay.h"
#include "grid.h"
#include "chisel.h"
#include "hammer.h"
#include "narrator.h"
#include "popup.h"
#include "confirm_popup.h"
#include "level.h"
#include "overlay_interface.h"
#include "inventory.h"
#include "placer.h"
#include "tooltip.h"
#include "gui.h"
#include "converter.h"
#include "input.h"
#include "grabber.h"
#include "effects.h"
#include "tutorial.h"
#include "assets.h"
#include "undo.h"
#include "timelapse.h"
#include "3d.h"
#include "titlescreen.h"
#include "background.h"
