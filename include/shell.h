#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#define MAX_LEN 512
#define MAXARGS 64
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20
// Function prototypes
char* read_cmd(char* prompt);
char** tokenize(char* cmdline);

int execute(char** arglist, int background, const char* command);

int handle_builtin(char** arglist);
void display_history(void);
char* get_history(int number);

void setup_completion(void);

// Background job management
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
} BackgroundJob;

void add_job(pid_t pid, const char* command);
void remove_job(pid_t pid);
void display_jobs(void);
void reap_background_jobs(void);
#endif // SHELL_H
