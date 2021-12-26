#define true 1
#define false 0
//! Variables to change with signals

float resetSpeed = 0;
int fileDescriptorLog;
int fileDescriptorErrorLog;
extern float resetSpeed;

#include "functionsFile.h"

void sigHandlerReset(int signum);
void motorProcess(char axis, int fileDescriptorErrorLog, int fileDescriptorLog);
void sendToMotor(int fileDescriptor, float speed);
void writeToTXT(char axis);

// void makeFolder(char *dirname);
// void masterToMotor(char *buf, char *nameMotorAxis);
// void motorToConsole(char *buf, char *nameMotorAxis);
// void sendToMotor(int fileDescriptor, float speed);
// void closePipe(int fileDescriptor);
// void logWrite(int fileDescriptor, char *string);
// void watchdogPID_Write(char *pipeName, int state);
// int logFileCreate();
// int logErrorFileCreate();
// void sigHandlerReset(int signum);
// void motorProcess(char axis, int fileDescriptorErrorLog, int fileDescriptorLog);

int main(int argc, char const *argv[])
{
    // sending data to motors
    char motorPathX[40];
    char motorPathZ[40];
    char motorConsoleX[40];
    char motorConsoleZ[40];
    makeFolder("logs");
    fileDescriptorLog = logFileCreate();
    fileDescriptorErrorLog = logErrorFileCreate();

    logWrite(fileDescriptorLog, "Master: Start\n;");

    printf("Created masterToMotor\n");
    masterToMotor(motorPathX, "X");
    printf("Created masterToMotor\n");
    masterToMotor(motorPathZ, "Z");

    motorToConsole(motorPathX, "X");
    motorToConsole(motorPathZ, "Z");
    printf("Creating log folder...\n");

    // // int r = strcat(pipeMotor, motorSymbol);
    pid_t pidMotorX = fork();

    if (pidMotorX == -1)
    {
        printf("Failed to fork\n");
        logWrite(fileDescriptorErrorLog, "Master: Failed to fork()\n");
        perror("The error is: ");
        return 10;
    }

    //* MotorX process
    // TODO Rewrite motors to hold the current state = done
    // TODO Motors reset on "T" and stop on "G"
    // TODO Watchdog PIDs
    // TODO
    // TODO Instalation Package

    if (pidMotorX == 0)
    {
        writeToTXT('X');
        watchdogPID_Write("communication/pid_MotorX", false, fileDescriptorErrorLog);
        yellow();
        printf("Child: MotorX running...\n");
        reset();
        motorProcess('X', fileDescriptorErrorLog, fileDescriptorLog);
    }
    else
    {
        printf("ChildX: Writing PID to watchdog\n");

        logWrite(fileDescriptorLog, "Master: Created a child MotorX\n;");

        // parent masterProcess
        pid_t pidMotorZ = fork();
        if (pidMotorZ == -1)
        {

            printf("Failed to fork\n");
            return 11;
        }
        if (pidMotorZ == 0)
        {
            writeToTXT('Z');

            printf("ChildZ: Writing PID to watchdog\n");

            watchdogPID_Write("communication/pid_MotorZ", false, fileDescriptorErrorLog);

            yellow();
            printf("Child: MotorZ running...\n");
            reset();
            motorProcess('Z', fileDescriptorErrorLog, fileDescriptorLog);
        }
        else
        {
            printf("Parent: Writing PID to watchdog\n");

            watchdogPID_Write("communication/pid_masterProcess", false, fileDescriptorErrorLog);

            //! PARENT
            logWrite(fileDescriptorLog, "Master: Created a child MotorZ\n;");

            yellow();
            printf("Parent: Running...\n");
            reset();

            // TODO RAW TERMINAL MODE = done
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
                yellow();
                printf("W - Moving in Z axis UP\nS - Moving in Z axis DOWN\nA - Moving in X axis LEFT\nD - moving in X axis RIGHT\n");
                reset();
                printf("Parent: Waiting for input\n");
                userControl = getchar();
                // system("clear");
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

    return 0;
}

void sigHandlerReset(int signum)
{
    if (signum = SIGUSR1)
    { // Return type of the handler function should be void
        printf("\nMotors registered a RESET button\n");

        printf("Child: Motor RESET\n");
        sendToMotor(fileDescriptorConsole, 0);
    }
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
    char motorProcessConsolePath[50] = "communication/motorProcessConsole_";
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

    while (1)
    {

        // printf("Child: Motor%c reading form Master\n", axis);\

        //*A random error with motor process
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
        // printf("Child: Motor%c speed -> %f m/s \n", axis, currentState);
        resetSpeed = currentState;

        signal(SIGUSR1, sigHandlerReset);
        printf("currentState -> %f\n", currentState);
        printf("resetSpeed -> %f\n", resetSpeed);

        if (currentState == resetSpeed)
        {
            sendToMotor(fileDescriptorConsole, currentState);
        }
        // else
        // {
        //     printf("Child: Motor%c RESET\n", axis);
        //     sendToMotor(fileDescriptorConsole, resetSpeed);
        // }
        usleep(400000);
    }
    close(fileDescriptorMaster);
}

void sendToMotor(int fileDescriptor, float speed)
{
    if (write(fileDescriptor, &speed, sizeof(speed)) == -1)
    {
        printf("Could not write to motor-pipe. Error -> %d\n", errno);
        fflush(stdout);
        logWrite(fileDescriptorErrorLog, "sendToMotor -> failed to write into a pipe");
        exit(-20);
    }
    fflush(stdout);
}

void writeToTXT(char axis)
{
    makeFolder("tmp");
    pid_t pidOfMotor = getpid();
    char helperChar[2];
    helperChar[0] = axis;
    helperChar[1] = '\0';
    char txtFilePath[20] = "tmp/PID_motor_";
    strcat(txtFilePath, helperChar);

    FILE *fp = fopen(txtFilePath, "w+");
    char pidOfMotorString[100];
    sprintf(pidOfMotorString, "%d", pidOfMotor);
    fputs(pidOfMotorString, fp);
    fclose(fp);
}