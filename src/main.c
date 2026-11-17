
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/history.h>

/* Remove leading and trailing spaces */
char* trim(char* str)
{
    if (str == NULL)
        return NULL;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    char* end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
        end--;

    *(end + 1) = '\0';

    return str;
}

/* Check whether a command runs in the background */
int is_background(char* command)
{
    if (command == NULL)
        return 0;

    int len = strlen(command);

    while (len > 0 && isspace((unsigned char)command[len - 1]))
        len--;

    return (len > 0 && command[len - 1] == '&');
}

/* Append a line to a block */
void append_line(char** block, const char* line)
{
    if (line == NULL)
        return;

    size_t old_len = (*block == NULL) ? 0 : strlen(*block);
    size_t line_len = strlen(line);

    char* temp = realloc(*block, old_len + line_len + 2);

    if (temp == NULL)
    {
        perror("realloc");
        return;
    }

    *block = temp;

    if (old_len > 0)
        (*block)[old_len++] = '\n';

    memcpy(*block + old_len, line, line_len);
    (*block)[old_len + line_len] = '\0';
}

/* Execute ordinary commands */
int process_commands(char* text)
{
    if (text == NULL)
        return 0;

    char* copy = strdup(text);

    if (copy == NULL)
    {
        perror("strdup");
        return 1;
    }

    char* saveptr = NULL;
    char* command = strtok_r(copy, ";\n", &saveptr);

    int status = 0;

    while (command != NULL)
    {
        command = trim(command);

        if (*command != '\0')
        {
            int background = is_background(command);

            char** tokens = tokenize(command);

            if (tokens != NULL)
            {
                status = execute(tokens, background, command);

                if (status == -1)
                {
                    free(copy);
                    return -1;
                }
            }
        }

        command = strtok_r(NULL, ";\n", &saveptr);
    }

    free(copy);
    return status;
}

/* Check whether a line exactly matches a keyword */
int is_keyword(const char* line, const char* keyword)
{
    if (line == NULL)
        return 0;

    char* copy = strdup(line);

    if (copy == NULL)
        return 0;

    int result = (strcmp(trim(copy), keyword) == 0);

    free(copy);
    return result;
}

/*
 * Read the next if-statement line.
 * First use remaining lines from pasted input.
 * If no pasted lines remain, read from the shell.
 */
char* get_if_line(char** saveptr)
{
    char* line = strtok_r(NULL, "\n", saveptr);

    if (line != NULL)
    {
        char* result = strdup(line);

        if (result == NULL)
            perror("strdup");

        return result;
    }

    char* input = read_cmd(PROMPT);

    if (input == NULL)
        return NULL;

    char* result = strdup(input);
    free(input);

    if (result == NULL)
        perror("strdup");

    return result;
}

/* Handle if-then-else-fi statements */
int handle_if_statement(char* first_line)
{
    if (first_line == NULL)
        return 1;

    /*
     * Keep a copy of the complete input.
     * This allows us to process multiple pasted lines.
     */
    char* input_copy = strdup(first_line);

    if (input_copy == NULL)
    {
        perror("strdup");
        return 1;
    }

    char* saveptr = NULL;

    /* Extract the first line containing the condition */
    char* first = strtok_r(input_copy, "\n", &saveptr);

    if (first == NULL)
    {
        free(input_copy);
        return 1;
    }

    first = trim(first);

    if (strncmp(first, "if ", 3) != 0)
    {
        printf("Error: Invalid if statement\n");
        free(input_copy);
        return 1;
    }

    /* Extract condition after if */
    char* condition = strdup(trim(first + 2));

    if (condition == NULL)
    {
        perror("strdup");
        free(input_copy);
        return 1;
    }

    char* then_block = NULL;
    char* else_block = NULL;

    int found_then = 0;
    int found_fi = 0;
    int in_else = 0;
    int parse_error = 0;

    /*
     * Read all remaining lines.
     * Stop when fi is found.
     */
    while (1)
    {
        char* line = get_if_line(&saveptr);

        if (line == NULL)
        {
            printf("Error: Expected 'fi'\n");
            parse_error = 1;
            break;
        }

        char* cleaned = trim(line);

        if (!found_then)
        {
            if (is_keyword(cleaned, "then"))
            {
                found_then = 1;
            }
            else
            {
                printf("Error: Expected 'then', received '%s'\n",
                       cleaned);

                parse_error = 1;
                free(line);
                break;
            }
        }
        else if (is_keyword(cleaned, "else"))
        {
            in_else = 1;
        }
        else if (is_keyword(cleaned, "fi"))
        {
            found_fi = 1;
            free(line);
            break;
        }
        else
        {
            if (in_else)
                append_line(&else_block, cleaned);
            else
                append_line(&then_block, cleaned);
        }

        free(line);
    }

    int status = 1;

    /*
     * Execute only if the complete if statement
     * has both then and fi.
     */
    if (!parse_error && found_then && found_fi)
    {
        /* Execute the condition */
        status = process_commands(condition);

        if (status == 0)
        {
            /* Condition successful: execute then block */
            if (then_block != NULL)
                status = process_commands(then_block);
        }
        else
        {
            /* Condition unsuccessful: execute else block */
            if (else_block != NULL)
                status = process_commands(else_block);
        }
    }

    free(condition);
    free(then_block);
    free(else_block);
    free(input_copy);

    return status;
}

/* Main shell loop */
int main(void)
{
    while (1)
    {
        char* cmdline = read_cmd(PROMPT);

        if (cmdline == NULL)
        {
            printf("\n");
            break;
        }

        char* command = trim(cmdline);

        if (*command == '\0')
        {
            free(cmdline);
            continue;
        }

        /* Handle history commands such as !1 */
        if (command[0] == '!' &&
            isdigit((unsigned char)command[1]))
        {
            int history_num = atoi(command + 1);
            HIST_ENTRY* entry = history_get(history_num);

            if (entry != NULL)
            {
                char* history_command = strdup(entry->line);

                if (history_command != NULL)
                {
                    printf("%s\n", history_command);

                    if (strncmp(trim(history_command), "if ", 3) == 0)
                        handle_if_statement(history_command);
                    else
                        process_commands(history_command);

                    free(history_command);
                }
            }
            else
            {
                printf("Error: No such command in history\n");
            }

            free(cmdline);
            continue;
        }

        add_history(command);

        int status;

        /* Detect if statement */
        if (strncmp(command, "if ", 3) == 0)
        {
            status = handle_if_statement(command);
        }
        else
        {
            status = process_commands(command);
        }

        if (status == -1)
        {
            free(cmdline);
            break;
        }

        free(cmdline);
    }

    return 0;
}
