#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "io_helpers.h"
#include "variables.h"

#include <ctype.h>
#include "builtins.h"
#include "commands.h"
#include <sys/wait.h>

// ===== Output helpers =====

/* Prereq: str is a NULL terminated string
 */
void display_message(char *str)
{
    write(STDOUT_FILENO, str, strnlen(str, MAX_STR_LEN));
}

/* Prereq: pre_str, str are NULL terminated string
 */
void display_error(char *pre_str, char *str)
{
    write(STDERR_FILENO, pre_str, strnlen(pre_str, MAX_STR_LEN));
    write(STDERR_FILENO, str, strnlen(str, MAX_STR_LEN));
    write(STDERR_FILENO, "\n", 1);
}

// ===== Input tokenizing =====

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr)
{
    int retval = read(STDIN_FILENO, in_ptr, MAX_STR_LEN + 1); // Not a sanitizer issue since in_ptr is allocated as MAX_STR_LEN+1
    int read_len = retval;
    if (retval == -1)
    {
        read_len = 0;
    }
    if (read_len > MAX_STR_LEN)
    {
        read_len = 0;
        retval = -1;
        write(STDERR_FILENO, "ERROR: input line too long\n", strlen("ERROR: input line too long\n"));
        int junk = 0;
        while ((junk = getchar()) != EOF && junk != '\n')
            ;
    }
    in_ptr[read_len] = '\0';
    return retval;
}

size_t tokenize_input(char *in_ptr, char **tokens)
{
    char *saveptr; // For strtok_r

    char *curr_ptr = strtok_r(in_ptr, DELIMITERS, &saveptr);
    size_t token_count = 0;
    while (curr_ptr != NULL)
    {
        // char *result = malloc(sizeof(char) * 128 + 1);
        // result[0] = '\0';

        if (strchr(curr_ptr, '$'))
        {
            expand(curr_ptr);
        }

        tokens[token_count] = curr_ptr;

        // tokens[token_count] = expand(curr_ptr, result);
        token_count++;

        curr_ptr = strtok_r(NULL, DELIMITERS, &saveptr);
    }

    tokens[token_count] = NULL;
    return token_count;
}

// char *expand(char *token, char *result)
// {
//     char buffer[128]; // Local buffer to build the result
//     buffer[0] = '\0';

//     if (!strchr(token, '$'))
//     {
//         strcpy(buffer, token);
//     }
//     else
//     {
//         char *temp;
//         char *saveptr; // For strtok_r
//         if (token[0] == '$')
//         {
//             temp = strtok_r(token, "$", &saveptr);
//             if (temp != NULL)
//             {
//                 strcat(buffer, find(myshfrontvar, temp));
//             }
//         }
//         else
//         {
//             temp = strtok_r(token, "$", &saveptr);
//             if (temp != NULL)
//             {
//                 strcat(buffer, temp);
//             }
//         }

//         temp = strtok_r(NULL, "$", &saveptr);
//         while (temp != NULL)
//         {
//             strcat(buffer, find(myshfrontvar, temp));
//             temp = strtok_r(NULL, "$", &saveptr);
//         }
//     }

//     // Copy the final result back to the provided buffer
//     strcpy(result, buffer);
//     return result;
// }

// /* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
//  * Warning: in_ptr is modified
//  * Return: number of tokens.
//  */
// size_t tokenize_input(char *in_ptr, char **tokens)
// {
//     char *saveptr; // For strtok_r

//     char *curr_ptr = strtok_r(in_ptr, DELIMITERS, &saveptr);
//     size_t token_count = 0;
//     while (curr_ptr != NULL)
//     {
//         char *result = malloc(sizeof(char) * 128 + 1);
//         result[0] = '\0';

//         // if (strchr(curr_ptr, '$')){
//         //     expand(curr_ptr);
//         // }

//         tokens[token_count] = curr_ptr;

//         tokens[token_count] = expand(curr_ptr, result);
//         token_count++;

//         curr_ptr = strtok_r(NULL, DELIMITERS, &saveptr);

//         // display_message(in_ptr);

//         // display_message("why: ");
//         // display_message(curr_ptr);
//     }

//     tokens[token_count] = NULL;
//     return token_count;
// }

// char *expand(char *token, char *result)
// {

//     if (!strchr(token, '$'))
//     {
//         strcpy(result, token);
//         return result;
//     }
//     char *temp;
//     char *saveptr; // For strtok_r
//     if (token[0] == '$')
//     {

//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, find(myshfrontvar, temp));
//             // display_message("\ntesting: ");
//             // display_message(find(myshfrontvar, temp));
//             // display_message("\n");
//         }
//     }
//     else
//     {
//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, temp);
//         }
//     }
//     // char *last = NULL;
//     temp = strtok_r(NULL, "$", &saveptr);
//     while (temp != NULL)
//     {
//         strcat(result, find(myshfrontvar, temp));
//         // last = strdup(temp);
//         temp = strtok_r(NULL, "$", &saveptr);
//     }
//     // if (last != NULL)
//     // {
//     //     strcat(result, last);
//     //     free(last);
//     // }

