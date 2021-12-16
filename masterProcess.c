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
        // writeErrorLog(fdlog_err, "command.h: commandMotor write failed");
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

int main(int argc, char const *argv[])
{
    // sending data to motors
    char motorPathX[40];
    char motorPathZ[40];
    masterToMotor(motorPathX, "X");
    masterToMotor(motorPathZ, "Z");

    char motorConsoleX[40];
    char motorConsoleZ[40];
    motorToConsole(motorPathX, "X");
    motorToConsole(motorPathZ, "Z");

    // int r = strcat(pipeMotor, motorSymbol);
    pid_t pidMotorX = fork();
    if (pidMotorX == -1)
    {

        printf("Failed to fork\n");
        return 10;
    }

    // MotorX process
    if (pidMotorX == 0)
    {
        //*Child process code

        float speed;
        int fileDescriptorXMaster = open("communication/motorProcess_X", O_RDONLY);

        if (fileDescriptorXMaster == -1)
        {
            printf("Could not open FIFO file\n");
            exit(4);
        }

        int fileDescriptorXConsole = open("communication/motorProcessConsole_X", O_WRONLY);
        if (fileDescriptorXConsole == -1)
        {
            printf("Could not open FIFO file\n");
            exit(3);
        }

        while (1)
        {

            read(fileDescriptorXMaster, &speed, sizeof(speed));
            red();
            printf("Process MX: MotorX speed -> %f m/s \n", speed);
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
            float speed;
            int fileDescriptor = open("communication/motorProcess_Z", O_RDONLY);
            if (fileDescriptor == -1)
            {
                printf("Could not open FIFO file\n");
                exit(4);
            }

            int fileDescriptorZConsole = open("communication/motorProcessConsole_Z", O_WRONLY);
            if (fileDescriptorZConsole == -1)
            {
                printf("Could not open FIFO file\n");
                exit(3);
            }

            while (1)
            {

                read(fileDescriptor, &speed, sizeof(speed));
                red();
                printf("Process MZ: Z speed -> %f m/s \n", speed);
                reset();
                sendToMotor(fileDescriptorZConsole, speed);
            }
            close(fileDescriptor);
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
                printf("GIVE ME INPUTTTT11!1!!11\n");
                userControl = getchar();
                // printf("Terminal ready:\n");

                system("clear");

                if (userControl == (int)'a')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorX, 10);
                }
                else if (userControl == (int)'d')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorX, -10);
                }
                else if (userControl == (int)'w')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorZ, 20);
                }
                else if (userControl == (int)'s')
                {
                    printf("You typed in %c !\n", userControl);
                    sendToMotor(fileDescriptorZ, -20);
                }
            }

            // pid_t processID = fork();

            /*fd[0] is for READ, fd[1] is for WRITE*/
            // mkfifo("myfifo1", 0777); // A classic mkfifo

            /*Waiting for ALL CHILD processes to finish*/
            while (wait(NULL) != -1 || errno != ECHILD)
                ;
        }
    }
    // getting file descriptors for fifo files in communication folder

    return 0;
}
