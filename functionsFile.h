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

void configTerminal();
void red();
void yellow();
void reset();
void makeFolder(char *dirname);
void createFIFO(char *nameMotorAxis);
void motorToConsole(char *buf, char *nameMotorAxis);
void closePipe(int fileDescriptor);
void logWrite(int fileDescriptor, char *string);
void watchdogPID_Write(char *pipeName, int state, int fileDescriptorErrorLog);
int createFile();
int logErrorFileCreate();
void writeCurrentProcessPIDToFile(char *path);
pid_t readProcessPIDFromFile(char *path);
void watchdogPIDT_txt(char *pipeName, int state, int fileDescriptorErrorLog);

/// Eliminate need for "enter" etc while inputting
void configTerminal()
{
    struct termios oldTerminal;
    struct termios newTerminal;

    tcgetattr(STDIN_FILENO, &oldTerminal);
    memcpy(&newTerminal, &oldTerminal, sizeof(struct termios));
    newTerminal.c_cc[VTIME] = 0;
    newTerminal.c_cc[VMIN] = 1;
    // bitwise AND assignment to set all flasgs at once || enable canonical input, enable echo
    newTerminal.c_lflag &= ~(ECHO | ICANON);
    // newTerminal.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);

    yellow();
    printf("Parent: Console set to cannonical mode...\n");
    reset();
}

void makeFolder(char *dirname)
{
    int result = mkdir(dirname, 0777);

    if (result == 0 || errno == EEXIST) // TODO remove dir if exists and then create
    {
        printf("Directory '%s' created\n", dirname);
    }
    else
    {
        printf("Could not create directory %s : %d\n Exiting...\n", dirname, errno);
        exit(0);
    }
}

// fills out preallocated CHAR ARRAY[40]
void createFIFO(char *pipePath)
{
    printf("Creating FIFO: '%s'\n", pipePath);

    if (mkfifo(pipePath, 0777) == 0 || errno == EEXIST)
    {
        printf("FIFO '%s' has been created.\n", pipePath);
    }
    else
    {
        printf("Could not create FIFO file `%s`: %d\n Exiting...\n", pipePath, errno);
        exit(1);
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

void writeCurrentProcessPIDToFile(char *path)
{
    pid_t pidOfMotor = getpid();

    FILE *fp = fopen(path, "w+");
    char pidOfMotorString[100];
    sprintf(pidOfMotorString, "%d", pidOfMotor);
    fputs(pidOfMotorString, fp);
    fclose(fp);
}

pid_t readProcessPIDFromFile(char *path)
{
    char PID_str[255];
    int PID_int[10];
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("Error! Marcin coś spierdolił\n"); // TODO
        perror("Error while opening a path with fopen readProcessPIDFromFile");
        exit(1);
    }

    fgets(PID_str, 255, (FILE *)fp);

    fclose(fp);

    pid_t pid = atoi(PID_str);
    return pid;
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
        printf("Log: Cannot write to log. Errno : %d\n", errno);
        exit(1);
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

int createFile(char *path)
{
    int fileDescriptor = open(path, O_CREAT | O_APPEND | O_WRONLY, 0777);

    if (fileDescriptor == -1)
    {
        printf("Error while creating a file '%s'. Errno -> %d \n Exiting...\n", path, errno);
        perror("Error perror");
        exit(1);
    }
    else
    {
        printf("File `%s` has been created.\n", path);
        return fileDescriptor;
    }
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
