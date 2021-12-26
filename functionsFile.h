#pragma once

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <string.h>

#define true 1
#define false 0

void red();
void yellow();
void reset();
void makeFolder(char *dirname);
void masterToMotor(char *buf, char *nameMotorAxis);
void motorToConsole(char *buf, char *nameMotorAxis);
void closePipe(int fileDescriptor);
void logWrite(int fileDescriptor, char *string);
void watchdogPID_Write(char *pipeName, int state, int fileDescriptorErrorLog);
int logFileCreate();
int logErrorFileCreate();
pid_t readPID(char *pipeName, int fileDescriptorErrorLog);
// making folder for fifo

pid_t readPID(char *pipeName, int fileDescriptorErrorLog)
{
    int fileDescriptor;
    pid_t pid;

    // (ignore "file already exists", errno 17)
    if (mkfifo(pipeName, 0777) == -1 && errno != 17)
    {
        printf("Error %d in ", errno);
        fflush(stdout);
        perror("functionsFile.h readPID mkfifo");
        logWrite(fileDescriptorErrorLog, "functionsFile.h: readPID mkfifo failed");
        exit(-1);
    }

    fileDescriptor = open(pipeName, O_RDONLY);
    if (fileDescriptor == -1)
    {
        printf("Error %d in ", errno);
        fflush(stdout);
        perror("functionsFile.h readPID open");
        logWrite(fileDescriptorErrorLog, "functionsFile.h: readPID open failed");
        exit(-1);
    }

    if (read(fileDescriptor, &pid, sizeof(pid)) == -1)
    {
        printf("Error %d in ", errno);
        fflush(stdout);
        perror("functionsFile.h readPID read");
        logWrite(fileDescriptorErrorLog, "functionsFile.h: readPID read failed");
        exit(-1);
    }

    if (close(fileDescriptor) == -1)
    {
        printf("Error %d in ", errno);
        fflush(stdout);
        perror("functionsFile.h readPID close");
        logWrite(fileDescriptorErrorLog, "functionsFile.h: readPID close failed");
        exit(-1);
    }

    return pid;
}

void makeFolder(char *dirname)
{
    int check;
    // system("clear");
    check = mkdir(dirname, 0777);

    // check if directory is created or not
    if (!check)
        printf("Directory %s created\n", dirname);
    else if (errno != EEXIST)
    {
        printf("Could not create directory %s : %d\n", dirname, errno);
        exit(0);
    }
    else
    {
        // TODO there is no MKFIFO directory, you should use [dirname] parameter ==DONE
        printf("FIFO %s directory already exists, running on...\n", dirname);
    }
}

// fills out preallocated CHAR ARRAY[40]
void masterToMotor(char *buf, char *nameMotorAxis)
{
    printf("Creating fifo files for masterToMotor...\n");

    int fileDescriptor;
    makeFolder("communication");
    char pipeMotor[40] = "communication/motorProcess_";
    strcat(pipeMotor, nameMotorAxis);

    printf("This is pipemotor after strcat_s ->  %s\n", pipeMotor);

    if (mkfifo(pipeMotor, 0777) == -1)
    {

        if (errno != EEXIST)
        {
            printf("Could not create FIFO file: %d\n", errno);
            exit(2);
        }
        else
        {
            printf("MKFIFO %s file  already exists, running on...\n", pipeMotor);
        }
    }
    yellow();
    printf("MKFIFO for motor '%s' created\n", nameMotorAxis);
    reset();

    // adding return for path
    for (int i = 0; i < strlen(pipeMotor); ++i)
    {
        buf[i] = i;
    }
}

// Function to append a char to a path for file
void motorToConsole(char *buf, char *nameMotorAxis)
{
    printf("Creating fifo files for motorToConsole...\n");

    int fileDescriptor;
    makeFolder("communication");
    char pipeMotor[40] = "communication/motorProcessConsole_";
    strcat(pipeMotor, nameMotorAxis);

    printf("This is pipemotorConsole after strcat_s ->  %s\n", pipeMotor);

    if (mkfifo(pipeMotor, 0777) == -1)
    {

        if (errno != EEXIST)
        {
            printf("Could not create FIFO file: %d\n", errno);
            exit(2);
        }
        else
        {
            printf("MKFIFO %s file  already exists, running on...\n", pipeMotor);
        }
    }
    yellow();
    printf("MKFIFO for motor '%s' created\n", nameMotorAxis);
    reset();

    // adding return for path
    for (int i = 0; i < strlen(pipeMotor); ++i)
    {
        buf[i] = i;
    }
}

