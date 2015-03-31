@echo off
set TOOLKIT_DIR=D:\_code\esp\toolkit
SET PROJECT_NAME=%1
SET PROJECT_DIR=%~dp0

PATH=%PATH%;%TOOLKIT_DIR%\xtensa-lx106-elf\bin;C:\MinGW\bin;C:\MinGW\msys\1.0\bin

rem generate webpages
%PROJECT_DIR%\src\www\htmltoc.exe .\src\www\html\ .\src\www\

make %2 %3 %4 %5