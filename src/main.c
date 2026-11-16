

#include "shell.h"
#include <limits.h>

int main() {

    char* cmdline;
    char** arglist;
    int result;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // STEP 1: Check whether the command is a history replay
        if (cmdline[0] == '!') {

            char* endptr;
            errno = 0;

            // Extract the number after !
            long number = strtol(cmdline + 1, &endptr, 10);

            // Check for invalid history number
            if (errno == ERANGE ||
                endptr == cmdline + 1 ||
                *endptr != '\0' ||
                number < 1 ||
                number > INT_MAX) {

                printf("Invalid history number\n");
                free(cmdline);
                continue;
            }

            // STEP 2: Retrieve the previous command
            char* previous = get_history((int)number);

            if (previous == NULL) {
                printf("Invalid history number\n");
                free(cmdline);
                continue;
            }

            // STEP 3: Make a separate copy of the command
            char* expanded = strdup(previous);

            if (expanded == NULL) {
                perror("strdup");
                free(cmdline);
                continue;
            }

            // Replace !n with the actual command
            free(cmdline);
            cmdline = expanded;

            // Display the command being executed
            printf("%s\n", cmdline);
        }

        // STEP 4: Tokenize the command
        if ((arglist = tokenize(cmdline)) != NULL) {

            // STEP 5: Store the actual command in history
            add_history(cmdline);

            // STEP 6: Execute the command
            result = execute(arglist);

            // Free memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }

            free(arglist);
        }
        else {
            result = 0;
        }

        // Free command line memory
        free(cmdline);

        // Exit the shell if user entered exit
        if (result == -1) {
            break;
        }
    }

    printf("\nShell exited.\n");

    return 0;
}

