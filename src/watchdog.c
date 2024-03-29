#include "../libs/functionsFile.h"

#define timeThreshold 60
#define True 1
#define False 0

pid_t pid_userConsole;
int fileDescriptorLog;
int fileDescriptorErrorLog;
float resetSpeed;

static volatile int signalPid = -1;

void signalHandler(int signum);
void get_pid(int sig, siginfo_t *info, void *context);

int main(int argc, char const *argv[])
{
    time_t lastTimeMotorXResponded = time(NULL);
    time_t lastTimeMotorZResponded = time(NULL);
    time_t lastTimeMasterResponded = time(NULL);
    time_t lastTimeUserConsoleResponded = time(NULL);

    makeFolder("logs");
    makeFolder("communication");
    makeFolder("tmp");

    fileDescriptorLog = createFile("logs/logs.txt");
    fileDescriptorErrorLog = createFile("logs/errorsLogs.txt");

    watchdogPIDT_txt("communication/pid_WATCHDOG", 0, fileDescriptorErrorLog);

    // watchdogPID_Write("communication/pid_WATCHDOG", 0, fileDescriptorErrorLog);

    pid_t MotorX = readProcessPIDFromFile("./tmp/PID_motor_X");
    printf("./tmp/PID_motor_X DONE\n");
    pid_t MotorZ = readProcessPIDFromFile("./tmp/PID_motor_Z");
    printf("./tmp/PID_motor_Z DONE\n");
    pid_t masterProcess = readProcessPIDFromFile("./tmp/PID_masterProcess");
    printf("./tmp/PID_masterProcess DONE\n");
    pid_t userConsole = readProcessPIDFromFile("./tmp/PID_userConsole");
    printf("./tmp/PID_userConsole DONE\n");

    // TODO remove prints below
    printf("PIDs have been read:\n");
    printf("MotorX %d\n", MotorX);
    printf("MotorZ %d\n", MotorZ);
    printf("masterProcess %d\n", masterProcess);
    printf("userConsole %d\n", userConsole);

    struct sigaction sa;
    printf("Watchdog is now waiting for signals and printing them below:\n");

    time_t currentTime;
    while (1)
    {
        printf("PID from void getpid() %d", getpid()); // display PID for kill()
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = get_pid;
        sigaction(SIGUSR1, &sa, NULL);
        pause(); // wait for a signal
        printf("PID of signal sender = %d\n", signalPid);

        // A set of IF statments checking if a signal has been recieved and reseting a waiting time for process to respond
        if (signalPid == MotorX)
        {
            lastTimeMotorXResponded = time(NULL);
            logWrite(fileDescriptorLog, "Watchdog: Recieved MotorX signal");
        }
        else if (signalPid == MotorZ)
        {
            lastTimeMotorZResponded = time(NULL);
            logWrite(fileDescriptorLog, "Watchdog: Recieved MotorZ signal");
        }
        else if (signalPid == masterProcess)
        {
            lastTimeMasterResponded = time(NULL);
            logWrite(fileDescriptorLog, "Watchdog: Recieved masterProcess signal");
        }
        else if (signalPid == userConsole)
        {
            lastTimeUserConsoleResponded = time(NULL);
            logWrite(fileDescriptorLog, "Watchdog: Recieved userConsole signal");
        }

        currentTime = time(NULL);

        if (currentTime - lastTimeMotorXResponded >= timeThreshold &&
            currentTime - lastTimeMotorZResponded >= timeThreshold &&
            currentTime - lastTimeMasterResponded >= timeThreshold &&
            currentTime - lastTimeMotorXResponded >= timeThreshold)
        {
            //* this is sending a signal to reset all of the required programs
            printf("watchdog reset userConsole");
            logWrite(fileDescriptorLog, "Watchdog: Reseting processes");

            // kill(userConsole, SIGUSR2);
        }
    }
    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo

    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}

// void signalHandler(int signum)
// {
//     if (signum == SIGUSR1)
//     {
//         // printf("Watchdog: RESET signal interrupted\n");
//         // fflush(stdout);
//         isKiller = false;
//     }

//     if (signum == SIGUSR2)
//     {
//         // printf("Watchdog: RESET signal interrupted\n");
//         // fflush(stdout);
//         isKiller = false;
//         writePID("tmp/PID_watchdog", false); // for motorx
//         writePID("tmp/PID_watchdog", true);  // for motorz
//     }
// }

void get_pid(int sig, siginfo_t *info, void *context)
{

    signalPid = info->si_pid;
}
