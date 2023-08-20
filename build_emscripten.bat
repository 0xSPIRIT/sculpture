@echo off

pushd out

set EMCC_DEBUG=1
call emcc ../src/em_main.c -O1 -g4 -gsource-map -sASSERTIONS -sINITIAL_MEMORY=700MB -std=c17 --preload-file ../res/ -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSAFE_HEAP=1 -sSDL2_IMAGE_FORMATS="png" -o index.html
set EMCC_DEBUG=0

popd

copy src\* out\src\
