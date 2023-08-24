@echo off

pushd emscripten

set EMCC_DEBUG=1
call emcc ../src/em_main.c -O0 -gsource-map -sASSERTIONS -sSAFE_HEAP=1 -sSTACK_SIZE=256MB -sINITIAL_MEMORY=1024MB -std=c17 --preload-file ../res/ -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -o index.html
rem call emcc ../src/em_main.c --embed-file ../res -O3 -sSTACK_SIZE=256MB -sINITIAL_MEMORY=1024MB -std=c17 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -o index.html
set EMCC_DEBUG=0


popd
