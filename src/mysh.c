#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"
#include "builtins.h"
#include "commands.h"
#include <sys/wait.h>
#include <signal.h>
void handler() { display_message("\n"); }

// You can remove attribute((unused)) once argc and argv are used.
int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[])
{

    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    char *prompt = "mysh$ "; // TODO Step 1, Uncomment this.

    char input_buf[MAX_STR_LEN + 1];
    input_buf[MAX_STR_LEN] = '\0';
    char *token_arr[MAX_STR_LEN] = {NULL};

    ssize_t token_count;

    // char cwd[1024];
    // if (getcwd(cwd, sizeof(cwd)) != NULL)
    // {
    //     printf("Current directory: %s\n", cwd);
    // }

    while (1)
    {
        piping = 0;
        // Prompt and input tokenization

        // TODO Step 2:
        // Display the prompt via the display_message function.
        display_message(prompt);
        int ret = get_input(input_buf);

        check_finished_process();

        // printf("%d\n", ret);

        token_count = tokenize_input(input_buf, token_arr);
        // fflush(stdout);
        // fflush(stdin);
        int saved_stdout = dup(STDOUT_FILENO);
        int saved_stdin = dup(STDIN_FILENO);

        // printf("%lu\n", token_count);

        // Clean exit -> ctrl+d or "exit"
        // TODO: The next line has a subtle issue.
        if ((ret != -1 && (token_count == 0 || (strncmp("exit", token_arr[0], 4) == 0))) && !(ret > 0 && token_count == 0))
        {
            break;
        }

        // 0, 0
        // 5, 1

        // Command execution
        if (token_count >= 1)
        {
            cmd_ptr cmd_fn;
            if (strcmp(token_arr[token_count - 1], "&") == 0)
            {
                cmd_fn = cmd_bgproc;
                ssize_t err = cmd_fn(token_arr);
                if (err == -1)
                {
                    display_error("ERROR: Builtin failed: ", token_arr[0]);
                }
                continue;
            }
            if (strcmp(token_arr[0], "|") != 0 && check_for_pipe(token_arr))
            {
                piping = 1;

                int count = 0;
                int fd[2];
                int temp[2];
                int run_check;
                pipe(fd);
                dup2(fd[1], STDOUT_FILENO);

                pid_t pid = fork();

                if (pid == 0)
                {
                    run_check = run_cmd_blt(token_arr, token_count);
                    close(fd[1]);
                    if (run_check == -1)
                    {
                        return -1;
                    }
                }
                else if (pid > 0)
                {
                    close(fd[1]);
                    waitpid(pid, NULL, 0);
                }
                else
                {
                    close(fd[1]);
                    display_error("ERROR: Could not run the process", "");
                    continue;
                }

                int flags = fcntl(fd[0], F_GETFL, 0);
                fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

                // int c;
                // while ((c = getchar()) != '\n' && c != EOF)
                //     ;

                // token_count = token_count - run_check - 1;
                for (int x = 0; token_arr[x] != NULL; x++)
                {
                    if (strcmp(token_arr[x], "|") == 0)
                    {

                        if (token_arr[x + 1] != NULL && strcmp(token_arr[x + 1], "|") != 0)
                        {
                            if (count % 2 == 0)
                            {
                                dup2(fd[0], STDIN_FILENO);

                                if (check_for_pipe(token_arr + x + 1))
                                { // if there exists a future pipe
                                    pipe(temp);

                                    int flags = fcntl(temp[0], F_GETFL, 0);
                                    fcntl(temp[0], F_SETFL, flags | O_NONBLOCK);

                                    dup2(temp[1], STDOUT_FILENO);

                                    pid_t pid = fork();

                                    if (pid == 0)
                                    {
                                        run_check = run_cmd_blt(token_arr + x + 1, token_count);
                                        if (run_check == -1)
                                        {
                                            return -1;
                                        }

                                        // token_count -= run_check + 1;
                                        close(fd[0]);
                                        close(temp[1]);
                                    }
                                    else if (pid > 0)
                                    {
                                        close(temp[1]);
                                        close(fd[0]);
                                        waitpid(pid, NULL, 0);
                                    }
                                    else
                                    {
                                        display_error("ERROR: Could not run the process", "");
                                        close(temp[1]);
                                        close(fd[0]);
                                        continue;
                                    }
                                }
                                else
                                {
                                    dup2(saved_stdout, STDOUT_FILENO);

                                    pid_t pid = fork();

                                    if (pid == 0)
                                    {
                                        run_check = run_cmd_blt(token_arr + x + 1, token_count);

                                        if (run_check == -1)
                                        {
                                            return -1;
                                        }

                                        // token_count -= run_check + 1;
                                        close(fd[0]);
                                    }
                                    else if (pid > 0)
                                    {
                                        close(fd[0]);
                                        waitpid(pid, NULL, 0);
                                    }
                                    else
                                    {
                                        display_error("ERROR: Could not run the process", "");
                                        close(fd[0]);
                                        continue;
                                    }
                                }
                            }
                            else if (count % 2 == 1)
                            {
                                dup2(temp[0], STDIN_FILENO);
                                if (check_for_pipe(token_arr + x + 1))
                                {
                                    pipe(fd);

                                    int flags = fcntl(fd[0], F_GETFL, 0);
                                    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

                                    dup2(fd[1], STDOUT_FILENO);

                                    pid_t pid = fork();
                                    if (pid == 0)
                                    {
                                        run_check = run_cmd_blt(token_arr + x + 1, token_count);
                                        if (run_check == -1)
                                        {
                                            return -1;
                                        }
                                        close(temp[0]);
                                        close(fd[1]);
                                    }
                                    else if (pid > 0)
                                    {
                                        close(temp[0]);
                                        close(fd[1]);
                                        waitpid(pid, NULL, 0);
                                    }
                                    else
                                    {
                                        display_error("ERROR: Could not run the process", "");
                                        close(temp[0]);
                                        close(fd[1]);
                                        continue;
                                    }
                                }
                                else
                                {
                                    dup2(saved_stdout, STDOUT_FILENO);

                                    pid_t pid = fork();

                                    if (pid == 0)
                                    {
                                        run_check = run_cmd_blt(token_arr + x + 1, token_count);
                                        if (run_check == -1)
                                        {
                                            return -1;
                                        }
                                        close(temp[0]);
                                    }
                                    else if (pid > 0)
                                    {
                                        close(temp[0]);
                                        waitpid(pid, NULL, 0);
                                    }
                                    else
                                    {
                                        display_error("ERROR: Could not run the process", "");
                                        close(temp[0]);
                                        continue;
                                    }
                                }
                            }

                            count++;
                        }
                    }
                }
                // Restore stdout and stdin
                dup2(saved_stdin, STDIN_FILENO);
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
                close(saved_stdin);
                continue;
            }

            // if (strcmp(token_arr[0], "|") != 0 && check_for_pipe(token_arr))
            // {
            //     // piping = 1;

            //     int count = 0;
            //     int fd[2];
            //     int temp[2];
            //     int run_check;
            //     pipe(fd);
            //     dup2(fd[1], STDOUT_FILENO);

            //     run_check = run_cmd_blt(token_arr, token_count);
            //     close(fd[1]);
            //     if (run_check == -1)
            //     {
            //         return -1;
            //     }

            //     // int c;
            //     // while ((c = getchar()) != '\n' && c != EOF)
            //     //     ;

            //     // token_count = token_count - run_check - 1;
            //     for (int x = 0; token_arr[x] != NULL; x++)
            //     {
            //         if (strcmp(token_arr[x], "|") == 0)
            //         {

            //             if (token_arr[x + 1] != NULL && strcmp(token_arr[x + 1], "|") != 0)
            //             {
            //                 if (count % 2 == 0)
            //                 {
            //                     dup2(fd[0], STDIN_FILENO);

            //                     if (check_for_pipe(token_arr + x + 1))
            //                     { // if there exists a future pipe
            //                         pipe(temp);
            //                         dup2(temp[1], STDOUT_FILENO);
            //                         run_check = run_cmd_blt(token_arr + x + 1, token_count);
            //                         if (run_check == -1)
            //                         {
            //                             return -1;
            //                         }

            //                         // token_count -= run_check + 1;
            //                         close(fd[0]);
            //                         close(temp[1]);
            //                     }
            //                     else
            //                     {
            //                         dup2(saved_stdout, STDOUT_FILENO);
            //                         run_check = run_cmd_blt(token_arr + x + 1, token_count);

            //                         if (run_check == -1)
            //                         {
            //                             return -1;
            //                         }

            //                         // token_count -= run_check + 1;
            //                         close(fd[0]);
            //                     }
            //                 }
            //                 else if (count % 2 == 1)
            //                 {
            //                     dup2(temp[0], STDIN_FILENO);
            //                     if (check_for_pipe(token_arr + x + 1))
            //                     {
            //                         pipe(fd);
            //                         dup2(fd[1], STDOUT_FILENO);
            //                         run_check = run_cmd_blt(token_arr + x + 1, token_count);
            //                         if (run_check == -1)
            //                         {
            //                             return -1;
            //                         }
            //                         close(temp[0]);
            //                         close(fd[1]);
            //                     }
            //                     else
            //                     {
            //                         dup2(saved_stdout, STDOUT_FILENO);
            //                         run_check = run_cmd_blt(token_arr + x + 1, token_count);
            //                         if (run_check == -1)
            //                         {
            //                             return -1;
            //                         }
            //                         close(temp[0]);
            //                     }
            //                 }

            //                 count++;
            //             }
            //         }
            //     }
            //     // Restore stdout and stdin
            //     dup2(saved_stdin, STDIN_FILENO);
            //     continue;
            // }

            bn_ptr builtin_fn = check_builtin(token_arr[0]);
            if (builtin_fn != NULL)
            {
                ssize_t err = builtin_fn(token_arr);
                if (err == -1)
                {
                    display_error("ERROR: Builtin failed: ", token_arr[0]);
                }
                continue;
            }

            cmd_fn = check_cmd(token_arr[0]);

            if (cmd_fn != NULL)
            {
                // display_message(token_arr[0]);
                ssize_t err = cmd_fn(token_arr);

                if (err == -1)
                {
                    display_error("ERROR: Builtin failed: ", token_arr[0]);
                }

                // if (getcwd(cwd, sizeof(cwd)) != NULL)
                // {
                //     display_message("Current directory: ");
                //     display_message(cwd);
                //     display_message("\n");
                // }
                continue;
            }

            // pid_t pid = fork();
            // execvp("/bin/" + tokens);

            char bin[MAX_STR_LEN];
            snprintf(bin, MAX_STR_LEN, "/bin/%s", token_arr[0]);

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
                execvp(bin, bin_cmds);

                snprintf(bin, MAX_STR_LEN, "/user/bin/%s", token_arr[0]);
                bin_cmds[0] = bin;
                execvp(bin, bin_cmds);

                display_error("ERROR: Unknown command: ", token_arr[0]);
                free(bin_cmds);
                exit(EXIT_FAILURE);
            }
            else if (pid > 0)
            {
                waitpid(pid, NULL, 0);
                free(bin_cmds);
            }
            else
            {
                display_error("ERROR: Unknown command: ", token_arr[0]);
                free(bin_cmds);
            }
        }
    }

    // // Free the memory allocated for tokens
    // for (ssize_t i = 0; i < token_count; i++)
    // {
    //     free(piped_token_arr[i]); // Free each token
    // }

    free_list(myshfrontvar);
    return 0;
}