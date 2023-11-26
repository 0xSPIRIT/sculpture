View src\win32_main.c for more information on how the project is laid
out and such.

== Building ==

Ensure you have MSVC installed, as well as clang. Ensure you add
LLVM\bin to the PATH, and that you're running a terminal using the
"x64 Native Tools Command Prompt for VS 2022" or whatever version
you're at.

To build, call build_release.bat from your terminal. An executable
should be generated in bin_release\ and you should be good to go!
Just make sure these relevent DLLs are in the same folder as the
executable:
- libfreetype-6.dll
- libpng16-16.dll
- SDL2.dll
- SDL2_image.dll
- SDL2_mixer.dll
- SDL2_ttf.dll
- zlib1.dll

To build with emscripten (webassembly), ensure you have
emscripten installed and you call emsdk_env.bat. Then, call
build_emscripten.bat. Output for this should be in a new folder
called emscripten\
