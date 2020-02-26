//
//  Project1.c
//  
//
//  Created by Xuanzuo Liu on 2020-02-20.
//

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAX_NUM 1000 // max number of letters
#define MAX_COM 100 // max number of commands

// shell cleaning
#define clearShell() printf("\e[1;1H\e[2J")

// Greeting shell during startup
void initShell()
{
    clearShell();
    char* username = getenv("USER");//getenv("PATH")
    printf("\n\n\n SP500: @%s", username);
    sleep(1);
    clearShell();
}

// Function for input
int takeInput(char* input)
{
    char* buf;
    buf = readline("\n>>> ");
    //buf = freopen(,"",stdout);
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(input, buf);
        return 0;
    } else {
        return 1;
    }
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));//getcwd() is designed to get current directory
    printf("\n Dir: %s", cwd);
}

// Function for executing embedded command
void execArgs(char** parsed)
{
    // Forking child
    pid_t pid = fork();

    if (pid == -1) {
        printf("\n Failed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\n Command executation failed..");
        }
        exit(0);
    } else {
        // child terminate
        wait(NULL);
        return;
    }
}

// execute piped embedded commands
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int fd[2];
    pid_t p1, p2;
    if (pipe(fd) < 0) {
        printf("\n Initialized failed");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\n Fork failed");
        return;
    }
    if (p1 == 0) {
        // p1 executing..
        // write at the write end
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\n Could not execute command 1..");
            exit(0);
        }
    } else {
        // Parent process executing
        p2 = fork();
        
        if (p2 < 0) {
            printf("\n Could not fork");
            return;
        }

        // read at the read end
        if (p2 == 0) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\n Could not execute command 2..");
                exit(0);
            }
        } else {
            // parent process executing, waiting for two children process
            wait(NULL);
            wait(NULL);
        }
    }
}// source: assignment 1-->execTwo

// Help
void openHelp()
{
    puts("\n>cd"
         "\n>ls"
         "\n>exit"
         "\n>all other general commands available in UNIX shell"
         "\n>pipe handling"
         "\n>improper space handling");
    return;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int cmdslist = 4, switchArgu = 0;
    char* cmds[cmdslist];
    char* username;

    cmds[0] = "hello";
    cmds[1] = "cd";
    cmds[2] = "help";
    cmds[3] = "exit";

    for (int i = 0; i < cmdslist; i++) {
        if (strcmp(parsed[0], cmds[i]) == 0) {
            switchArgu = i + 1;
            break;
        }
    }

    switch (switchArgu) {
    case 1:
        username = getenv("USER");
        printf("\nHelp\n", username);
        return 1;
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        printf("\nExit\n");
        exit(0);
    default:
        break;
    }
    return 0;
}

// function for finding pipe
int parsePipe(char* input, char** foundpipe)
{
    for (int i = 0; i < 2; i++) {
        foundpipe[i] = strsep(&input, "|");
        if (foundpipe[i] == NULL)
            break;
    }

    if (foundpipe[1] == NULL)
        return 0; // returns zero when there is no pip.
    else {
        return 1;
    }
}

// function for parsing command words
void parseSpace(char* input, char** parsed)
{
    for (int i = 0; i < MAX_COM; i++) {
        parsed[i] = strsep(&input, " ");
        if (strlen(parsed[i]) == 0){
            i--;}
        if (parsed[i] == NULL){
            break;}
    }
}

int processString(char* input, char** parsed, char** parsedpipe)
{

    char* foundpipe[2];
    int piped = 0;
    piped = parsePipe(input, foundpipe);

    if (piped) {
        parseSpace(foundpipe[0], parsed);
        parseSpace(foundpipe[1], parsedpipe);
    } else {
        parseSpace(input, parsed);
    }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

int main()
{
    char inputString[MAX_NUM], *parsedArgs[MAX_COM];
    char* parsedArgsPiped[MAX_COM];
    int execFlag = 0;
    initShell();
    while (1) {
        // print current driction every time
        printDir();
        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // returns zero if there is no command
        // 1 if it is a simple command
        // 2 if it is including a pipe.
        if (execFlag == 1)
            execArgs(parsedArgs);
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}
