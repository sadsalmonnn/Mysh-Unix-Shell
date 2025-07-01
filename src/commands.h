#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <unistd.h>
#include <dirent.h>

typedef ssize_t (*cmd_ptr)(char **);
cmd_ptr check_cmd(const char *cmd);

ssize_t cmd_ls(char **tokens);
ssize_t ls_helper(char *path, int depth, char *substring);

ssize_t cmd_cd(char **tokens);
ssize_t cmd_cat(char **tokens);
ssize_t cmd_wc(char **tokens);
ssize_t cmd_kill(char **tokens);
ssize_t cmd_bgproc(char **tokens);
ssize_t cmd_ps(char **tokens);

typedef struct Bgprocess
{
    int num;
    char *command;
    pid_t pid;
    struct Bgprocess *next;
} Bgprocess;

extern Bgprocess *bg_process_list; // {ls, |wc|, ls} -> {ls, | , wc, |, ls}
extern int piping;
void clear_stdin();

static const char *const COMMANDS[] = {"ls", "cd", "cat", "wc", "kill", "ps"};
static const cmd_ptr COMMANDS_FN[] = {cmd_ls, cmd_cd, cmd_cat, cmd_wc, cmd_kill, cmd_ps, NULL}; // Extra null element for 'non-command'
static const ssize_t COMMANDS_COUNT = sizeof(COMMANDS) / sizeof(char *);

void check_finished_process();
char *combine_tokens(char **tokens);
void reorder_bg_process_list();

#endif