@echo off

pushd emscripten

set EMCC_DEBUG=0
call emcc ../src/em_main.c -O0 --preload-file ../data -sSTACK_SIZE=256MB -sINITIAL_MEMORY=1024MB -std=gnu17 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -o index.html

popd
