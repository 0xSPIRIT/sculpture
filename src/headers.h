#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "typedefs.h"
#include "cursor.h"
#include "overlay.h"
#include "grid.h"
#include "chisel.h"
#include "deleter.h"
#include "popup.h"
#include "placer.h"
#include "level.h"
#include "overlay_interface.h"
#include "inventory.h"
#include "gui.h"
#include "input.h"
#include "grabber.h"
#include "effects.h"
#include "chisel_blocker.h"
#include "blocker.h"
#include "assets.h"
#include "undo.h"
#include "view.h"