//     return result;
// }

// Fake code

void expand(char *token)
{
    char *curr_ptr = token;
    char *start_ptr = NULL;
    char expanded_token[128];
    expanded_token[0] = '\0';

    start_ptr = strchr(curr_ptr, '$');

    while (start_ptr != NULL)
    {
        strncat(expanded_token, curr_ptr, start_ptr - curr_ptr);

        curr_ptr = start_ptr + 1;

        char *end_ptr = curr_ptr;
        while (*end_ptr && !isspace(*end_ptr) && strchr(DELIMITERS, *end_ptr) == NULL)
        {
            end_ptr++;
        }

        size_t var_length = end_ptr - curr_ptr;
        char var_name[var_length + 1];
        strncpy(var_name, curr_ptr, var_length);
        var_name[var_length] = '\0';

        char *var_value = find(myshfrontvar, var_name);

        if (var_value != NULL)
        {
            strcat(expanded_token, var_value);
        }

        curr_ptr = end_ptr;

        start_ptr = strchr(curr_ptr, '$');
    }

    strcat(expanded_token, curr_ptr);

    strcpy(token, expanded_token);
}

// char *expand(char *token, char *result)
// {

//     if (!strchr(token, '$'))
//     {
//         strcpy(result, token);
//         return result;
//     }
//     char *temp;
//     char *saveptr; // For strtok_r
//     if (token[0] == '$')
//     {

//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, find(myshfrontvar, temp));
//             // display_message("\ntesting: ");
//             // display_message(find(myshfrontvar, temp));
//             // display_message("\n");
//         }
//     }
//     else
//     {
//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, temp);
//         }
//     }
//     // char *last = NULL;
//     temp = strtok_r(NULL, "$", &saveptr);
//     while (temp != NULL)
//     {
//         strcat(result, find(myshfrontvar, temp));
//         // last = strdup(temp);
//         temp = strtok_r(NULL, "$", &saveptr);
//     }
//     // if (last != NULL)
//     // {
//     //     strcat(result, last);
//     //     free(last);
//     // }

//     return result;
// }

// ssize_t *expand(char *token, char *result, ssize_t remaining_space)
// {

//     if (!strchr(token, '$'))
//     {
//         strncpy(result, token, remaining_space);
//         return remaining_space - strlen(result);
//     }

//     // fhjdh$fjhdjhf
//     // $hfdjf$jhfdj
//     ssize_t start = 0;
//     ssize_t last_index = 0;
//     ssize_t letter_count = 0;
//     for (int i = 0; token[i] != '\0'; i++)
//     {
//         if (remaining_space == 0)
//         {
//             break;
//         }
//         if (i == 0 && token[i] == '$')
//         {
//             last_index++;
//             start++;
//             continue;
//         }
//         if (token[i] == '$')
//         {
//             if (start == 0)
//             {
//                 char substring[letter_count + 1];
//                 substring[letter_count] = '\0';
//                 strncpy(substring, token + last_index, letter_count);

//                 strncat(result, substring, remaining_space);

//                 if (remaining_space < strlen(result))
//                 {
//                     remaining_space = 0;
//                 }
//                 else
//                 {
//                     remaining_space -= strlen(result);
//                 }

//                 letter_count = 0;
//                 last_index = i + 1;
//                 continue;
//             }
//             if (token[i + 1] != '\0' && token[i + 1] == '$')
//             {
//                 strncat(result, "$", remaining_space);
//                 remaining_space--;
//                 last_index++;
//                 continue;
//             }

//             printf("Found $ at index: %d\n", i);

//             char substring[letter_count + 1];
//             substring[letter_count] = '\0';
//             strncpy(substring, token + last_index, letter_count);

//             // char * val = find(myshfrontvar, substring);

//             strncat(result, find(myshfrontvar, substring), remaining_space);

//             if (remaining_space < strlen(result))
//             {
//                 remaining_space = 0;
//             }
//             else
//             {
//                 remaining_space -= strlen(result);
//             }

//             letter_count = 0;
//             last_index = i + 1;
//             continue;
//         }

//         letter_count++;
//     }
//     return remaining_space;
// }

// ssize_t dollar_indicator = 0;
// ssize_t index = 0;
// ssize_t dupe_indicator = 0;
// while (!(curr_len > 128 || (index + 1) == strlen(token))){

//     if (dollar_indicator == 0 && token[index] != '$'){
//         strncpy(result, token[index], 128 - curr_len);
//         curr_len = strlen(result);
//         index++;
//         continue;
//     }

//     if (token[index] == '$'){

//     }
// }

