#!/bin/bash
rm bin/masterProcess.exe
rm -r /logs

gcc src/masterProcess.c -o bin/masterProcess.exe
./bin/masterProcess.exe

echo return value: $?
