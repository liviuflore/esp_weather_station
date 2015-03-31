@echo off

set TOOLKIT_DIR=D:\_code\esp\toolkit
PATH=%PATH%;%TOOLKIT_DIR%\xtensa-lx106-elf\bin;C:\MinGW\bin;C:\MinGW\msys\1.0\bin


SET PROJECT_NAME=%1
SET PROJECT_DIR=%~dp0

rem generate webpages
%PROJECT_DIR%\src\www\htmltoc.exe %PROJECT_DIR%\src\www\html\ %PROJECT_DIR%\src\www\

make %2 %3 %4 %5