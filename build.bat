@echo off
mkdir apps\binaries

if %1 == all goto ALL

:CUSTOM

IF (%1) == () GOTO END
gcc apps\%1.c -o apps\binaries\%1.exe
echo %1
SHIFT
GOTO CUSTOM

:ALL

for %%i IN (apps\*.c) DO gcc %%i -o apps\binaries\%%~ni.exe

:END