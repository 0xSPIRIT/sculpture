@echo off
gcc *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe
exit %errorlevel%

rem set var=%errorlevel%
rem rem We don't want to try exporting pdf when we have an error, that'll just waste time.

rem if not %var% == 0 exit %var%

rem echo Info: Exporting .pdb...

rem pushd ..\bin
rem rem Producing .pdb file from GCC debug symbols.
rem cv2pdb sculpture.exe
rem popd

rem exit %var%
