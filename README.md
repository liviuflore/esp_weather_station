# ESP Weather Station
code for small weather station based on esp8266 chip

---- WORK IN PROGRESS ----

There are two VS2013 projects included:
* main project that contains the code that runs on ESP8266 platform
* a test project for testing the web pages, that compiles anr runs on windows

For one to build the ESP project:
* get the esp toolchain (https://github.com/liviuflore/esp_toolchain/)
* install minGW (mingw-developer-toolkit and mingw-base) (http://www.mingw.org/)
* install python 2.7.x (https://www.python.org/) and pySerial (http://pyserial.sourceforge.net/)
* modify espmake.cmd to point to your correct paths.

Sources of inspiration:
http://www.esp8266.com/wiki/doku.php?id=setup-windows-compiler-esp8266

ESP8266 WIKI:
http://www.esp8266.com/wiki/doku.php
