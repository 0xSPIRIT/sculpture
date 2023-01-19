@echo off

set Common_Compiler_Flags=/nologo /O2 /arch:AVX2 /GR- /EHa- /MT /FC /Fo:"obj\\" /DALASKA_RELEASE_MODE /DALASKA_USE_SOFTWARE_RENDERER
set Linker_Flags=user32.lib shell32.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib SDL2_mixer.lib

rem gcc main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\win32_sculpture.exe

if not exist src/main.c goto INVALID_DIR

pushd bin\

REM Delete all the sculpture_***.pdb's
del sculpture_*.pdb >nul 2>nul

REM Build the game layer (.dll that links into SDL layer)
cl.exe %Common_Compiler_Flags% ..\src\game.c %Linker_Flags% /link /incremental:no /DLL /out:sculpture.dll /SUBSYSTEM:WINDOWS

set err=%errorlevel%

REM Firstly, check if the exe file is locked so we don't try to compile unnecessarily,
REM and we still retain the errorlevel from the DLL compilation.
(>>win32_sculpture.exe call;) 2>nul || goto end

REM Build the SDL layer (.exe)
cl.exe %Common_Compiler_Flags% ..\src\main.c %Linker_Flags% SDL2main.lib /link /incremental:no /out:win32_sculpture.exe /NODEFAULTLIB:msvcrt /SUBSYSTEM:WINDOWS

if NOT %errorlevel%==0 (set err=%errorlevel%)

popd
goto end
   
:INVALID_DIR
echo.
echo Invalid current path. Execute from the base directory (one up from src/)
echo.
   
:end
exit /b %err%