@echo off
pushd .\bin\
cl.exe /nologo /GR- /EHa- /MT /O2 /FC /Fo:"obj\\" ..\src\*.c user32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib SDL2_image.lib /link /out:sculpture.exe /machine:x64
popd
