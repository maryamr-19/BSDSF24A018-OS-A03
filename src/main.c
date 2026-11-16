
#include "shell.h"
#include <limits.h>
#include <ctype.h>

// Remove spaces from the beginning and end of a command
char* trim(char* str) {

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    char* end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    return str;
}


// Check whether a command ends with &
int is_background(char* command) {

    int length = strlen(command);

    if (length > 0 && command[length - 1] == '&') {
        command[length - 1] = '\0';
        return 1;
    }

    return 0;
}


int main() {

    char* cmdline;
    char** arglist;
    int result = 0;

    // Initialize GNU Readline history
    using_history();
    stifle_history(HISTORY_SIZE);

    // Enable custom shell Tab completion
    setup_completion();


    while (1) {

        // Reap completed background processes before prompting
        reap_background_jobs();

        cmdline = read_cmd(PROMPT);

        if (cmdline == NULL) {
            break;
        }


        // Handle history replay
        if (cmdline[0] == '!') {

            char* endptr;
            errno = 0;

            long number = strtol(cmdline + 1, &endptr, 10);

            if (errno == ERANGE ||
                endptr == cmdline + 1 ||
                *endptr != '\0' ||
                number < 1 ||
                number > INT_MAX) {

                printf("Invalid history number\n");
                free(cmdline);
                continue;
            }

            char* previous = get_history((int)number);

            if (previous == NULL) {
                printf("Invalid history number\n");
                free(cmdline);
                continue;
            }

            char* expanded = strdup(previous);

            if (expanded == NULL) {
                perror("strdup");
                free(cmdline);
                continue;
            }

            free(cmdline);
            cmdline = expanded;

            printf("%s\n", cmdline);
        }


        // Store the complete input line in history
        add_history(cmdline);


        // Split commands using ;
        char* saveptr;
        char* command = strtok_r(cmdline, ";", &saveptr);


        // Execute commands one by one
        while (command != NULL) {

            command = trim(command);

            if (strlen(command) == 0) {
                command = strtok_r(NULL, ";", &saveptr);
                continue;
            }


            // Detect background execution
            int background = is_background(command);

            command = trim(command);


            if (strlen(command) == 0) {
                command = strtok_r(NULL, ";", &saveptr);
                continue;
            }


            // Tokenize command
            arglist = tokenize(command);


            if (arglist != NULL) {

                // Execute command
                result = execute(arglist, background, command);


                // Free token memory
                for (int i = 0; arglist[i] != NULL; i++) {
                    free(arglist[i]);
                }

                free(arglist);
            }


            // Stop if exit command was entered
            if (result == -1) {
                break;
            }


            // Move to next command after ;
            command = strtok_r(NULL, ";", &saveptr);
        }


        free(cmdline);


        if (result == -1) {
            break;
        }
    }


    // Final check for completed background processes
    reap_background_jobs();

    printf("\nShell exited.\n");

    return 0;
}
