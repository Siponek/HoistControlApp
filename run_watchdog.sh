#!/bin/bash
rm bin/watchdog.exe

gcc src/watchdog.c -o bin/watchdog.exe
./bin/watchdog.exe

echo return value: $?
