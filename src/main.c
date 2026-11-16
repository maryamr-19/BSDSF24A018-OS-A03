
#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;
    int result;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        if ((arglist = tokenize(cmdline)) != NULL) {

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

        free(cmdline);

        // Exit the shell if user entered exit
        if (result == -1) {
            break;
        }
    }

    printf("\nShell exited.\n");
    return 0;
}
