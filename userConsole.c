#include "functionsFile.h"

int openPipeUserMaster(const char *path);
void signalHandlerForRESET(int signum);
void createProcessForUserInteractions();
float readMessageFromPipe(int fileDescriptor);

int fileDescriptorLog;
int fileDescriptorErrorLog;

// This is hte "Inspection console" from assignment
int main(int argc, char const *argv[])
{
    fileDescriptorLog = createFile("logs/logs.txt");
    fileDescriptorErrorLog = createFile("logs/errorsLogs.txt");
    srand((unsigned)time(NULL));

    printf("Opening a pipe X\n");
    int fileDescriptorX = openPipeUserMaster("communication/motorProcessConsole_X");

    printf("Opening a pipe Z\n");
    int fileDescriptorZ = openPipeUserMaster("communication/motorProcessConsole_Z");

    // pid_t watchDogPID = readPID("communication/pid_WATCHDOG", fileDescriptorErrorLog); // TODO

    writeCurrentProcessPIDToFile("tmp/PID_userConsole");

    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    float messageX;
    float messageZ;
    while (1)
    {
        system("clear");
        signal(SIGUSR2, signalHandlerForRESET);
        printf("Console: Reading from MotorX\n");
        messageX = readMessageFromPipe(fileDescriptorX);
        fflush(stdout);
        printf("Console: Done1\n");

        printf("Console: Reading from MotorZ\n");
        messageZ = readMessageFromPipe(fileDescriptorZ);
        fflush(stdout);
        printf("Console: Done2\n");

        currentStateX += messageX;
        currentStateZ += messageZ;

        red();
        // TODO I think you should remove random error value in userConsole.
        // TODO you are not able to measure it. Currenty it's just a zero all the time.
        printf("Current random error: %f\n", randomError);
        yellow();
        printf("X: %f\n", currentStateX);
        printf("Z: %f\n", currentStateZ);
        reset();
        // printf("Sending a signal to watchdog!\n");
        printf("This is my PID! -> %d\n", getpid());

        // for testing (sending PID with signals)
        // kill(watchDogPID, SIGUSR1);

        usleep(50000);
    }

    closePipe(fileDescriptorX);
    closePipe(fileDescriptorZ);

    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}

int openPipeUserMaster(const char *path)
{
    int fileDescriptor = open(path, O_RDONLY | O_NONBLOCK);

    if (fileDescriptor == -1)
    {
        printf("Could not open FIFO file\n");
        perror("The error with opening a pipe\n");
        exit(1);
    }
    printf("Pipe opened\n");

    return fileDescriptor;
}

void signalHandlerForRESET(int signum)
{
    if (signum == SIGUSR2)
    {
        // TODO R button, reset
        kill(readProcessPIDFromFile(".tmp/PID_motor_X"), SIGUSR1);
        kill(readProcessPIDFromFile(".tmp/PID_motor_Z"), SIGUSR1);
    }
}

/// Child console process for registering user's input.
void createProcessForUserInteractions()
{
    pid_t childConsole = fork();

    if (childConsole == -1)
    {
        printf("Failed to fork\n");
        exit(1);
    }

    else if (childConsole == 0)
    {
        configTerminal();

        int userInput;
        while (1)
        {
            userInput = getchar();
            if (userInput == (int)'r')
            {
                printf("You typed in %c !\n", userInput);
                logWrite(fileDescriptorLog, "userConsole: RESET BUTTON PRESSED\n");
                // kill(getppid(), SIGUSR2);
                signalHandlerForRESET(SIGUSR2);
            }
            if (userInput == (int)'x')
            {
                printf("You typed in %c !\n", userInput);
                logWrite(fileDescriptorLog, "userConsole: watned to close terminal\n");
                exit(1);
            }
        }
    }
    else
    {
        return;
    }
}

float readMessageFromPipe(int fileDescriptor)
{
    float message;
    int readCode = read(fileDescriptor, &message, sizeof(message));

    printf("readCode -> %d and message is received", readCode);

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
            exit(1);
        }
    }
    return message;
}