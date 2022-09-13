@echo off

set Common_Compiler_Flags=/nologo /Zi /MT /Odi /FC /Fo:"obj\\"
set Common_Linker_Flags=user32.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib

rem @echo off
rem gcc booter/win32__main.c *.c -Wall -pedantic -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_gfx -g -o ..\bin\sculpture.exe

pushd ..\bin

REM Silence the error output from this.
del *.pdb >nul 2>nul

REM Build the game layer (.dll that links into SDL layer)
REM We also set the PDB to a unique filename so visual studio doesn't lock our PDB.
cl.exe %Common_Compiler_Flags% ..\src\*.c %Common_Linker_Flags% /link /incremental:no /PDB:sculpture_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%hr%%time:~3,2%%time:~6,2%.pdb /DLL /out:sculpture.dll

set err=%errorlevel%

REM Build the SDL layer (.exe)
REM Firstly, check if the file is locked so we don't try to compile unnecessarily,
REM and we still retain the errorlevel from the DLL compilation.
(>>win32_sculpture.exe call;) 2>nul || goto end

cl.exe %Common_Compiler_Flags% ..\src\boot\*.c %Common_Linker_Flags% SDL2main.lib /link /incremental:no /out:win32_sculpture.exe
set err=errorlevel

popd
   
:end
exit %err%
