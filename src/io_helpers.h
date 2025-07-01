#ifndef __IO_HELPERS_H__
#define __IO_HELPERS_H__

#include <sys/types.h>

#define MAX_STR_LEN 128
#define DELIMITERS " \t\n" // Assumption: all input tokens are whitespace delimited

/* Prereq: pre_str, str are NULL terminated string
 */
void display_message(char *str);
void display_error(char *pre_str, char *str);

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr);

/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens);

// char *expand(char *token, char *result);
void expand(char *token);
int check_for_pipe(char ** tokens);
int run_cmd_blt(char ** tokens, int token_count);
// char *expand(char *token);

// ssize_t expand(char *token, char *result, ssize_t remaining_space);
// char *process_string(const char *input);
// size_t expand(char *token, char *result, size_t remaining_char);
#endif
