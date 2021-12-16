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
#include <string.h>

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

void makeFolder(char *dirname)
{
    int check;
    system("clear");
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

char *motorToConsoleFifo(char *nameMotorAxis)
{
    int fileDescriptor;
    makeFolder("communication");
    char pipeMotor[40] = "communication/motorProcessConsole_";
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

    return pipeMotor;
}

// Closes the pipe
void closePipe(int fileDescriptor)
{
    if (close(fileDescriptor) == -1)
    {
        printf("Could not close pipe for userCONSOLE. Error -> %d\n", errno);

        exit(-21);
    }
}

float readmessageFromPipe(int fileDescriptor)
{
    float message;

    if (read(fileDescriptor, &message, sizeof(message)) == -1)
    {
        printf("Could not from pipe for userCONSOLE. Error -> %d\n", errno);

        exit(-31);
    }
    return message;
}

int main(int argc, char const *argv[])
{
    printf("UserConsole running...\n");
    srand((unsigned)time(NULL));

    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo
    int fileDescriptorX = open("communication/motorProcessConsole_X", O_RDONLY);
    if (fileDescriptorX == -1)
    {
        printf("Could not open FIFO file X\n");
        exit(3);
    }

    int fileDescriptorZ = open("communication/motorProcessConsole_Z", O_RDONLY);
    if (fileDescriptorZ == -1)
    {
        printf("Could not open FIFO file Z\n");
        exit(4);
    }
    printf("Pipes opened...\n");

    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    // float randomError = (rand() % 1000) / 100000;
    float messageX;
    float messageZ;

    while (1)
    {
        randomError = (float)rand() / (RAND_MAX);
        messageX = readmessageFromPipe(fileDescriptorX);
        fflush(stdout);

        messageZ = readmessageFromPipe(fileDescriptorZ);
        fflush(stdout);

        currentStateX += messageX;
        currentStateZ += messageZ;

        system("clear");
        red();
        printf("Current state of the hoist:\n");
        printf("Current random error: %f\n", randomError);

        yellow();
        // if ()
        printf("X: %f\n", currentStateX - randomError);
        printf("Z: %f\n", currentStateZ - randomError);
        reset();
        usleep(100);
    }
    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}