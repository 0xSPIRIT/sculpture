View src\main.c for more information on how the project
is laid out and such.

== Building ==

Ensure you have MSVC installed (I used MSVC 2022, unsure if
it would be compatible with older versions). Install SDL2,
SDL2_ttf, and SDL2_image from their website, add them to
your include and lib directories for MSVC. Open the native
developer console from the Start Menu, and navigate to this
directory with the build.bat or buildsuper.bat (optimized)
then simply call it. Two binaries will be produced in bin/:
win32_sculpture.exe and sculpture.dll. Double-click
win32_sculpture.exe and run it!

The game *should* work with GCC as well. The build file
has a gcc compile line commented out in there.
