#!/bin/bash

gcc masterProcess.c -o masterProcess.exe
./masterProcess.exe

echo return value: $?
