
#include "shell.h"

// Handle built-in commands
// Returns 0 = not built-in
// Returns 1 = built-in handled
// Returns -1 = exit command
int handle_builtin(char** arglist) {

    // EXIT command
    if (strcmp(arglist[0], "exit") == 0) {
        return -1;
    }

    // CD command
    if (strcmp(arglist[0], "cd") == 0) {

        if (arglist[1] == NULL) {
            printf("Usage: cd <directory>\n");
        }
        else if (chdir(arglist[1]) == -1) {
            perror("cd");
        }

        return 1;
    }

    // HELP command
    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in Commands:\n");
        printf("exit  - Exit the shell\n");
        printf("cd    - Change directory\n");
        printf("help  - Display available commands\n");
        printf("jobs  - Display job control status\n");

        return 1;
    }

    // JOBS command
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    // Not a built-in command
    return 0;
}


// Execute commands
int execute(char* arglist[]) {

    // Check built-in commands BEFORE fork()
    int result = handle_builtin(arglist);

    if (result == -1) {
        return -1;
    }

    if (result == 1) {
        return 0;
    }

    // External commands are executed using fork()
    int status;
    int cpid = fork();

    switch (cpid) {

        case -1:
            perror("fork failed");
            exit(1);

        case 0: // Child process
            execvp(arglist[0], arglist);
            perror("Command not found");
            exit(1);

        default: // Parent process
            waitpid(cpid, &status, 0);
            return 0;
    }
}

