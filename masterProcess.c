#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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

// making folder for fifo
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

// fills out preallocated CHAR ARRAY[40]
void masterToMotor(char *buf, char *nameMotorAxis)
{
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

void motorToConsole(char *buf, char *nameMotorAxis)
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

    // adding return for path
    for (int i = 0; i < strlen(pipeMotor); ++i)
    {
        buf[i] = i;
    }
}

void sendToMotor(int fileDescriptor, float speed)
{
    if (write(fileDescriptor, &speed, sizeof(speed)) == -1)
    {
        printf("Could not write to motor-pipe. Error -> %d\n", errno);
        // fflush(stdout);
        // perror("command.h motorcomm write");
        // writeErrorLog(fileDescriptorlog_err, "command.h: commandMotor write failed");
        exit(-20);
    }
}

void closePipe(int fileDescriptor)
{
    if (close(fileDescriptor) == -1)
    {
        printf("Could not close pipe. Error -> %d\n", errno);

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

    fileDescriptor = open("logs/errorsLogs.txt", O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (fileDescriptor == -1)
    {
        printf("Error while creating an error log file. Errno -> %d \n", errno);
        fflush(stdout);
        perror("logErrorFileCreate()\n");
        exit(-51);
    }

    return fileDescriptor;
}

int main(int argc, char const *argv[])
{
    // sending data to motors

    int fileDescriptorLog;
    int fileDescriptorErrorLog;
    char motorPathX[40];
    char motorPathZ[40];
    masterToMotor(motorPathX, "X");
    masterToMotor(motorPathZ, "Z");

    char motorConsoleX[40];
    char motorConsoleZ[40];
    motorToConsole(motorPathX, "X");
    motorToConsole(motorPathZ, "Z");
    printf("Creating log folder...\n");

    makeFolder("logs");
    fileDescriptorLog = logFileCreate();
    fileDescriptorErrorLog = logFileCreate();

    logWrite(fileDescriptorLog, "Master: Start\n;");

    // int r = strcat(pipeMotor, motorSymbol);
    pid_t pidMotorX = fork();
    if (pidMotorX == -1)
    {
        printf("Failed to fork\n");
        logWrite(fileDescriptorErrorLog, "Master: Failed to fork()\n");
        perror(errno);
        return 10;
    }

    // MotorX process
    // TODO Rewrite motors to hold the current state
    // TODO Motors reset on "T" and stop on "G"
    // TODO Watchdog PIDs
    // TODO

    // TODO Instalation Package

    if (pidMotorX == 0)
    {
        //*Child process code
        logWrite(fileDescriptorLog, "Child MotorX: Start\n;");

        float speed;
        int fileDescriptorXMaster = open("communication/motorProcess_X", O_RDONLY);

        if (fileDescriptorXMaster == -1)
        {
            printf("Could not open FIFO file\n");
            logWrite(fileDescriptorErrorLog, "MotorX: Could not open RD FIFO file\n");

            exit(4);
        }

        int fileDescriptorXConsole = open("communication/motorProcessConsole_X", O_WRONLY);
        if (fileDescriptorXConsole == -1)
        {
            printf("Could not open FIFO file\n");
            logWrite(fileDescriptorErrorLog, "MotorX: Could not open WR FIFO file\n");

            exit(3);
        }

        while (1)
        {
            printf("Child: MotorX reading form Master\n");

            if (read(fileDescriptorXMaster, &speed, sizeof(speed)) == -1)
            {
                printf("Could not read FIFO file\n");
                logWrite(fileDescriptorErrorLog, "MotorX: Could not open WR FIFO file\n");
                perror(errno);
                exit(3);
            }
            red();
            printf("Process MX: MotorX speed -> %f m/s \n", speed);
            logWrite(fileDescriptorLog, "MotorX: Message recievied\n");

            reset();
            sendToMotor(fileDescriptorXConsole, speed);
        }
        closePipe(fileDescriptorXMaster);
        closePipe(fileDescriptorXConsole);
    }
    else
    {
        // parent masterProcess
        pid_t pidMotorZ = fork();
        if (pidMotorZ == -1)
        {

            printf("Failed to fork\n");
            return 11;
        }
        if (pidMotorZ == 0)
        {
            //*MotorZ process
            logWrite(fileDescriptorLog, "Child MotorX: Start\n;");

            float speed;
            int fileDescriptorMaster = open("communication/motorProcess_Z", O_RDONLY);
            if (fileDescriptorMaster == -1)
            {
                printf("Could not open FIFO master-child file\n");
                perror(errno);

                exit(4);
            }

            int fileDescriptorZConsole = open("communication/motorProcessConsole_Z", O_WRONLY);
            if (fileDescriptorZConsole == -1)
            {
                printf("Could not open FIFO console file\n");
                perror(errno);
                exit(3);
            }

            while (1)
            {
                printf("Child: MotorZ reading form Master\n");
                if (read(fileDescriptorMaster, &speed, sizeof(speed)) == -1)
                {
                    printf("Could not read FIFO file\n");
                    logWrite(fileDescriptorErrorLog, "MotorZ: Could not open WR FIFO file\n");
                    perror(errno);
                    exit(3);
                }

                red();
                printf("Child: MotorZ speed -> %f m/s \n", speed);
                reset();
                sendToMotor(fileDescriptorZConsole, speed);
            }
            close(fileDescriptorMaster);
        }

        else
        {
            //! PARENT

            yellow();
            printf("W - Moving in Z axis UP\nS - Moving in Z axis DOWN\nA - Moving in X axis LEFT\nD - moving in X axis RIGHT\n");
            reset();

            // TODO RAW TERMINAL MODE
            // Eliminate need for "enter" etc while inputting

            int userControl;
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

            int fileDescriptorX = open("communication/motorProcess_X", O_WRONLY);
            if (fileDescriptorX == -1)
            {
                printf("Could not open FIFO file\n");
                exit(3);
            }

            int fileDescriptorZ = open("communication/motorProcess_Z", O_WRONLY);
            if (fileDescriptorZ == -1)
            {
                printf("Could not open FIFO file\n");
                exit(4);
            }
            while (1)
            {
                // system("clear");
                printf("Waiting for input\n");
                userControl = getchar();
                // printf("Terminal ready:\n");

                if (userControl == (int)'a')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorX, (float)10);
                    logWrite(fileDescriptorLog, "Master: X++\n");
                }
                else if (userControl == (int)'d')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorX, (float)-10);
                    logWrite(fileDescriptorLog, "Master: X--\n");
                }
                else if (userControl == (int)'w')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorZ, (float)20);
                    logWrite(fileDescriptorLog, "Master: Z++\n");
                }
                else if (userControl == (int)'s')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorZ, (float)-20);
                    logWrite(fileDescriptorLog, "Master: Z--\n");
                }
            }

            // pid_t processID = fork();

            /*fileDescriptor[0] is for READ, fileDescriptor[1] is for WRITE*/
            // mkfifo("myfifo1", 0777); // A classic mkfifo

            /*Waiting for ALL CHILD processes to finish*/
            while (wait(NULL) != -1 || errno != ECHILD)
                ;
        }
    }
    // getting file descriptors for fifo files in communication folder

    return 0;
}
