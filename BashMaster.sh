#!/bin/bash
rm masterProcess.exe
gcc masterProcess.c -o masterProcess.exe
./masterProcess.exe

echo return value: $?
