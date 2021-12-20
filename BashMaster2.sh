#!/bin/bash
rm userConsole.exe

gcc userConsole.c -o userConsole.exe
./userConsole.exe

echo return value: $?
