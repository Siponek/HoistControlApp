#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <string.h>


int isInputValid(char *userInput)
{
    regex_t regex;

    int compiledSuccessfully = regcomp(&regex, "^[xz][-]?[0-9]+$", REG_EXTENDED);

    if (compiledSuccessfully == 0)
    {
        printf("Regex: expression compiled successfully.\n");
        int isFound = regexec(&regex, userInput, 0, NULL, 0);
        if (isFound == 0)
        {
            printf("Regex: Found.\n");
            return 0;
        }
        else
        {
            printf("Regex: Not found.\n");
            return 1;
        }
    }
    else
    {
        printf("Regex: Compilation error.\n");
        return 1;
    }
}
void red()
{
    printf("\x1b[32m");
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

void createPipe(char *pipeName)
{
    if (mkfifo(pipeName, 0777) == -1)
    {

        if (errno != EEXIST)
        {
            printf("Could not create FIFO file: %d\n", errno);
            exit(0);
        }
        else
        {
            printf("MKFIFO file already exists, running on...\n");
        }
    }
    yellow();
    printf("MKFIFO created\n");
    reset();
}

int openPipeWrite(char *pipeName)
{
    int fileDescriptor = open(pipeName, O_WRONLY);
    if (fileDescriptor >= 0)
    {
        return fileDescriptor;
    }
    else
    {
        printf("Could not open FIFO file\n");
        exit(0);
    }
}

// void writeToPipe(char *pipe, char *moveSize)
// {
//     int fileDescriptor = openPipe(pipe);
//     write(fileDescriptor, moveSize, strlen(moveSize) + 1);
//     close(fileDescriptor);
//     return NULL;
// }

void writeToPipe(char *pipe, char *moveSize)
{
    int fileDescriptor = openPipeWrite(pipe);
    write(fileDescriptor, moveSize, strlen(moveSize) + 1);
    close(fileDescriptor);
}

// THIS IS MAIN
// TODO MotorZ, USER-READ CONSOLE, WATCHDOG, START/STOP for Client (signal?)
int main(int argc, char const *argv[])
{
    char *pipeX = "x";
    char *pipeZ = "z";
    char *pipeConfig = "config";

    //createPipe(pipeZ);

    // FUNKCJE DO U¯YCIA - fd = open(), close(fd) write(fd, &do czego, sizeof()), read(fd, &do czego, sizeof())
    char userInput[100];
    char *motorSymbol = malloc(2*sizeof(char));
    int lengthOfMove;
    char moveSize[100];


    createPipe(pipeX);

    while (1)
    {
        yellow();
        printf("Please type in x,z with number e.g. - 'x10', 'x-10', 'z20', `z-10` \n");
        reset();
        scanf("%s", userInput);
        fflush(stdin);

        printf("This is input from scanf %s\n", userInput);
        fflush(stdout);

        /* Alloc 2 chars mem, manual assignment of fist
        char of literal format_string. Manual add of '\0' at the end. Free mem after write.
        */
        motorSymbol[0] = userInput[0];
        motorSymbol[1] = '\0';
        lengthOfMove = sizeof(userInput) / (sizeof(char)) - 1;

        if (userInput[1] == '-')
        {
            memcpy(moveSize, &userInput[2], lengthOfMove - 1);
        }
        else
        {
            memcpy(moveSize, &userInput[1], lengthOfMove);
        }
        //printf("userInput[1] -> %s\n", strcat(strncpy(str, userInput, 1), '\\0'));
        printf("motorSymbol -> %s\n", motorSymbol);
        printf("moveSize -> %s\n", moveSize);
        printf("strlen(userInput) -> %ld\n", strlen(userInput));


        writeToPipe(motorSymbol, moveSize);
        if (isInputValid(userInput) == 1)
        {
            printf("Invalid input.");
            continue;
        }
        fflush(stdin);
        // if (userInput[0] == 'q')
        //     exit(EXIT_SUCCESS);
    }
    free(motorSymbol);

    /*fd[0] is for READ, fd[1] is for WRITE*/
    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}
