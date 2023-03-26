@echo off

pushd out\

call emcc ../src/em_main.c --preload-file ..\res -sALLOW_MEMORY_GROWTH -error-limit=0 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -lSDL2 -lSDL2_ttf -fdeclspec -o index.html

popd
