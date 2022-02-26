#!/bin/bash
rm -rf bin/masterProcess.exe
# rm -rf /logs
mkdir bin
gcc src/masterProcess.c -o bin/masterProcess.exe
./bin/masterProcess.exe

echo return value: $?
