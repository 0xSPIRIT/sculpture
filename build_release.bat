@echo off

set Common_Compiler_Flags=/nologo /Z7 /O2 /GR- /GS- /EHa- /MT /FC /Fo:"obj\\" /DALASKA_RELEASE_MODE /D_CRT_SECURE_NO_WARNINGS
set Linker_Flags=user32.lib shell32.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib SDL2_mixer.lib

if not exist src/win32_main.c goto INVALID_DIR

if not exist bin_release\ mkdir bin_release
if not exist bin_release\obj\ mkdir bin_release\obj\

where /q cl.exe
IF ERRORLEVEL 1 (
    call clx64.bat
)

pushd bin_release\

cl.exe %Common_Compiler_Flags% ..\src\win32_main.c ..\src\game.c %Linker_Flags% SDL2main.lib /link /NOIMPLIB /NOEXP /incremental:no /out:alaska.exe /SUBSYSTEM:windows

if NOT %errorlevel%==0 (set err=%errorlevel%)

if not exist res\ mkdir res
rem Copy the resources and its subdirectories
xcopy /q /y /e /k /h /i ..\res\ res\

popd
goto end
   
:INVALID_DIR
echo.
echo Invalid current path. Execute from the base directory (one up from src/)
echo.
   
:end
exit /b %err%
