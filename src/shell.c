
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

// Read command using GNU Readline
char* read_cmd(char* prompt) {

    char* cmdline = readline(prompt);

    // Handle Ctrl + D
    if (cmdline == NULL) {
        return NULL;
    }

    return cmdline;
}

// Free partially allocated token list
static void free_tokens(char** arglist, int count) {
    for (int i = 0; i < count; i++) {
        free(arglist[i]);
    }

    free(arglist);
}

// Tokenize command and recognize special operators



char** tokenize(char* cmdline) {

    if (cmdline == NULL || cmdline[0] == '\0') {
        return NULL;
    }

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));

    if (arglist == NULL) {
        perror("malloc");
        return NULL;
    }

    int argnum = 0;
    int i = 0;

    while (cmdline[i] != '\0') {

        // Skip spaces and tabs outside quotes
        while (cmdline[i] == ' ' || cmdline[i] == '\t') {
            i++;
        }

        if (cmdline[i] == '\0') {
            break;
        }

        if (argnum >= MAXARGS) {
            printf("Error: Too many arguments\n");
            free_tokens(arglist, argnum);
            return NULL;
        }

        // Recognize shell operators
        if (cmdline[i] == '>' ||
            cmdline[i] == '<' ||
            cmdline[i] == '|') {

            arglist[argnum] = malloc(2);

            if (arglist[argnum] == NULL) {
                perror("malloc");
                free_tokens(arglist, argnum);
                return NULL;
            }

            arglist[argnum][0] = cmdline[i];
            arglist[argnum][1] = '\0';

            argnum++;
            i++;
            continue;
        }

        char token[MAX_LEN];
        int j = 0;
        char quote = '\0';

        // Read one complete token
        while (cmdline[i] != '\0') {

            char current = cmdline[i];

            // Start or end quoted text
            if (current == '"' || current == '\'') {

                if (quote == '\0') {
                    quote = current;
                    i++;
                    continue;
                }
                else if (quote == current) {
                    quote = '\0';
                    i++;
                    continue;
                }
            }

            // Stop at whitespace or operators outside quotes
            if (quote == '\0' &&
                (current == ' ' || current == '\t' ||
                 current == '>' || current == '<' ||
                 current == '|')) {
                break;
            }

            if (j < MAX_LEN - 1) {
                token[j++] = current;
            }

            i++;
        }

        if (quote != '\0') {
            printf("Error: Unclosed quotation mark\n");
            free_tokens(arglist, argnum);
            return NULL;
        }

        token[j] = '\0';

        arglist[argnum] = strdup(token);

        if (arglist[argnum] == NULL) {
            perror("strdup");
            free_tokens(arglist, argnum);
            return NULL;
        }

        argnum++;
    }

    if (argnum == 0) {
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;

    return arglist;
}


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

