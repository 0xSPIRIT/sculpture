@echo off

set Common_Compiler_Flags=/nologo /GR- /EHa- /Zi /MT /FC /Fo:"obj\\" 
set Common_Linker_Flags=user32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib SDL2_image.lib

rem @echo off
rem gcc booter/win32__main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe

pushd ..\bin

rem Producing actual game DLL.
cl.exe %Common_Compiler_Flags% /Fdsculpture.pdb ..\src\*.c %Common_Linker_Flags% /link /out:sculpture.exe /machine:x64

exit errorlevel

popd

exit errorlevel
   
