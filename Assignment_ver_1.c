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

int main(int argc, char const *argv[])
{
    char userInput[100];
    while (1)
    {
        yellow();
        printf("Please type in x,z with number e.g. - 'x10', 'x-10', 'z20', `z-10` \n");
        reset();
        scanf("%s", userInput);
        fflush(stdin);
        printf("%s\n", userInput);
        fflush(stdout);

        if (isInputValid(userInput) == 1)
        {
            printf("Invalid input.");
            exit(0);
        }

        char motorSymbol = userInput[0];
        int lengthOfMove = sizeof(userInput) / (sizeof(char)) - 1;
        char moveSize[lengthOfMove + 1];

        if (userInput[1] == '-')
        {
            memcpy(moveSize, &userInput[2], lengthOfMove - 1);
        }
        else
        {
            memcpy(moveSize, &userInput[1], lengthOfMove);
        }

        printf("motorSymbol -> %c\n", motorSymbol);
        printf("moveSize -> %s\n", moveSize);
    }

    /*fd[0] is for READ, fd[1] is for WRITE*/
    /*Waiting for ALL CHILD processes to finish*/
    while (wait(NULL) != -1 || errno != ECHILD)
        ;
    return 0;
}