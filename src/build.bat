@echo off
gcc *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe

pushd ..\bin
rem Producing .pdb file from GCC debug symbols.
cv2pdb sculpture.exe
popd
