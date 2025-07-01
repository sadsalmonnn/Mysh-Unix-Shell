#include <string.h>

#include "builtins.h"
#include "io_helpers.h"

#include "variables.h"

#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>
// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd)
{
    ssize_t cmd_num = 0;

    if (strchr(cmd, '=') != NULL)
    {
        return BUILTINS_FN[1];
    }

    while (cmd_num < BUILTINS_COUNT &&
           strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0)
    {
        cmd_num += 1;
    }
    return BUILTINS_FN[cmd_num];
}

// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo.
 */
ssize_t bn_echo(char **tokens)
{
    ssize_t index = 1;

    if (tokens[index] != NULL)
    {
        // TODO:
        // Implement the echo command
        display_message(tokens[index]);
        index++;
    }
    while (tokens[index] != NULL)
    {
        display_message(" ");
        display_message(tokens[index]);
        // TODO:
        // Implement the echo command
        index += 1;
    }
    display_message("\n");

    return 0;
}

ssize_t bn_var(char **tokens)
{

    // check if theres anything after the assigning and if the assigning has a variable name
    if (tokens[0] == NULL || tokens[0][0] == '=')
    {
        return -1;
    }

    char *variable = NULL;
    char *value = NULL;

    // Use sscanf to split the string at the first '='
    int num_tokens = sscanf(tokens[0], "%m[^=]=%m[^\n]", &variable, &value);

    if (num_tokens == 0) // No = found
    {
        return -1;
    }
    else if (num_tokens == 1) // = found, but no value
    {
        value = "";
    }

    insert(&myshfrontvar, variable, value);

    return 0;
}