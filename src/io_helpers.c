#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "io_helpers.h"
#include "builtins.h"
#include "variables.h"


// ===== Output helpers =====

/* Prereq: str is a NULL terminated string
 */
void display_message(char *str) {
    write(STDOUT_FILENO, str, strnlen(str, MAX_STR_LEN));
}


/* Prereq: pre_str, str are NULL terminated string
 */
void display_error(char *pre_str, char *str) {
    write(STDERR_FILENO, pre_str, strnlen(pre_str, MAX_STR_LEN));
    write(STDERR_FILENO, str, strnlen(str, MAX_STR_LEN));
    write(STDERR_FILENO, "\n", 1);
}

void write_to_file(char *str, FILE *fp){
    fputs(str, fp);
}


// ===== Input tokenizing =====

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr) {
    int retval = read(STDIN_FILENO, in_ptr, MAX_STR_LEN+1); // Not a sanitizer issue since in_ptr is allocated as MAX_STR_LEN+1
    int read_len = retval;
    if (retval == -1) {
        read_len = 0;
    }
    if (read_len > MAX_STR_LEN) {
        read_len = 0;
        retval = -1;
        write(STDERR_FILENO, "ERROR: input line too long\n", strlen("ERROR: input line too long\n"));
        int junk = 0;
        while((junk = getchar()) != EOF && junk != '\n');
    }
    in_ptr[read_len] = '\0';
    return retval;
}

/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens) {
    int i = 0;
    while(tokens[i] != NULL){
        tokens[i] = NULL;
        i++;
    }
    // TODO, uncomment the next line.
    char *curr_ptr = strtok (in_ptr, DELIMITERS);
    size_t token_count = 0;

    while (curr_ptr != NULL) {  // TODO: Fix this
       	tokens[token_count] = curr_ptr;
	    curr_ptr = strtok (NULL, DELIMITERS);
	    token_count++;
    }
    tokens[token_count] = NULL;
    return token_count;
}

char *separate_name(char *input){
    char name_r[MAX_STR_LEN];
    char *name;
    name = malloc(sizeof(char*) * MAX_STR_LEN);
    int index;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '='){
            index = i;
            break;
        }
    }
    strncpy(name_r, input, index);
    int get_size = (strchr(input, '=')) - input;
    strcpy(name, name_r);
    name[get_size] = '\0';
    return name;
}

char *separate_value(char *input){
    char value_r[MAX_STR_LEN];
    char *value;
    value = malloc(sizeof(char*) * MAX_STR_LEN);
    int index;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '='){
            index = i;
            break;
        }
    }
    strncpy(value_r, &(input[index + 1]), strlen(input) - index);
    strcpy(value, value_r);
    value[strlen(input) - index - 1] = '\0';
    return value;
}

char* get_value(char* name){
    char value[MAX_STR_LEN];
    Node *curr = first;
    while(curr != NULL){
        if(strcmp(name, curr->names) == 0){
            strcpy(value, curr->data);
        }
        curr = curr->next;
    }
    char* val_ptr = value;
    return val_ptr;
}

void handle_command(char **tokens){
    int pid;
    bn_ptr builtin_fn = check_builtin(tokens[0]);
    if (builtin_fn != NULL) {
        ssize_t err = builtin_fn(tokens);
        if (err == - 1) {
            display_error("ERROR: Builtin failed: ", tokens[0]);
        }
    }else if(strchr(tokens[0], '=') != NULL){
        return;
    }else{
        pid = fork();
        if(pid < 0){
            display_error("ERROR: ", "Error while trying to fork.");
            exit(EXIT_FAILURE);
        }else if(pid == 0){
            int ret = execvp(tokens[0], tokens);
            if (ret == -1) {
                display_error("ERROR: Unrecognized command: ",  tokens[0]);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }
    waitpid(pid, NULL, 0);
}

int check_bg(char **tokens){
    int i = 0;
    while(tokens[i + 1] != NULL){
        i++;
    }
    if(strcmp(tokens[i], "&") == 0){
        tokens[i] = NULL;
        return 1;
    }

    if((tokens[i])[strlen(tokens[i]) - 1] == '&'){
        (tokens[i])[strlen(tokens[i]) - 1] = '\0';
        return 1;
    }
    return 0;
}

void display_start(){
    char str_num[MAX_STR_LEN];
    char str_pid[MAX_STR_LEN];
    sprintf(str_num, "%d", processes[num_items - 1].num);
    sprintf(str_pid, "%d", getpid());
    display_message("[");
    display_message(str_num);
    display_message("] ");
    display_message(str_pid);
    display_message("\n");
}

void display_done(int i){
    char str_num[MAX_STR_LEN];
    sprintf(str_num, "%d", processes[i].num);
    display_message("[");
    display_message(str_num);
    display_message("]+  Done  ");
    display_message(processes[i].name);
    display_message("\n");
}

int check_proc(){
    int i = 0;
    int status;
    while(i < num_items){
        waitpid(processes[i].pid, &status, WNOHANG);
        if(waitpid(processes[i].pid, NULL, WNOHANG) == 0){
            return 0;
        }
        i++;
    }
    return 1;
}

void handle_variable(char **token_arr){
    int bg = check_bg(token_arr);
    int pid;
    int fd[2];
    if(bg == 1){
        pipe(fd);
        num_items++;
        processes[num_items - 1].num = num_items;
        processes[num_items - 1].name[0] = '\0';
        strcat(processes[num_items - 1].name, token_arr[0]);
        int j = 1;
        while(token_arr[j] != NULL){
            strcat(processes[num_items - 1].name, " ");
            strcat(processes[num_items - 1].name, token_arr[j]);
            j++;
        }
        pid = fork();
        if(pid < 0){
            display_error("ERROR: ", "Error while trying to fork.");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            close(fd[0]);
            int child_pid = getpid();
            if(write(fd[1], &child_pid, sizeof(int)) < 0){
                display_error("ERROR: ", "Error while trying to write.");
                exit(EXIT_FAILURE);
            }
            display_start();
            close(fd[1]);
        }
    }

    if(bg == 1 && pid > 0){
        close(fd[1]);
        read(fd[0], &pid, sizeof(int));
        processes[num_items - 1].pid = pid;
        return;
    }

    if(strchr(token_arr[0], '$') != NULL){
        if(first != NULL){
            char *token_name = get_name(token_arr[0]);
            char *value = get_value(token_name);
            token_arr[0] = value;
            handle_command(token_arr);
        }else{
            display_error("ERROR: Unrecognized command: ",  token_arr[0]);
        }
    }else{
        handle_command(token_arr);
    }
    if(pid == 0){
        exit(EXIT_SUCCESS);
    }
}

void handle_assign(char **token_arr){
    char *name = separate_name(token_arr[0]);
    char *value = separate_value(token_arr[0]);
    set_variable(name, value);
    free(name);
    free(value);
}

char **fix_tokens(char **tokens){
    int i = 0;
    while(tokens[i] != NULL){
        if((tokens[i])[0] == '$'){
            char* name = get_name(tokens[i]);
            tokens[i] = get_value(name);
        } 
        i++;
    }
    return tokens;
}