void closePipe(int fileDescriptor)
{
    if (close(fileDescriptor) == -1)
    {
        printf("Could not close pipe. Error -> %d\n", errno);
        perror("The error with closing a pipe\n");

        exit(-21);
    }
}

void logWrite(int fileDescriptor, char *string)
{
    // get current time
    time_t realTime;
    struct tm *timeLog;

    char *currentTime = malloc(sizeof(timeLog));
    time(&realTime);
    timeLog = localtime(&realTime);

    sprintf(currentTime,
            "|%d-%d-%d %d:%d:%d|",
            timeLog->tm_mday,
            1 + timeLog->tm_mon,
            1900 + timeLog->tm_year,
            timeLog->tm_hour,
            timeLog->tm_min,
            timeLog->tm_sec);

    // locking the file
    flock(fileDescriptor, LOCK_EX);
    if (dprintf(fileDescriptor, "%s %s\n", currentTime, string) < 0)
    {
        printf("Log: Cannot write to log. errno : %d\n", errno);
        fflush(stdout);
        perror("My error: ");
        exit(-50);
    }
    flock(fileDescriptor, LOCK_UN);
}

// *This function creates a FIFO file with specified PID of the process
void watchdogPID_Write(char *pipeName, int state, int fileDescriptorErrorLog)
{
    int fileDescriptor;
    pid_t processID = getpid();

    if (mkfifo(pipeName, 0777) == -1 && errno != EEXIST)
    {
        fflush(stdout);
        perror("Problem with mkfifo for watchdogPID");
        logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to create a fifo file");
        exit(-61);
    }
    printf("Opening a watchdog pipe!\n");
    fileDescriptor = open(pipeName, O_WRONLY);
    if (fileDescriptor == -1)
    {
        fflush(stdout);
        perror("Problem with opening a pipe for watchdogPID");
        logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to open a fifo file");
        exit(-62);
    }
    printf("Writing to a watchdog pipe!\n");

    if (write(fileDescriptor, &processID, sizeof(processID)) == -1)
    {
        fflush(stdout);
        perror("Problem with writing to a pipe for watchdogPID");
        logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to write into a fifo file");
        exit(-63);
    }

    if (state)
    {
        if (close(fileDescriptor) == -1)
        {
            fflush(stdout);
            perror("Problem with closing a pipe for watchdogPID");

            logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to close a fifo file");
            exit(-64);
        }
    }
}

void watchdogPIDT_txt(char *pipeName, int state, int fileDescriptorErrorLog)
{
    int fileDescriptor;
    pid_t processID = getpid();

    printf("Opening/creating a watchdog .txt file!\n");
    fileDescriptor = open(pipeName, O_CREAT | O_APPEND | O_WRONLY, 0777);

    if (fileDescriptor == -1)
    {
        fflush(stdout);
        perror("Problem with opening a pipe for watchdogPID");
        logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to open a fifo file");
        exit(-62);
    }
    printf("Writing to a watchdog pipe!\n");

    if (write(fileDescriptor, &processID, sizeof(processID)) == -1)
    {
        fflush(stdout);
        perror("Problem with writing to a pipe for watchdogPID");
        logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to write into a fifo file");
        exit(-63);
    }

    if (state)
    {
        if (close(fileDescriptor) == -1)
        {
            fflush(stdout);
            perror("Problem with closing a pipe for watchdogPID");

            logWrite(fileDescriptorErrorLog, "watchdogPID_Write -> failed to close a fifo file");
            exit(-64);
        }
    }
}

//*Creating log file
// opens info log
int logFileCreate()
{
    int fileDescriptor;
    fileDescriptor = open("logs/logs.txt", O_CREAT | O_APPEND | O_WRONLY, 0777);

    if (fileDescriptor == -1)
    {
        printf("Error while creating a log file. Errno -> %d \n", errno);
        fflush(stdout);
        perror("logFileCreate()\n");
        exit(-51);
    }
    printf("logFileCreate -> %d \n", fileDescriptor);

    return fileDescriptor;
}

int logErrorFileCreate()
{
    int fileDescriptor;
    fileDescriptor = open("logs/errorsLogs.txt", O_CREAT | O_APPEND | O_WRONLY, 0777);
    if (fileDescriptor == -1)
    {
        printf("Error while creating an error log file. Errno -> %d \n", errno);
        fflush(stdout);
        perror("logErrorFileCreate()\n");
        exit(-52);
    }
    printf("logErrorFileCreate -> %d \n", fileDescriptor);

    return fileDescriptor;
}

void red()
{
    printf("\x1b[1;31m");
}

void yellow()
{
    printf("\033[1;33m");
}

void reset()
{
    fflush(stdin);
    printf("\x1b[0m");
    fflush(stdout);
}
