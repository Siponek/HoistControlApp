#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


void red()
{
    printf("\x1b[33m");
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
    if (mkfifo("/tmp/myfifo", 0777) == -1)
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

int openPipe(char *pipeName)
{
    int fileDescriptor = open(pipeName, O_RDONLY);
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

void writeToPipe(char *motorSymbol, char *moveSize)
{
    // char pipeName[6];
    // strcpy(&pipeName, 'PIPE_');
    // strcat(&pipeName, &motorSymbol);

    int fileDescriptor = openPipe(motorSymbol);
    write(fileDescriptor, moveSize, strlen(moveSize) + 1);
    close(fileDescriptor);
}

int main(int argc, char const *argv[])
{
    char *pipeX = "x";
    char messageParentString[80];
    int messageParent;
    char format_string[80] = "%d%d%d%d%d";

    printf("Creaing Pipe\n");
    createPipe(pipeX);
    int fd1;
    int n1, n2, n3;
    printf("Opening Pipe\n");

    fd1 = openPipe(pipeX);
    printf("Creaing Pipe\n");
    int readError = read(fd1, messageParentString, 80);
    close(fd1);

    // printf(stdin);
    // printf(stdout);


    printf("Read error is %d\n", readError);
    sscanf(messageParentString, format_string, &n1, &n2, &n3);
    printf(" strlen(messageParentString) is %d\n", strlen(messageParentString));


    printf("The messageParentString is %d and %d and %d\n", n1, n2, n3);




    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo

    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}
