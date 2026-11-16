
#include "shell.h"

// History storage: array of 20 command strings
static char* history[HISTORY_SIZE];
static int history_count = 0;


char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }

    cmdline[pos] = '\0';
    return cmdline;
}


char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));

    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;

        if (*cp == '\0') break;

        start = cp;
        len = 1;

        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) {
            free(arglist[i]);
        }

        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}


// Add a command to history
void add_history(const char* command) {

    // If history is full, remove the oldest command
    if (history_count == HISTORY_SIZE) {

        free(history[0]);

        // Shift remaining commands one position left
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }

        history_count--;
    }

    // Store a copy of the new command
    history[history_count] = strdup(command);

    if (history[history_count] != NULL) {
        history_count++;
    }
}


// Display all stored commands
void display_history(void) {

    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}


// Retrieve a command using its history number
char* get_history(int number) {

    if (number < 1 || number > history_count) {
        return NULL;
    }

    return history[number - 1];
}
