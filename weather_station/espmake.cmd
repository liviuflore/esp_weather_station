@echo off

PATH=%PATH%;C:\MinGW\bin;C:\MinGW\msys\1.0\bin

SET PROJECT_NAME=%1
SET PROJECT_DIR=%~dp0

set TOOLKIT_DIR=%PROJECT_DIR%\..\..\esp_toolkit\
PATH=%PATH%;%TOOLKIT_DIR%\xtensa-lx106-elf\bin;

rem generate webpages if building all
IF [%2]==[] (
    %PROJECT_DIR%\src\www\htmltoc.exe %PROJECT_DIR%\src\www\html\ %PROJECT_DIR%\src\www\
)

make %2 %3 %4 %5