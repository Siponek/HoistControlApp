#include "functionsFile.h"

int main(int argc, char const *argv[])
{
    char characa[] = "123";
    int number = atoi(characa);

    printf("PID of motor x %d", number);

    /*fd[0] is for READ, fd[1] is for WRITE*/
    // mkfifo("myfifo1", 0777); // A classic mkfifo

    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}