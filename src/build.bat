@echo off

set Common_Compiler_Flags=/nologo /GR- /EHa- /Zi /MT /FC /Fo:"obj\\" 
set Common_Linker_Flags=user32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib SDL2_image.lib

rem @echo off
rem gcc booter/main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe

pushd ..\bin

rem Producing actual game DLL.
cl.exe %Common_Compiler_Flags% /Fdsculpture.pdb ..\src\*.c /Fmsculpture.map %Common_Linker_Flags% /link /DLL /out:sculpture.dll /machine:x64

rem Producing booter exe
cl.exe %Common_Compiler_Flags% /Fdwin32_sculpture.pdb ..\src\booter\main.c %Common_Linker_Flags% /link /out:win32_sculpture.exe /machine:x64
exit errorlevel

popd

exit errorlevel
   
