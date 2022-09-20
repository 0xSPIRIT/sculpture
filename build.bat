@echo off

set Common_Compiler_Flags=/nologo /Zi /GR- /EHa- /Odi /MT /FC /Fo:"obj\\" /D_CRT_SECURE_NO_WARNINGS
set Common_Linker_Flags=user32.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib

rem @echo off
rem gcc main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\win32_sculpture.exe

pushd bin\

REM Delete all the sculpture_***.pdb's
del sculpture_*.pdb >nul 2>nul

REM Build the game layer (.dll that links into SDL layer)
REM We also set the PDB to a unique filename so visual studio doesn't lock our PDB.
cl.exe %Common_Compiler_Flags% ..\src\game.c %Common_Linker_Flags% /link /incremental:no /PDB:sculpture_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%hr%%time:~3,2%%time:~6,2%.pdb /DLL /out:sculpture.dll

set err=%errorlevel%

REM Firstly, check if the exe file is locked so we don't try to compile unnecessarily,
REM and we still retain the errorlevel from the DLL compilation.
(>>win32_sculpture.exe call;) 2>nul || goto end

REM Build the SDL layer (.exe)
cl.exe %Common_Compiler_Flags% ..\src\main.c %Common_Linker_Flags% SDL2main.lib /link /incremental:no /out:win32_sculpture.exe
set err=errorlevel

popd
   
:end
exit /b %err%
