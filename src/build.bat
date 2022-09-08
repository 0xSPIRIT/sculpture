@echo off

pushd ..\bin
cl.exe /nologo /GR- /EHa- /MT /Zi /Oi /FC /Fo:"obj\\" ..\src\*.c user32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib SDL2_image.lib /link /out:sculpture.exe /machine:x64
popd

exit errorlevel
   
rem @echo off
rem call build_cl.bat
rem @echo off
rem gcc *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe
rem exit errorlevel
