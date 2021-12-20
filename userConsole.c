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

//*loggingIN
void logging(int fileDescriptor, char *string)
{
    // get current time
    time_t realTime;
    struct tm *timeLog;

    char *currentTime = malloc(sizeof(timeLog));
    time(&realTime);
    timeLog = localtime(&realTime);

    sprintf(currentTime,
            "[%d-%d-%d %d:%d:%d]",
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
        perror(errno);
        exit(-50);
    }
    flock(fileDescriptor, LOCK_UN);
}

//*Creating log file
// opens info log
int logFileCreate()
{
    int fileDescriptor;

    fileDescriptor = open("logs/logging.log", O_CREAT | O_APPEND | O_WRONLY, 0777);
    if (fileDescriptor == -1)
    {
        printf("Error while creating a log file. Errno -> %d \n", errno);
        fflush(stdout);
        perror("logFileCreate()\n");
        exit(-51);
    }

    return fileDescriptor;
}

float readmessageFromPipe(int fileDescriptor)
{
    float message;
    int readCode;
    readCode = read(fileDescriptor, &message, sizeof(message));

    printf("readCode -> %d and message is recieved\n", readCode);

    if (readCode == -1)
    {
        if (errno == EAGAIN)
        {
            printf("Pipe is empty\n");
            return 0;
        }
        else
        {
            printf("Could not read from pipe for the userCONSOLE. Error -> %d\n", errno);
            fflush(stdout);
            perror("readmessageFromPipe()\n");

            exit(-31);
        }
    }
    return message;
}

int openPipeUserMaster(const char *path)
{
    int fileDescriptor;
    fileDescriptor = open(path, O_RDONLY);

    if (fileDescriptor == -1)
    {
        printf("Could not open FIFO file\n");
        exit(3);
    }
    return fileDescriptor;
}

int main(int argc, char const *argv[])
{
    system("clear");

    printf("UserConsole running...\n");
    srand((unsigned)time(NULL));

    /*fileDescriptor[0] is for READ, fileDescriptor[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo

    int fileDescriptorX;
    fileDescriptorX = openPipeUserMaster("communication/motorProcessConsole_X");

    int fileDescriptorZ;
    fileDescriptorZ = openPipeUserMaster("communication/motorProcessConsole_Z");

    //*Stoping cucking the goddamn pipe!
    if (fcntl(fileDescriptorX, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("Error witch fcntl\n");
        exit(40);
    };
    if (fcntl(fileDescriptorZ, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("Error witch fcntl\n");
        exit(41);
    };

    printf("Pipes opened...\n");

    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    // float randomError = (rand() % 1000) / 100000;
    float messageX;
    float messageZ;

    while (1)
    {
        system("clear");
        randomError = (float)rand() / (RAND_MAX);

        printf("Console: Reading from MotorX\n");
        messageX = readmessageFromPipe(fileDescriptorX);
        fflush(stdout);
        printf("Console: Done1\n");

        printf("Console: Reading from MotorZ\n");
        messageZ = readmessageFromPipe(fileDescriptorZ);
        fflush(stdout);
        printf("Console: Done2\n");

        currentStateX += messageX;
        currentStateZ += messageZ;

        red();
        printf("Current state of the hoist:\n");
        printf("Current random error: %f\n", randomError);

        yellow();
        // if ()
        printf("X: %f\n", currentStateX - randomError);
        printf("Z: %f\n", currentStateZ - randomError);
        reset();
        usleep(100000);
    }

    closePipe(fileDescriptorX);
    closePipe(fileDescriptorZ);

    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}