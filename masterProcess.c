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

//! Variables to change with signals

float resetSpeed = 0;

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
    fflush(stdout);
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
        perror("My error: ");
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

void sigHandlerReset(int signum)
{

    // Return type of the handler function should be void
    printf("\nMotors registered a RESET button\n");
    resetSpeed = 0;
}

void motorProcess(char axis, int fileDescriptorErrorLog, int fileDescriptorLog)
{
    //*Motor process
    logWrite(fileDescriptorLog, "Child Motor: Start\n;");
    srand((unsigned)time(NULL));
    float currentError = 0;
    float currentState = 0;
    float speed;
    char motorProcessPath[50] = "communication/motorProcess_";
    char motorProcessConsolePath[50] = "communication/motorProcess_";
    char helperChar[2];
    int randomness;

    helperChar[0] = axis;
    helperChar[1] = '\0';

    strcat(motorProcessPath, helperChar);
    strcat(motorProcessConsolePath, helperChar);

    printf("motorProcessPath to : %s\n", motorProcessPath);
    printf("motorProcessConsolePath to : %s\n", motorProcessConsolePath);

    // printf("Child: Fifo master->child file location: %s\n", motorProcessPath);
    // printf("Child: Fifo child-userConsole file location: %s\n", motorProcessPath);
    printf("Child: %c Opening a pipe masterToChild\n", axis);

    int fileDescriptorMaster = open(motorProcessPath, O_RDONLY);
    if (fileDescriptorMaster == -1)
    {
        printf("Could not open FIFO master-child file\n");
        perror("My error: ");

        exit(4);
    }

    printf("Child: %c Opening a pipe childToConsole\n", axis);

    int fileDescriptorConsole = open(motorProcessConsolePath, O_WRONLY);
    if (fileDescriptorConsole == -1)
    {
        printf("Could not open FIFO console file\n");
        perror("My error: ");

        exit(3);
    }

    printf("Setting up a non blocking pipe...\n");

    if (fcntl(fileDescriptorMaster, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("Error witch fcntl\n");
        exit(42);
    };

    while (1)
    {

        // printf("Child: Motor%c reading form Master\n", axis);
        currentError = ((float)rand() / (RAND_MAX)) / 10;
        randomness = rand() % 10;
        if (read(fileDescriptorMaster, &speed, sizeof(speed)) == -1)
        {
            if (errno == EAGAIN)
            {
                printf("Pipe is empty\n");
            }
            else
            {
                printf("Could not read FIFO file\n");
                logWrite(fileDescriptorErrorLog, "Motor: Could not open WR FIFO file\n");
                perror("My error: ");

                exit(3);
            }
        }
        randomness = rand() % 10;
        if (randomness >= 5)
        {
            currentState = speed + currentError;
        }
        else
        {
            currentState = speed - currentError;
        }

        // to allow to pass the next if(), if the signal is detected then the fucntion
        // changes resetSpeed to 0 and resets the engine
        printf("Child: Motor%c speed -> %f m/s \n", axis, currentState);
        resetSpeed = currentState;

        signal(SIGUSR1, sigHandlerReset);

        if (currentState == resetSpeed)
        {
            red();
            reset();
            sendToMotor(fileDescriptorConsole, currentState);
        }
        else
        {
            printf("Child: Motor%c RESET\n", axis);
            sendToMotor(fileDescriptorConsole, resetSpeed);
        }
        usleep(400000);
    }
    close(fileDescriptorMaster);
}

int main(int argc, char const *argv[])
{
    // sending data to motors

    int fileDescriptorLog;
    int fileDescriptorErrorLog;
    char motorPathX[40];
    char motorPathZ[40];

    printf("Created masterToMotor\n");
    masterToMotor(motorPathX, "X");
    printf("Created masterToMotor\n");
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

    // // int r = strcat(pipeMotor, motorSymbol);
    pid_t pidMotorX = fork();
    if (pidMotorX == -1)
    {
        printf("Failed to fork\n");
        logWrite(fileDescriptorErrorLog, "Master: Failed to fork()\n");
        perror("The error is: ");
        return 10;
    }

    // // MotorX process
    // // TODO Rewrite motors to hold the current state = done
    // // TODO Motors reset on "T" and stop on "G"
    // // TODO Watchdog PIDs
    // // TODO
    // // TODO Instalation Package

    if (pidMotorX == 0)
    {
        yellow();
        printf("Child: MotorX running...\n");
        reset();
        motorProcess('X', fileDescriptorErrorLog, fileDescriptorLog);
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
            yellow();
            printf("Child: MotorZ running...\n");
            reset();
            motorProcess('Z', fileDescriptorErrorLog, fileDescriptorLog);
        }
        else
        {
            //! PARENT
            yellow();
            printf("Parent: Running...\n");
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

            yellow();
            printf("Parent: Console set to cannonical mode...\n");
            reset();

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
                fflush(stdin);
                system("clear");
                yellow();
                printf("W - Moving in Z axis UP\nS - Moving in Z axis DOWN\nA - Moving in X axis LEFT\nD - moving in X axis RIGHT\n");
                reset();
                printf("Parent: Waiting for input\n");
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
                else if (userControl == (int)'t')
                {
                    red();
                    printf("You typed in %c , THE RESET BUTTON!\n", userControl);
                    reset();

                    // sendToMotor(fileDescriptorZ, (float)0);
                    // sendToMotor(fileDescriptorX, (float)0);
                    kill(pidMotorZ, SIGUSR1);
                    kill(pidMotorX, SIGUSR1);

                    logWrite(fileDescriptorLog, "Master: RESET BUTTON\n");
                }
                else
                {
                    printf("Waiting for input...\n");
                }

                usleep(100000);
            }

            // pid_t processID = fork();

            /*fileDescriptor[0] is for READ, fileDescriptor[1] is for WRITE*/
            // mkfifo("myfifo1", 0777); // A classic mkfifo

            /*Waiting for ALL CHILD processes to finish*/
            while (wait(NULL) != -1 || errno != ECHILD)
                ;
        }
    }
    // // getting file descriptors for fifo files in communication folder

    // return 0;
}
