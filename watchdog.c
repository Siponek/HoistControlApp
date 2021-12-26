#include "functionsFile.h"

#define watchdogTime 60
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

    char *pipeNameInspector = "communication/PID_inspector";
    int aliveMotorX;
    int aliveMotorZ;
    int aliveMaster;
    int aliveUserConsole;

    // timming
    time_t secondsMotorX = time(NULL);
    time_t secondsMotorZ = time(NULL);
    time_t secondsMaster = time(NULL);
    time_t secondsUserConsole = time(NULL);
    time_t timeThreshold = 30;
    time_t currentTime;

    fileDescriptorLog = logFileCreate();
    // closePipe(fileDescriptorLog);
    fileDescriptorErrorLog = logErrorFileCreate();

    pid_t MotorX;
    pid_t MotorZ;
    pid_t masterProcess;
    pid_t userConsole;
    printf("Watchdoing writing its PID to a file...:\n");
    watchdogPID_Write("communication/pid_WATCHDOG", 0, fileDescriptorErrorLog);

    MotorX = readPID("communication/pid_MotorX", fileDescriptorErrorLog);
    MotorZ = readPID("communication/pid_MotorZ", fileDescriptorErrorLog);
    masterProcess = readPID("communication/pid_masterProcess", fileDescriptorErrorLog);
    // userConsole = readPID("communication/pid_userConsole", fileDescriptorErrorLog);

    printf("PIDs have been read:\n");
    printf("MotorX %d\n", MotorX);
    printf("MotorZ %d\n", MotorZ);
    printf("masterProcess %d\n", masterProcess);
    // printf("userConsole %d\n", userConsole);
    struct sigaction sa;
    printf("Watchdog is now waiting for signals and printing them below:\n");

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
            secondsMotorX = time(NULL);
        }
        else if (signalPid == MotorZ)
        {
            secondsMotorZ = time(NULL);
        }
        else if (signalPid == masterProcess)
        {
            secondsMaster = time(NULL);
        }
        else if (signalPid == userConsole)
        {
            secondsUserConsole = time(NULL);
        }

        currentTime = time(NULL);

        if (currentTime - secondsMotorX >= timeThreshold &&
            currentTime - secondsMotorZ >= timeThreshold &&
            currentTime - secondsMaster >= timeThreshold &&
            currentTime - secondsMotorX >= timeThreshold)
        {
            //* this is sending a signal to reset all of the required programs
            printf("watchdog reset userConsole");
            kill(userConsole, SIGUSR2);
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
