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
    //fflush(stdin);
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

//returns a fileDescriptor for named pipe
int openPipeRead(char *pipeName)
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


//returns a fileDescriptor for named pipe
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


int writeToPipe(char *pipe, char *moveSize)
{
    int fileDescriptor = openPipeWrite(pipe);
    write(fileDescriptor, moveSize, strlen(moveSize) + 1);
    close(fileDescriptor);
    return fileDescriptor;

}

int readFromPipe(char *pipe, char *moveSize)
{
    int fileDescriptor = openPipeRead(pipe);
    read(fileDescriptor, moveSize, strlen(moveSize) + 1);
    close(fileDescriptor);
    return fileDescriptor;
}


int main(int argc, char const *argv[])
{
    char *pipeX = "x";
    char messageParentString[100];
    int messageParent;
    char format_string[100] = "%d";

    printf("Creaing Pipe\n");
    createPipe(pipeX);
    int fd1;
    int n1, n2, n3;
    printf("Opening Pipe\n");
    while(1)
    {
      // if (readFromPipe(pipeX, messageParentString) == - 1)
      if ( read(openPipeRead(pipeX), messageParentString, strlen(messageParentString) + 1) == - 1)
      {
        printf("Reading from pipe failed!\n");
        return 10;
      }
      else
      {
        printf("READING FROM PIPE SUCCESS\n");
      }
      sscanf(messageParentString, format_string, &n1);
      printf(" strlen(messageParentString) is %ld\n", strlen(messageParentString));
      printf("The messageParentString is %d and\n", n1);
    }
    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo
    /*Waiting for ALL CHILD processes to finish*/

    return 0;
}