// int flag = 0;
// char *saveptr;
// char *temp = strtok_r(token, "$", &saveptr);;

// if (temp == NULL){
//     strncpy(result, token, 128 - curr_len);
//     return curr_len + strlen(result);
// }

// while (!(curr_len > 128 || temp != NULL))
// {
//     if (!flag){
//         if (token[0] == '$'){
//             char * val = find(myshfrontvar, temp);
//             strncpy(result, val, 128 - curr_len);
//             curr_len += strlen(result);
//         } else {
//             strncpy(result, temp, 128 - curr_len);
//             curr_len += strlen(result);
//         }
//         temp = strtok_r(NULL, "$", &saveptr);;
//         continue;
//         flag++;
//     }

//     strncpy(result, temp, 128 - curr_len);
//     curr_len += strlen(result);
//     temp = strtok_r(NULL, "$", &saveptr);;
// }
// }

// ssize_t expand(char *token, char *result)
// {

//     if (!strchr(token, '$'))
//     {
//         strcpy(result, token);
//         return strlen(result);
//     }
//     char *temp;
//     char *saveptr; // For strtok_r
//     if (token[0] == '$')
//     {

//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, find(myshfrontvar, temp));
//             // display_message("\ntesting: ");
//             // display_message(find(myshfrontvar, temp));
//             // display_message("\n");
//         }
//     }
//     else
//     {
//         temp = strtok_r(token, "$", &saveptr);
//         if (temp != NULL)
//         {
//             strcat(result, temp);
//         }
//     }
//     // char *last = NULL;
//     temp = strtok_r(NULL, "$", &saveptr);
//     while (temp != NULL)
//     {
//         strcat(result, find(myshfrontvar, temp));
//         // last = strdup(temp);
//         temp = strtok_r(NULL, "$", &saveptr);
//     }
//     // if (last != NULL)
//     // {
//     //     strcat(result, last);
//     //     free(last);
//     // }

//     return strlen(result);
// }

/*
Return 1 if pipe
Return 0 if no pipe
*/
int check_for_pipe(char **tokens)
{
    for (int i = 0; tokens[i] != NULL; i++)
    {
        if (strcmp(tokens[i], "|") == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*
Return number of tokens ran if run fine
Return -1 if not run fine
*/
int run_cmd_blt(char **tokens, int token_count)
{

    char **token_arr = malloc((token_count + 1) * (sizeof(char *)));

    int i = 0;

    // while (strcmp(tokens[i], "|") != 0 && tokens[i] != NULL){
    while (tokens[i] != NULL && strcmp(tokens[i], "|") != 0)
    {
        token_arr[i] = tokens[i];

        i++;
    }

    token_arr[i] = NULL;

    cmd_ptr cmd_fn;
    if (strcmp(token_arr[i - 1], "&") == 0)
    {
        cmd_fn = cmd_bgproc;
        ssize_t err = cmd_fn(token_arr);
        if (err == -1)
        {
            display_error("ERROR: Builtin failed: ", token_arr[0]);
            free(token_arr);
            return -1;
        }
        free(token_arr);
        return i;
    }

    bn_ptr builtin_fn = check_builtin(token_arr[0]);
    if (builtin_fn != NULL)
    {
        ssize_t err = builtin_fn(token_arr);
        if (err == -1)
        {
            display_error("ERROR: Builtin failed: ", token_arr[0]);
            free(token_arr);
            return -1;
        }
        free(token_arr);
        return i;
    }

    cmd_fn = check_cmd(token_arr[0]);

    if (cmd_fn != NULL)
    {
        if (cmd_fn == cmd_cd)
        {
            return i;
        }
        ssize_t err = cmd_fn(token_arr);
        if (err == -1)
        {
            display_error("ERROR: Builtin failed: ", token_arr[0]);
            free(token_arr);
            return -1;
        }
        free(token_arr);
        return i;
    }

    char bin[MAX_STR_LEN];
    sprintf(bin, "/bin/%s", token_arr[0]);

    char **bin_cmds = malloc((token_count + 1) * sizeof(char *));

    bin_cmds[0] = bin;
    for (int i = 1; i < token_count; i++)
    {
        bin_cmds[i] = token_arr[i];
    }
    bin_cmds[token_count] = NULL;

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(bin_cmds[0], bin_cmds);

        sprintf(bin, "/user/bin/%s", token_arr[0]);
        bin_cmds[0] = bin;
        execvp(bin_cmds[0], bin_cmds);

        display_error("ERROR: Unknown command: ", token_arr[0]);
        free(bin_cmds);
        free(token_arr);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        waitpid(pid, NULL, 0);
        free(bin_cmds);
        free(token_arr);
        return i;
    }
    else
    {
        display_error("ERROR: Unknown command: ", token_arr[0]);
        free(bin_cmds);
        free(token_arr);
        return -1;
    }
}