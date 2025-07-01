#include <string.h>

#include "commands.h"
#include "io_helpers.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

Bgprocess *bg_process_list = NULL;

int piping = 0;

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
cmd_ptr check_cmd(const char *cmd)
{
    ssize_t cmd_num = 0;

    while (cmd_num < COMMANDS_COUNT &&
           strncmp(COMMANDS[cmd_num], cmd, MAX_STR_LEN) != 0)
    {
        cmd_num += 1;
    }
    return COMMANDS_FN[cmd_num];
}

ssize_t cmd_ls(char **tokens)
{
    char *currpath = ".";
    if (tokens[1] == NULL)
    {
        DIR *dir = opendir(currpath);

        if (dir == NULL)
        {
            display_error("ERROR: Invalid path", "");
            return -1;
        }

        struct dirent *de;

        while ((de = readdir(dir)) != NULL)
        {
            display_message(de->d_name);
            display_message("\n");
        }

        closedir(dir);
        return 0;
    }

    if (strcmp(tokens[1], "--f") == 0 || strcmp(tokens[1], "--d") == 0 || strcmp(tokens[1], "--rec") == 0)
    {
        currpath = ".";
    }
    else
    {
        currpath = tokens[1];
    }

    int depth = 0;
    int depth_flag = 0;
    int rec = 0;
    int rec_flag = 0;
    char *substring = NULL;
    int currpath_flag = 0;
    for (int i = 1; tokens[i] != NULL; i++)
    {
        if (strcmp(tokens[i], "--d") == 0 && tokens[i + 1] != NULL && !depth_flag)
        {
            depth = atoi(tokens[i + 1]);
            depth_flag = 1;
            i++;
            continue;
        }

        if (strcmp(tokens[i], "--rec") == 0 && !rec_flag)
        {
            rec = 1;
            rec_flag = 1;
            continue;
        }

        if (strcmp(tokens[i], "--f") == 0 && tokens[i + 1] != NULL && substring == NULL)
        {
            substring = tokens[i + 1];
            i++;
            continue;
        }

        if (!currpath_flag)
        {
            currpath = tokens[i];
            currpath_flag = 1;
            continue;
        }
        else if (currpath_flag)
        {
            display_error("ERROR: Too many arguments: cd takes a single path", "");
            return -1;
        }
    }

    if ((depth_flag && !rec_flag) || (depth < 0))
    {
        return -1;
    }

    DIR *check_directory = opendir(currpath);
    if (check_directory == NULL)
    {
        display_error("ERROR: Invalid path", "");
        return -1;
    }
    closedir(check_directory);

    if (rec && depth == 1)
    {
        DIR *dir = opendir(currpath);

        struct dirent *de;

        while ((de = readdir(dir)) != NULL)
        {
            display_message(de->d_name);
            display_message("\n");
        }

        closedir(dir);
        return 0;
    }
    else if (rec && depth_flag)
    {
        ls_helper(currpath, depth, substring);
    }
    else if (rec && !depth_flag)
    {
        ls_helper(currpath, -1, substring);
    }
    else
    {
        struct dirent *dirinfo;
        DIR *dir = opendir(currpath);

        if (dir == NULL)
        {
            display_error("ERROR: Invalid path", "");
            closedir(dir);
            return -1;
        }

        while ((dirinfo = readdir(dir)) != NULL)
        {

            size_t needed_size = strlen(dirinfo->d_name) + 1;

            char *dir_name = malloc(needed_size);
            snprintf(dir_name, needed_size, "%s\t", dirinfo->d_name);

            // char dir_name[MAX_STR_LEN];
            // snprintf(dir_name, sizeof(dir_name), "%s\t", dirinfo->d_name);

            if (substring != NULL && strstr(dir_name, substring) == NULL)
            {
                free(dir_name);
                continue;
            }

            // Display the message
            display_message(dir_name);
            display_message("\n");
            free(dir_name);
        }
        closedir(dir);
    }
    return 0;
}

