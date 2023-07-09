@echo off

REM  A windows utility to replace the really slow vcvarsall.bat
REM  This sets up the current environment for you.
REM 
REM  You probably won't have the same exact visual studio and
REM  windows kits directories as me, so figure those out by
REM  going into the directories yourself. All you need to do
REM  is change the four variables at the top of this file, and
REM  you're good to go!
REM
REM  To find your MSVC_VERSION and WINDOWS_KIT_VERSION, look
REM  at the Community\VC\Tools\MSVC\ directory, and for the
REM  windows kits, look at the \Lib\ directory. In both those
REM  folders it should have the correct version names as folders.
REM  Paste 'em into the variables and you should be good!
REM
REM  What I like to do is start my terminal with this automatically.
REM  Something like:  cmd.exe /k clx64.bat

set MSVC_PATH=C:\Program Files\Microsoft Visual Studio\2022\
set MSVC_VERSION=14.31.31103
set WINDOWS_KIT_PATH=C:\Program Files (x86)\Windows Kits\10\
set WINDOWS_KIT_VERSION=10.0.19041.0

set LIB=%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\lib\x64;^
%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\atlmfc\lib\x64;^
%WINDOWS_KIT_PATH%Lib\%WINDOWS_KIT_VERSION%\um\x64;^
%WINDOWS_KIT_PATH%Lib\%WINDOWS_KIT_VERSION%\ucrt\x64;^
%WINDOWS_KIT_PATH%Lib\%WINDOWS_KIT_VERSION%\ucrt_enclave\x64

set LIBPATH=%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\lib\x64;^
%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\atlmfc\lib\x64

set INCLUDE=c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\%MSVC_VERSION%\include;^
%WINDOWS_KIT_PATH%Include\%WINDOWS_KIT_VERSION%\um\;^
%WINDOWS_KIT_PATH%Include\%WINDOWS_KIT_VERSION%\ucrt\;^
%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\atlmfc\include;^
%WINDOWS_KIT_PATH%Include\%WINDOWS_KIT_VERSION%\shared

set PATH=%PATH%;^
%MSVC_PATH%Community\VC\Tools\MSVC\%MSVC_VERSION%\bin\Hostx64\x64;^
%WINDOWS_KIT_PATH%bin\%WINDOWS_KIT_VERSION%\x64\;^
%WINDOWS_KIT_PATH%Debuggers\x64

echo Successfully set up the terminal for MSVC x64 C/C++ development.
