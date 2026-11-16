 
#include "shell.h"
// Available built-in commands
static const char *commands[] = {
    "cd",
    "exit",
    "help",
    "history",
    "jobs",
    NULL
};

// Generate matching built-in commands
static char *command_generator(const char *text, int state) {
    static int index;
    static size_t length;

    if (state == 0) {
        index = 0;
        length = strlen(text);
    }

    while (commands[index] != NULL) {
        const char *name = commands[index++];

        if (strncmp(name, text, length) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

// Decide whether to complete commands or filenames
static char **shell_completion(const char *text, int start, int end) {
    (void)end;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, rl_filename_completion_function);
}

// Enable custom Tab completion
void setup_completion(void) {
    rl_attempted_completion_function = shell_completion;
}


char* read_cmd(char* prompt) {

    // Read command using GNU Readline
    char* cmdline = readline(prompt);

    // Handle Ctrl + D
    if (cmdline == NULL) {
        return NULL;
    }

    // Return the command, even if it is empty
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



// Display command history
// Display command history
void display_history(void) {

    HIST_ENTRY** entries = history_list();

    if (entries == NULL) {
        return;
    }

    for (int i = 0; entries[i] != NULL; i++) {
        printf("%d  %s\n", history_base + i, entries[i]->line);
    }
}
// Retrieve a command using its history number
char* get_history(int number) {
    HIST_ENTRY* entry = history_get(number);

    if (entry == NULL) {
        return NULL;
    }

    return entry->line;
}