ssize_t ls_helper(char *path, int depth, char *substring)
{
    DIR *directory = opendir(path);
    if (directory == NULL)
    {
        return 0;
    }

    struct dirent *dirinfo;

    while ((dirinfo = readdir(directory)) != NULL)
    {
        size_t needed_size = strlen(dirinfo->d_name) + 1;
        char *dir_name = malloc(needed_size);
        snprintf(dir_name, needed_size, "%s\t", dirinfo->d_name);

        // Check if the substring is part of the directory or file name
        if (substring == NULL || strstr(dir_name, substring) != NULL)
        {
            display_message(dir_name);
            display_message("\n");
        }

        if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0)
        {
            free(dir_name);
            continue;
        }
        free(dir_name);

        // Construct full path
        char *new_path = malloc(strlen(path) + strlen(dirinfo->d_name) + 2);
        snprintf(new_path, strlen(path) + strlen(dirinfo->d_name) + 2, "%s/%s", path, dirinfo->d_name);

        // Recurse if depth is greater than 1 or depth is -1 (infinite)
        if (depth > 1 || depth == -1)
        {
            ls_helper(new_path, (depth == -1) ? -1 : depth - 1, substring);
        }
        free(new_path);
    }

    closedir(directory);
    return 0;
}

ssize_t cmd_cd(char **tokens)
{
    char *path;

    if (tokens[1] == NULL || tokens[1][0] == '\0')
    {
        path = "/";
    }
    else
    {
        path = tokens[1];
    }

    if (chdir(path) != 0)
    {
        display_error("ERROR: Invalid path ", path);
        return -1;
    }

    return 0;
}

ssize_t cmd_cat(char **tokens)
{
    FILE *file_ptr = stdin; // Use stdin for standard input

    // If a file is provided, open it
    if (tokens[1] != NULL)
    {
        file_ptr = fopen(tokens[1], "r");
        if (file_ptr == NULL)
        {
            display_error("ERROR: Cannot open file", ""); // Provide the filename in the error message
            return -1;
        }
    }

    // Read and display the file or standard input
    int charc;
    while ((charc = fgetc(file_ptr)) != EOF)
    {
        char string[2];
        string[0] = (char)charc;
        string[1] = '\0';
        display_message(string);
    }

    // Close the file only if it was opened (not stdin)
    if (file_ptr != stdin)
    {
        fclose(file_ptr);
    }
    else if (file_ptr == stdin)
    {
        clearerr(stdin);
    }

    return 0; // Success
}

ssize_t cmd_wc(char **tokens)
{
    FILE *file_ptr = stdin; // Use stdin for standard input

    // If a file is provided, open it
    if (tokens[1] != NULL)
    {
        file_ptr = fopen(tokens[1], "r");
        if (file_ptr == NULL)
        {
            display_error("ERROR: Cannot open file", "");
            return -1;
        }
    }

    // Check for too many arguments
    // if (tokens[2] != NULL) {
    //     display_error("ERROR: Too many arguments: wc takes a single file", "");
    //     if (file_ptr != stdin) {
    //         fclose(file_ptr);
    //     }
    //     return -1;
    // }

    ssize_t char_count = 0;       // Characters in the current word
    ssize_t total_char_count = 0; // Total characters
    ssize_t word_count = 0;       // Total words
    ssize_t line_count = 0;       // Total lines

    int ch;
    while ((ch = fgetc(file_ptr)) != EOF)
    {

        // Count characters
        total_char_count++;

        // Count lines
        if (ch == '\n')
        {
            line_count++;
        }

        // Count words
        if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
        {
            if (char_count > 0)
            {
                word_count++;
            }
            char_count = 0;
        }
        else
        {
            char_count++;
        }
    }

    // Handle the last word if the file doesn't end with a delimiter
    if (char_count > 0)
    {
        word_count++;
    }

    // Close the file if it was opened
    if (file_ptr != stdin)
    {
        fclose(file_ptr);
    }
    else if (file_ptr == stdin)
    {
        clearerr(stdin);
    }

    // Display the counts (like wc)
    char word_count_str[MAX_STR_LEN];
    char line_count_str[MAX_STR_LEN];
    char char_count_str[MAX_STR_LEN];

    // if (piping == 1)
    // {
    //     sprintf(word_count_str, "word count: %ld\n", word_count);
    // }
    // else
    // {
    //     sprintf(word_count_str, "\nword count: %ld\n", word_count);
    // }
    sprintf(word_count_str, "word count %ld\n", word_count);
    sprintf(char_count_str, "character count %ld\n", total_char_count);
    sprintf(line_count_str, "newline count %ld\n", line_count);

    display_message(word_count_str);
    display_message(char_count_str);
    display_message(line_count_str);

    return 0;
}

