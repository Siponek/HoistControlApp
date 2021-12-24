#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include "functionsFile.h"

#define watchdogTime 60
#define true 1
#define false 0
pid_t pid_userConsole;
int fileDescriptorLog;
int fileDescriptorErrorLog;
float resetSpeed;

int main(int argc, char const *argv[])
{

    char *pipeNameInspector = "communication/PID_inspector";

    fileDescriptorLog = logFileCreate();
    // closePipe(fileDescriptorLog);
    fileDescriptorErrorLog = logErrorFileCreate();

    pid_t MotorX;
    pid_t MotorZ;
    pid_t masterProcess;
    pid_t userConsole;

    MotorX = readPID("communication/pid_MotorX", fileDescriptorErrorLog);
    MotorZ = readPID("communication/pid_MotorZ", fileDescriptorErrorLog);
    masterProcess = readPID("communication/pid_masterProcess", fileDescriptorErrorLog);
    // userConsole = readPID("communication/pid_userConsole", fileDescriptorErrorLog);

    printf("PIDs have been read:\n");
    printf("MotorX %d\n", MotorX);
    printf("MotorZ %d\n", MotorZ);
    printf("masterProcess %d\n", masterProcess);
    // printf("userConsole %d\n", userConsole);

    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo

    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}