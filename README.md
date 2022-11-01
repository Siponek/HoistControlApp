# HoistControlApp

## Hoist control simulator
This is a simulation of real-life engine-driven hoist. Main goal is to steer the hoist in X and Z axis with arrow keys. This program simulates user interface and master engine using POSIX techniques such as forks, pipes and signals for inter-process communication in Linux. Also comes with a watchdog that checks heartbeat of other processes and restarts them when necessary.
## Instalation
(Final attempt branch)
Run **install.sh** with optional path where to unzip source files. Beware that script does not check if path is correct.
## Usage
Run **run.sh** o start the simulation. This will start three processes and all of them need be ran in order to start simulation.

## Technicalities of enviroment
Tested on __WSL2__ and Ubuntu 18.04.</br>
If using __WSL2__ (Windows Subsystem for Linux) to run Unix enviroment I suggest mounting filesystem with metadata funtionalities. __WSL2__ does not allow e.g. named pipes in C programming sense to work properly.</br>

For Ubuntu:</br>
sudo umount /mnt/c sudo mount -t drvfs C: /mnt/c -o metadata

## Requirements
GCC at least version 6
