#!/bin/bash
rm masterProcess.exe
rm -r logs

gcc masterProcess.c -o masterProcess.exe
./masterProcess.exe

echo return value: $?
