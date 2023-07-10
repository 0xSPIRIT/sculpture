View src\main.c for more information on how the project
is laid out and such.

== Building ==

Ensure you have MSVC installed (I used MSVC 2022, it will
most likely work with previous versions). Install SDL2,
SDL2_ttf, and SDL2_image from their website, add them to
your include and lib directories for MSVC. Open the native
developer console from the Start Menu, and navigate to this
directory with the build.bat or build_release.bat (optimized)
then simply call it. Two binaries will be produced in bin/:
win32_sculpture.exe and sculpture.dll. Double-click
win32_sculpture.exe and run it!

To build with emscripten (webassembly), ensure you have
emscripten installed with emcc.exe in your PATH. Then,
call build_emscripten.bat. Output for this should be in
a new folder called out\

The game should work with GCC as well, but you'll have to
figure out the build line.
