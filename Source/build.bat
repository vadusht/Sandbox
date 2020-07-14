@echo off

IF NOT EXIST ..\Build MKDIR ..\Build

PUSHD ..\Build
cl.exe /Zi ..\Source\Platform.cpp User32.lib Gdi32.lib Winmm.lib
cl.exe /Zi /LD ..\Source\Game.cpp 
REM cl.exe /Zi ..\Source\Parser.cpp
REM cl.exe /Zi ..\Source\Test.cpp
POPD