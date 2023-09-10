@echo off

pushd emscripten

rem set EMCC_DEBUG=1
rem call emcc ../src/em_main.c -O0 -gsource-map -sASSERTIONS -sSAFE_HEAP=1 -sSTACK_SIZE=256MB -sINITIAL_MEMORY=1024MB -std=c17 --preload-file ../data/ -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -o index.html
call emcc ../src/em_main.c -O3 --preload-file ../data -sSTACK_SIZE=256MB -sINITIAL_MEMORY=1024MB -std=c17 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS="png" -o index.html --shell-file template.html
rem set EMCC_DEBUG=0

popd
