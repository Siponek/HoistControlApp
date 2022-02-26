#!/bin/bash
rm bin/userConsole.exe

gcc src/userConsole.c -o bin/userConsole.exe
./bin/userConsole.exe

echo return value: $?
