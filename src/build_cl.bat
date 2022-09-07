@echo off
cl.exe /Zi main.c *.c user32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib SDL2_image.lib
