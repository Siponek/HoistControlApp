#!/bin/bash
rm watchdog.exe

gcc watchdog.c -o watchdog.exe
./watchdog.exe

echo return value: $?