ssize_t cmd_kill(char **tokens)
{
    // int count;
    // for (count = 1; tokens[count - 1] != NULL; count++){}

    // if (count > 3) {
    //           display_error("ERROR: Too many arguments: kill takes a pid and a signum", "");
    //     return -1;
    // }

    // if (count < 2) {
    //     return -1;
    // }

    if (tokens[1] == NULL)
    {
        return -1;
    }

    pid_t pid = atoi(tokens[1]);
    if (pid <= 0)
    {
        return -1;
    }

    int signum;
    if (tokens[2] == NULL)
    {
        signum = SIGTERM;
    }
    else
    {
        signum = atoi(tokens[2]);
        if (signum == 0)
        {
            return -1;
        }
    }

    if (signum < 1 || signum > 31)
    {
        display_error("ERROR: Invalid signal specified", "");
        return -1;
    }

    if (kill(pid, signum) == -1)
    {
        display_error("ERROR: The process does not exist", "");
        return -1;
    }

    return 0;
}

int num_process = 0;

ssize_t cmd_bgproc(char **tokens)
{
    int tokencount;
    for (tokencount = 0; tokens[tokencount] != NULL; tokencount++)
    {
    }

    if (tokencount < 2)
    {
        return -1;
    }

    num_process++;
    pid_t pid = fork();

    if (pid < 0)
    {
        display_error("ERROR: fork failed", "");
        return -1;
    }
    else if (pid > 0)
    {

        char buffer[MAX_STR_LEN + 1];

        sprintf(buffer, "[%d] %d\n", num_process, pid);
        display_message(buffer);

        // Create a new background process node
        Bgprocess *new_process = malloc(sizeof(Bgprocess));
        if (!new_process)
        {
            display_error("ERROR: Memory allocation failed", "");
            return -1;
        }

        new_process->num = num_process;
        new_process->pid = pid;
        new_process->command = combine_tokens(tokens);

        new_process->next = bg_process_list; // Assuming you have a head pointer for the list
        bg_process_list = new_process;       // Add to the linked list

        waitpid(pid, NULL, 1);

        return 0;
    }
    else if (pid == 0)
    {

        char **args = malloc((tokencount + 1) * sizeof(char *));
        if (!args)
        {
            display_error("ERROR: Memory allocation failed", "");
            return -1;
        }

        int arg_index = 0;
        for (int i = 0; i < tokencount - 1; i++)
        {
            args[arg_index++] = tokens[i];
        }
        args[arg_index] = NULL;

        execvp(tokens[0], args);

        display_error("ERROR: Could not run the process", "");
        free(args);
        return -1;
    }
    return 0;
}

void check_finished_process()
{
    Bgprocess *prev = NULL;
    Bgprocess *curr = bg_process_list;

    while (curr != NULL)
    {
        int status;
        pid_t result = waitpid(curr->pid, &status, 1);

        if (result > 0)
        {
            char buffer[MAX_STR_LEN + 1];
            sprintf(buffer, "[%d]+  Done %s\n", curr->num, curr->command);
            display_message(buffer);

            if (prev == NULL)
            {
                bg_process_list = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }

            // Free memory
            free(curr->command);
            Bgprocess *temp = curr;
            curr = curr->next;
            free(temp);

            num_process--;
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
    reorder_bg_process_list();
}

void reorder_bg_process_list()
{
    // Step 1: Count the number of nodes in the list
    int count = 0;
    Bgprocess *current = bg_process_list;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    // Step 2: Reassign the num values
    current = bg_process_list;
    while (current != NULL)
    {
        current->num = count;
        count--;
        current = current->next;
    }
}

char *combine_tokens(char **tokens)
{
    if (tokens == NULL || tokens[0] == NULL)
    {
        return NULL;
    }

    int total_length = 0;
    int i;

    for (i = 0; tokens[i] != NULL && strcmp(tokens[i], "&") != 0; i++)
    {
        total_length += strlen(tokens[i]) + 1;
    }

    if (total_length == 0)
    {
        return NULL;
    }

    char *combined = malloc(total_length);
    if (!combined)
    {
        return NULL;
    }

    combined[0] = '\0';

    for (int j = 0; j < i; j++)
    {
        strcat(combined, tokens[j]);
        if (j < i - 1)
            strcat(combined, " ");
    }

    return combined;
}

ssize_t cmd_ps(char **tokens)
{
    if (tokens[1] != NULL)
    {
        display_error("ERROR: Too many arguments", "");
        return -1;
    }

    Bgprocess *curr = bg_process_list;

    if (curr == NULL)
    {
        return 0;
    }

    while (curr != NULL)
    {
        char buffer[MAX_STR_LEN + 1];
        sprintf(buffer, "%s %i\n", curr->command, curr->pid);
        display_message(buffer);
        curr = curr->next;
    }
    return 0;
}