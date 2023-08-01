View src\main.c for more information on how the project is laid
out and such.

== Building ==

Ensure you have MSVC installed, as well as clang. Ensure you add
LLVM\bin to the PATH, and that you're running a cmd.exe using the
"x64 Native Tools Command Prompt for VS 2022" or whatever version
you're at.

To build, call build_release.bat from your terminal. an executable
should be generated in bin_release\ and you should be good to go!

If you want AVX2 support which makes things faster you can add
'/arch:AVX2' to the Common_Compiler_Flags compiler options.

To build with emscripten (webassembly), ensure you have
emscripten installed with emcc.exe in your PATH. Then, call
build_emscripten.bat. Output for this should be in a new folder
called out\
