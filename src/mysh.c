#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"

void sig_handler(int signo)
{
  if (signo == SIGINT){
        char *prompt = "mysh$ ";
        display_message("\n");
        display_message(prompt);
  }
}

int main(int argc, char* argv[]) {

    signal(SIGINT, sig_handler);
    sigset_t sigset;
    sigemptyset(&sigset);

    char *prompt = "mysh$ ";
    char input_buf[MAX_STR_LEN + 1];
    input_buf[MAX_STR_LEN] = '\0';
    char *token_arr[MAX_STR_LEN] = {NULL};
    num_items = 0;

    init_list();
    while (1) {
        int i = 0;
        int num = num_items;
        while(i < num){
            if(waitpid(processes[i].pid, NULL, WNOHANG) > 0){
                display_done(i);
                processes[i].pid = 0;
                processes[i].name[0] = '\0';
                processes[i].num = 0;
                num_items--;

                int k = i + 1;
                while(k < num){
                    processes[k - 1].pid = processes[k].pid;
                    strcpy(processes[k - 1].name, processes[k].name);
                    processes[k - 1].num = processes[k].num;
                    k++;
                }
            }
            i++;
        }
        if(check_proc() == 1){
            num_items = 0;
        }
        // Prompt and input tokenization
        // TODO Step 2:
        // Display the prompt via the display_message function.
	    display_message(prompt);

        int ret = get_input(input_buf);
        char input[MAX_STR_LEN + 1];
        strcpy(input, input_buf);
        size_t token_count = tokenize_input(input_buf, token_arr);

        // Clean exit
        if (ret != -1 && (token_count == 0 || (strncmp("exit", token_arr[0], 4) == 0))) {  
            if(input_buf[0] != 10){  
                break;  
            } 
        }

        // Command execution
        if (token_count >= 1) {
            if(strchr(input, '|') != NULL){
                handle_piping(input);
            }else if(token_count == 1 && strchr(input, '=') != NULL){
                handle_assign(token_arr);
            }else if(token_count >= 1){
                handle_variable(token_arr);
            }else{
                display_error("ERROR: Unrecognized command: ", token_arr[0]);
            }
        }
    }
    if(first != NULL){
        Node *current = first;
        Node *list;
        while(current != NULL){
            free(current->data);
            free(current->names);
            list = current->next;
            free(current);
            current = list;
        }
    }
    return 0;
}
