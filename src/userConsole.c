#include "../libs/functionsFile.h"

int openPipeUserMaster(const char *path);
void signalHandlerForRESET(int signum);
void createProcessForUserInteractions(pid_t watchdogPID); // TODO run this by exec()
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
    pid_t watchdogPID = readProcessPIDFromFile("communication/pid_WATCHDOG");
    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    float messageX;
    float messageZ;
    pid_t childProcessInteraction = fork();

    if (childProcessInteraction == -1)
    {
        printf("Failed to create a fork\n");
        perror("Error while creating a fork");
    }

    if (childProcessInteraction == 0)
    {
        createProcessForUserInteractions(watchdogPID);
    }
    else
    {
        // struct sigaction sa;
        // memset(&sa, 0, sizeof(sa));
        // sa.sa_handler = &signalHandlerForRESET;
        // sigaction(SIGUSR1, &sa, NULL);

        while (1)
        {
            system("clear");
            // signal(SIGUSR2, signalHandlerForRESET);
            printf("Console: Reading from MotorX\n");
            messageX = readMessageFromPipe(fileDescriptorX);
            fflush(stdout);
            // printf("Console: Done1\n");

            printf("Console: Reading from MotorZ\n");
            messageZ = readMessageFromPipe(fileDescriptorZ);
            fflush(stdout);
            // printf("Console: Done2\n");
            // * Adding limits to movement
            if (abs(currentStateX + messageX) < 500)
            {
                currentStateX += messageX;
                yellow();
                printf("X: %f\n", currentStateX);
                reset();
            }
            else
            {
                yellow();
                printf("X: %f   Going out of bounds!\n", currentStateZ);
                reset();
            }
            if (abs(currentStateZ + messageZ) < 500)
            {
                currentStateZ += messageZ;
                yellow();
                printf("Z: %f\n", currentStateZ);
                reset();
            }
            else
            {
                yellow();
                printf("Z: %f   Going out of bounds!\n", currentStateZ);
                reset();
            }
            // TODO I think you should remove random error value in userConsole.
            // TODO you are not able to measure it. Currenty it's just a zero all the time.
            // Random error implemented in motor's Processes
            // printf("Current random error: %f\n", randomError);
            // printf("Sending a signal to watchdog!\n");
            // // kill(watchdogPID, SIGUSR1);
            logWrite(fileDescriptorLog, "userConsole: Sending signal to watchdog\n");
            red();
            printf("This is my PID! -> %d\n", getpid());
            reset();

            // for testing (sending PID with signals)
            // // kill(watchDogPID, SIGUSR1);

            usleep(50000);
        }
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

// void signalHandlerForRESET(int signum)
// {
//     int pidMOTORX = readProcessPIDFromFile(".tmp/PID_motor_X");
//     int pidMOTORZ = readProcessPIDFromFile(".tmp/PID_motor_Z");

//     if (signum == SIGUSR2)
//     {
//         // TODO R button, reset
//         printf("sending reset signals to motors\n");
//         // kill(pidMOTORX, SIGUSR1);
//         // kill(pidMOTORZ, SIGUSR1);
//     }
// }

/// Child console process for registering user's input.
void createProcessForUserInteractions(pid_t watchdogPID)
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
        int parentPID = getpid();

        while (1)
        {
            userInput = getchar();
            // kill(watchdogPID, SIGUSR1);
            logWrite(fileDescriptorLog, "userConsoleChild: Sending signal to watchdog\n");
            if (userInput == (int)'r')
            {
                printf("You typed in %c !\n", userInput);
                logWrite(fileDescriptorLog, "userConsole: RESET BUTTON PRESSED\n");
                // kill(parentPID, SIGUSR2);
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

    printf("readCode -> %d and message is received\n", readCode);

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