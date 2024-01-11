#ifndef __IO_HELPERS_H__
#define __IO_HELPERS_H__

#include <sys/types.h>


#define MAX_STR_LEN 64
#define DELIMITERS " \t\n"     // Assumption: all input tokens are whitespace delimited

struct process{
    int num;
    char name[MAX_STR_LEN];
    int pid;
};
struct process processes[MAX_STR_LEN];
int num_items;

/* Prereq: pre_str, str are NULL terminated string
 */
void display_message(char *str);
void display_error(char *pre_str, char *str);
void display_done(int i);

char *separate_name(char *input);
char *separate_value(char *input);
void handle_variable(char **token_arr);
void handle_assign(char **token_arr);
char *get_value(char* name);
char **fix_tokens(char **tokens);
void write_to_file(char *str, FILE *fp);
void display_start();
int check_proc();
int check_bg(char **tokens);

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr);


/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens);


#endif