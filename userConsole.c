#include <sys/types.h> //
#include <sys/wait.h>  //
#include <sys/stat.h>  //
#include <sys/file.h>
#include <stdio.h>   //
#include <stdlib.h>  //
#include <unistd.h>  //
#include <signal.h>  //
#include <fcntl.h>   //
#include <errno.h>   //
#include <string.h>  //
#include <time.h>    //
#include <math.h>    //
#include <termios.h> //

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

void consoleFifo(char axis)
{
    float currentError = 0;
    float currentState = 0;
    float speed;
    char motorProcessConsolePath[50] = "communication/motorProcess_";
    char helperChar[2];

    helperChar[0] = axis;
    helperChar[1] = '\0';

    strcat(motorProcessConsolePath, helperChar);

    printf("motorProcessConsolePath to : %s\n", motorProcessConsolePath);

    // printf("Child: Fifo master->child file location: %s\n", motorProcessPath);
    // printf("Child: Fifo child-userConsole file location: %s\n", motorProcessPath);

    printf("Child: %c Opening a pipe childToConsole\n", axis);

    int fileDescriptorConsole = open(motorProcessConsolePath, O_RDONLY);
    if (fileDescriptorConsole == -1)
    {
        printf("Could not open FIFO console file\n");
        perror("My error: ");

        exit(3);
    }
}

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
        perror("This is the error: ");
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

    printf("readCode -> %d and message is recieved", readCode);

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

// int openPipeUserMaster(const char *path)
// {
//     int fileDescriptor;
//     char motorProcessPath[50];
//     strcat(motorProcessPath, "communication//motorProcessConsole_X");
//     printf("Przed open, to jest motorProcessPath: %s\n", motorProcessPath);
//     fileDescriptor = open("./communication/motorProcessConsole_X\0", O_RDONLY);

//     printf("Po open\n fileDescriptor to %d", fileDescriptor);

//     if (fileDescriptor == -1)
//     {
//         printf("Could not open FIFO file\n");
//         perror("This is the error");
//         exit(3);
//     }
//     return fileDescriptor;
// }

int openPipeUserMaster(const char *path)
{
    int fileDescriptor;
    fileDescriptor = open(path, O_RDONLY | O_NONBLOCK);

    if (fileDescriptor == -1)
    {
        printf("Could not open FIFO file\n");
        perror("The error with opening a pipe\n");
        exit(3);
    }
    printf("Pipe opened\n");

    return fileDescriptor;
}

int main(int argc, char const *argv[])
{
    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    // float randomError = (rand() % 1000) / 100000;
    float messageX;
    float messageZ;
    int fileDescriptorZ;
    int fileDescriptorX;

    printf("UserConsole running...\n");

    srand((unsigned)time(NULL));

    /*fileDescriptor[0] is for READ, fileDescriptor[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo
    printf("Making a FIFO X\n");

    printf("Making a FIFO Z\n");

    printf("Opening a pipe1 Main\n");
    fileDescriptorX = openPipeUserMaster("communication/motorProcessConsole_X");

    printf("Opening a pipe2 Main\n");
    fileDescriptorZ = openPipeUserMaster("communication/motorProcessConsole_Z");
    printf("Pipes opened!\n");

    //*Stoping cucking the goddamn pipe!
    // if (fcntl(fileDescriptorX, F_SETFL, O_NONBLOCK) == -1)
    // {
    //     printf("Error witch fcntl\n");
    //     exit(40);
    // };
    // if (fcntl(fileDescriptorZ, F_SETFL, O_NONBLOCK) == -1)
    // {
    //     printf("Error witch fcntl\n");
    //     exit(41);
    // };

    printf("Pipes opened...\n");

    while (1)
    {
        system("clear");

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
        printf("X: %f\n", currentStateX);
        printf("Z: %f\n", currentStateZ);
        reset();
        usleep(50000);
    }

    closePipe(fileDescriptorX);
    closePipe(fileDescriptorZ);

    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}