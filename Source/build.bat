@echo off

IF NOT EXIST ..\Build MKDIR ..\Build

PUSHD ..\Build
cl.exe /Zi ..\Source\Main.cpp User32.lib Gdi32.lib
cl.exe /Zi ..\Source\Parser.cpp
cl.exe /Zi ..\Source\Test.cpp
POPD