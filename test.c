#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <string.h>

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

int main(int argc, char const *argv[])
{
    char axis = 'a';
    char helperChar[2];

    char motorProcessPath[50] = "communication/motorProcess_";
    helperChar[1] = '\0';
    helperChar[0] = axis;

    strcat(motorProcessPath, helperChar);
    printf("This is the string -> %s", motorProcessPath);
    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo
    srand((unsigned)time(NULL));
    int randomness = rand() % 10;
    printf("%d\n", randomness);

    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}