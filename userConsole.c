#include "functionsFile.h"

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

//*Creating log file
// opens info log

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

void signalHandlerForRESET(int signum)
{
    char PID_motorX_str[255];
    char PID_motorZ_str[255];

    printf("Przed File open!");

    FILE *fp_motorX = fopen("./tmp/PID_motor_X", "r");
    if(fp_motorX == NULL)
   {
      printf("Error! X");    // TODO
      exit(1);             
   }
    FILE *fp_motorZ = fopen("./tmp/PID_motor_Z", "r");

if(fp_motorX == NULL)
   {
      printf("Error! Z");   // TODO
      exit(1);             
   }

    printf("Przed F GETS!");
    fgets(PID_motorX_str, 255, (FILE *)fp_motorX);
    fgets(PID_motorZ_str, 255, (FILE *)fp_motorZ);
    printf("PO F GETS!");

    fclose(fp_motorX);
    fclose(fp_motorZ);

    pid_t PID_motorX = atoi(PID_motorX_str);
    pid_t PID_motorZ = atoi(PID_motorZ_str);
    printf("Przed PID OF MOTOR!");

    printf("PID of motor X %d", PID_motorX); // TODO REMOVE
    printf("PID of motor Z %d", PID_motorZ); // TODO REMOVE

    if (signum == SIGUSR2)
    {
        // TODO R button, reset
        kill(PID_motorX, SIGUSR1);
        kill(PID_motorZ, SIGUSR1);
    }
}

// This is hte "Inspection console" from assignment
int main(int argc, char const *argv[])
{
    printf("Przed signal");

    float currentStateX = 0;
    float currentStateZ = 0;
    float randomError;
    // float randomError = (rand() % 1000) / 100000;
    float messageX;
    float messageZ;
    int fileDescriptorZ;
    int fileDescriptorX;
    int fileDescriptorLog;
    int fileDescriptorErrorLog;
    pid_t watchDogPID;

    fileDescriptorLog = logFileCreate();
    fileDescriptorErrorLog = logErrorFileCreate();
    srand((unsigned)time(NULL));

    pid_t childConsole = fork();

    if (childConsole == -1)
    {

        printf("Failed to fork\n");
        return 11;
    }

    //*CHILD CONSOLE PROCESS FOR REGISTERING INPUT
    if (childConsole == 0)
    {
        struct termios oldTerminal;
        struct termios newTerminal;
        // CANNONICAL TERMINAL
        tcgetattr(STDIN_FILENO, &oldTerminal);
        memcpy(&newTerminal, &oldTerminal, sizeof(struct termios));
        newTerminal.c_cc[VTIME] = 0;
        newTerminal.c_cc[VMIN] = 1;
        // bitwise AND assignment to set all flasgs at once || enable canonical input, enable echo
        newTerminal.c_lflag &= ~(ECHO | ICANON);
        // newTerminal.c_lflag &= ~(ICANON);

        tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);
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
                exit(111);
            }
        }
    }

    printf("UserConsole running...\n");

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
    watchDogPID = readPID("communication/pid_WATCHDOG", fileDescriptorErrorLog);

    while (1)
    {
        system("clear");
        signal(SIGUSR2, signalHandlerForRESET);
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
        printf("Sending a signal to watchdog!\n");
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