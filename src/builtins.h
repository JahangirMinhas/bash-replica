#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include <unistd.h>

#ifndef MAX_BACKLOG
    #define MAX_BACKLOG 5
#endif

#ifndef MAX_USR_MSG
    #define MAX_USR_MSG 128
#endif

#ifndef BUF_SIZE
    #define BUF_SIZE MAX_USR_MSG + 3 
#endif

int server;

/* Type for builtin handling functions
 * Input: Array of tokens
 * Return: >=0 on success and -1 on error
 */
typedef ssize_t (*bn_ptr)(char **);
ssize_t bn_echo(char **tokens);
ssize_t cat(char **tokens);
ssize_t wc(char **tokens);
ssize_t cd(char **tokens);
ssize_t ls(char **tokens);
ssize_t ps(char **tokens);
ssize_t start_server(char **tokens);
ssize_t send_server(char **tokens);
ssize_t close_server(char **tokens);
ssize_t kill_proc(char **tokens);
ssize_t handle_piping(char *in_ptr);

char *get_name(char *token);

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd);

int check_assignment(char *in_ptr, char **tokens);

/* BUILTINS and BUILTINS_FN are parallel arrays of length BUILTINS_COUNT
 */
static const char * const BUILTINS[] = {"echo", "cat", "wc", "cd", "ls", "kill", "ps", "start-server", "send", "close-server"};
static const bn_ptr BUILTINS_FN[] = {bn_echo, cat, wc, cd, ls, kill_proc, ps, start_server, send_server, close_server, NULL};    // Extra null element for 'non-builtin'
static const size_t BUILTINS_COUNT = sizeof(BUILTINS) / sizeof(char *);

#endif
