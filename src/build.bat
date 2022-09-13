@echo off

set Common_Compiler_Flags=/nologo /Zi /MT /FC /Fo:"obj\\" 
set Common_Linker_Flags=user32.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib

rem @echo off
rem gcc booter/win32__main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe

pushd ..\bin

REM Build the SDL layer (.exe)
cl.exe %Common_Compiler_Flags% ..\src\boot\main.c %Common_Linker_Flags% SDL2main.lib  /link /out:SDL_sculpture.exe

REM Build the game layer (.dll that links into SDL layer)
cl.exe %Common_Compiler_Flags% ..\src\*.c %Common_Linker_Flags% /link /DLL /out:sculpture.dll

popd
   
exit errorlevel

