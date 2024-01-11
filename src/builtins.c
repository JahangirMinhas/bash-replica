#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"

char *get_name(char *token){
    char token_name[MAX_STR_LEN];
    char *token_ptr;
    strncpy(token_name, token + 1, strlen(token) - 1);
    token_name[strlen(token) - 1] = '\0';
    token_ptr = token_name;
    return token_ptr;
}

// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd) {
    ssize_t cmd_num = 0;
    while (cmd_num < BUILTINS_COUNT &&
           strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0) {
        cmd_num += 1;
    }
    return BUILTINS_FN[cmd_num];
}

char **get_stdin(char **tokens){
    char output[MAX_STR_LEN];  
    output[0] = '\0';      
    fgets(output, MAX_STR_LEN, stdin);
    output[strlen(output) - 1] = '\0';
    tokens[1] = output;
    return tokens;
}

// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo. 
 */
ssize_t bn_echo(char **tokens) {
    if(tokens[1] == NULL){
        tokens = get_stdin(tokens);
        if(tokens[1][0] == '\0'){
            display_error("ERROR: ", "No input source provided");
            return(-1);
        }
    }
    ssize_t index = 1;
    int exist = 0;
    char *token_name;
    if(strchr(tokens[index], '$') != NULL){
        token_name = get_name(tokens[index]);
    }else{
        token_name = tokens[index];
    }
    if (tokens[index] != NULL) {
        if (strchr(tokens[index], '$') != NULL && strlen(tokens[index]) > 1){
            Node *curr = first;
		    while(curr != NULL){
			    if(strcmp(curr->names, token_name) == 0){
                    display_message(curr->data);
                    exist = 1;
                    break;
                }else{
                    curr = curr->next;
                }
		    }
            if(exist == 0){
                display_message(tokens[index]);
            }
            index += 1;
        }else{
            display_message(tokens[index]);
            index++;
        }
    }
    while (tokens[index] != NULL) {
        int exist = 0;
        char *token_name_c;
        if(tokens[index][0] == '$'){
            token_name_c = get_name(tokens[index]);
        }else{
            token_name_c = tokens[index];
        }
        // TODO:
        // Implement the echo command
        if (strchr(tokens[index], '$') != NULL && strlen(tokens[index]) > 1){
            Node *curr = first;
		    while(curr != NULL){
			    if(strcmp(curr->names, token_name_c) == 0){
                    display_message(" ");
                    display_message(curr->data);
                    exist = 1;
                    break;
                }else{
                    curr = curr->next;
                }
		    }
            if(exist == 0){
                display_message(" ");
                display_message(tokens[index]);
            }
            index += 1;
        }else{
            display_message(" ");
            display_message(tokens[index]);
            index += 1;
        }
    }
    display_message("\n");
    return 0;
}

ssize_t cat(char **tokens){
    tokens = fix_tokens(tokens);
    FILE *f;
    if(tokens[1] == NULL){
        int fd = 0;
        int ret;
        struct pollfd fds[1];
        int time;
        fds[0].fd = fd;
        fds[0].events = 0;
        fds[0].events |= POLLIN;
        time = 10;
        ret = poll(fds, 1, time);
        if(ret == 0){
            display_error("ERROR: ", "No input source provided");
            return -1;
        }
        f = stdin;
    }else{
        f = fopen(tokens[1], "r");
        if(f == NULL){
            display_error("ERROR: ", "Cannot open file");
            return -1;
        }
    }
    int letter = fgetc(f);
    while(1){
        if(feof(f)) {
            break ;
        }
        char c = (char)letter;
        char cc[2] = {c, '\0'};
        display_message(cc);
        letter = fgetc(f);
    }
    return 0;
}

ssize_t wc(char **tokens){
    tokens = fix_tokens(tokens);
    FILE *f;
    if(tokens[1] == NULL){
        int fd = 0;
        int ret;
        struct pollfd fds[1];
        int time;
        fds[0].fd = fd;
        fds[0].events = 0;
        fds[0].events |= POLLIN;
        time = 10;
        ret = poll(fds, 1, time);
        if(ret == 0){
            display_error("ERROR: ", "No input source provided");
            return -1;
        }
        f = stdin;
    }else{
        f = fopen(tokens[1], "r");
        if(f == NULL){
            display_error("ERROR: ", "Cannot open file");
            return(-1);
        }
    }
    int letter = fgetc(f);
    char last;
    int word = 0;
    int character = 0;
    int newline = 0;
    do{
        if(feof(f)) {
            break ;
        }
        if (letter == 32 && last != 32 && last != 0 && last != 10){
            word++;
        }else if(letter == 10){
            newline++;
            if(last != 32 && last != 10 && last != 0){
                word++;
            }
        }
        character++;
        last = letter;
        letter = fgetc(f);
    }while(1);
    char words[MAX_STR_LEN];
    char characters[MAX_STR_LEN];
    char newlines[MAX_STR_LEN];
    sprintf(words, "%d", word);
    sprintf(characters, "%d", character);
    sprintf(newlines, "%d", newline);
    display_message("word count ");
    display_message(words);
    display_message("\n");
    display_message("character count ");
    display_message(characters);
    display_message("\n");
    display_message("newline count ");
    display_message(newlines);
    display_message("\n");
    return 0;
}

ssize_t cd(char **tokens){
    tokens = fix_tokens(tokens);
    int dots = 0;
    int j = 0;
    char path[256];
    int i;
    if(tokens[1] != NULL){
        for(i = 0; i < strlen(tokens[1]); i++){
            if(tokens[1][i] == '.'){
                dots++;
            }else if(tokens[1][i] == '/'){
                dots = 0;
            }
            if(dots > 2){
                if(j == 0){
                    j = i;
                }
                path[j] = '/';
                path[j + 1] = '.';
                path[j + 2] = '.';
                j = j + 2;
                //}
            }else{
                path[j] = tokens[1][i];
            }
            j++;
        }
}
    path[j] = '\0';
    if(chdir(path) == 0){
        return 0;
    }else{
        display_error("ERROR: ", "Invalid path");
        return -1;
    }
}

void rec_traversal(char* path, int i, int d, char *substring){
    DIR* directory = opendir(path);
    struct dirent* node;
    if (i > d || directory == NULL) {
        return;
    }

    node = readdir(directory);
    while (node != NULL) {
        if(node->d_type == DT_DIR){
            if((substring != NULL && strstr(node->d_name, substring)) || substring == NULL){
                display_message(node->d_name);
                display_message("\n");
            }
            if(strcmp(node->d_name, ".") != 0 && strcmp(node->d_name, "..") != 0){
                if(!(i + 1 > d)){
                    char joint_path[200] = {'\0'};
                    strcat(joint_path, path);
                    strcat(joint_path, "/");
                    strcat(joint_path, node->d_name);
                    rec_traversal(joint_path, i + 1, d, substring);
                }
            }
        }else if(node->d_type == DT_REG){
            if((substring != NULL && strstr(node->d_name, substring)) || substring == NULL){
                display_message(node->d_name);
                display_message("\n");
            }
        }
        node = readdir(directory);
    }
    closedir(directory);
    return;
}

ssize_t ls(char **tokens){
    int i = 0;
    int found = 0;
    int found_path = 0;
    DIR *directory;
    struct dirent *node = NULL;
    char path[MAX_STR_LEN] = {'\0'};
    int d = 0;
    char *substring = NULL;

    tokens = fix_tokens(tokens);

    while(tokens[i] != NULL){
        if((strcmp(tokens[i], "--rec") == 0) || (strcmp(tokens[i], "--d") == 0)){
            if(found_path == 1){
                display_error("ERROR: ", "invalid arguements.");
                return -1;
            }
            if(tokens[i + 3] != NULL && found != 1){
                found = 1;
                if(tokens[i + 1] == NULL){
                    display_error("ERROR: ", "--rec and --d must be provided together");
                    return -1;
                }
                if((strcmp(tokens[i], "--rec") == 0) && (strcmp(tokens[i + 2], "--d") != 0)){
                    display_error("ERROR: ", "--rec and --d must be provided together");
                    return -1;
                }
                if((strcmp(tokens[i], "--d") == 0) && (strcmp(tokens[i + 2], "--rec") != 0)){
                    display_error("ERROR: ", "--d and --rec must be provided together");
                    return -1;
                }
            }else if(found == 0){
                display_error("ERROR: ", "--rec and --d must be provided together");
                return -1;
            }
        }else if(strcmp(tokens[i], "ls") == 0 && tokens[i + 1] != NULL){
            if((strcmp(tokens[i + 1], "--rec") != 0) && (strcmp(tokens[i + 1], "--d") != 0) && (strcmp(tokens[i + 1], "--f") != 0)){
                strcpy(path, tokens[i + 1]);
                found_path = 1;
            }
        }
        if(strcmp(tokens[i], "--rec") == 0){
           strcpy(path, tokens[i + 1]);
        }else if(strcmp(tokens[i], "--d") == 0){
            d = atoi(tokens[i + 1]);
        }else if((strcmp(tokens[i], "--f") == 0) && tokens[i + 1] != NULL){
            substring = tokens[i + 1];
        }
        i++;
    }

    if(path[0] == '\0'){
        strcpy(path, ".");
    }
    directory = opendir(path);

    
    if(directory == NULL){
        display_error("ERROR: ", "Invalid path");
        return(-1);
    }

    if(path == NULL){
        while((node = readdir(directory))){
            if((substring != NULL && strstr(node->d_name, substring)) || substring == NULL){
                display_message(node->d_name);
                display_message("\n");
            }
        }
    }else{
        rec_traversal(path, 0, d, substring);
    }

    closedir(directory);
    return 0;
}

ssize_t kill_proc(char **tokens){
    tokens = fix_tokens(tokens);
    if(tokens[1] == NULL){
        display_error("ERROR: ", "Provide a process id.");
        return -1;
    }
    int i_pid = atoi(tokens[1]);
    if(tokens[2] == NULL){
        if(kill(i_pid, 15) == -1){
            int i = 0;
            int found = 0;
            while(i < num_items){
                if(i_pid == processes[i].pid){
                    found = 1;
                }
                i++;
            }
            if(found == 0){
                display_error("ERROR: ", "The process does not exist");
            }
            return -1;
        }
    }else{
        int i_sig_num = atoi(tokens[2]);
        if(kill(i_pid, i_sig_num) == -1){
            int i = 0;
            int found = 0;
            while(i < num_items){
                if(i_pid == processes[i].pid){
                    found = 1;
                }
                i++;
            }
            if(i_sig_num < 0 || i_sig_num > 64){
                display_error("ERROR: ", "Invalid signal specified");
            }else if(found == 0 && i_pid != getpid()){
                display_error("ERROR: ", "The process does not exist");
            }
            return -1;
        }
    }
    int i = 0;
    int num = num_items;
    while(i < num){
        if(processes[i].pid == i_pid){
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
    return 0;
}

ssize_t ps(char **tokens){
    int i = 0;
    while(i < num_items){
        char str_num[MAX_STR_LEN];
        char str_pid[MAX_STR_LEN];
        sprintf(str_num, "%d", processes[i].num);
        sprintf(str_pid, "%d", processes[i].pid);
        display_message(processes[i].name);
        display_message(" ");
        display_message(str_pid);
        display_message("\n");
        i++;
    }
    return 0;
}

ssize_t close_server(char **tokens){
    if(close(server) < 0){
        display_error("ERROR: ", "Error closing server");
        return -1;
    }
    server = 0;
    return 0;
}

int read_from_socket(int sock_fd, char *buf, int *inbuf) {
    int read_socket = read(sock_fd, buf + *inbuf, BUF_SIZE - *inbuf);
    *inbuf += read_socket;

    if(read_socket == 0 || *inbuf == BUF_SIZE){
        return -1;
    }

    for(int i = 0; i <= *inbuf - 2; i++){
         if(buf[i] == '\r' && buf[i + 1] == '\n'){
           return 0;
       }
    }
    return 2;
}

int find_network_newline(const char *buf, int inbuf) {
    for(int i = 0; i < inbuf - 1; i++){
       if(buf[i] == '\r' && buf[i+1] == '\n'){
           return i + 2;
       }
   }

    return -1;
}

int get_message(char **dst, char *src, int *inbuf) {
    int size = find_network_newline(src, *inbuf);
    if(size == -1){
        return 1;
    }

    *dst = malloc(BUF_SIZE);
    if(*dst == NULL){
        return 1;
    }

    memmove(*dst, src, size -2);
    (*dst)[size -2] = '\0';
    memmove(src, src + size, BUF_SIZE - size);
    *inbuf -= size;

    return 0;
}

ssize_t send_server(char **tokens){
    if(tokens[1] == NULL){
        display_error("ERROR: ", "No port provided");
        return -1;
    }
    if(tokens[2] == NULL){
        display_error("ERROR: ", "No hostname provided");
        return -1;
    }
    if(tokens[1] == NULL){
        display_error("ERROR: ", "No message provided");
        return -1;
    }

    int SERVER_PORT = atoi(tokens[1]);
    int sendfd;
    struct sockaddr_in addr;

    if((sendfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        display_error("ERROR: ", "Error creating socket.");
        return -1;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    if(inet_pton(AF_INET, tokens[2], &addr.sin_addr) <= 0){
        display_error("ERROR: ", "Inet Pton Error");
        return -1;
    }

    if(connect(sendfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        display_error("ERROR: ", "Connection failed.");
        return -1;
    }
    char buf[MAX_USR_MSG + 2];
    buf[0] = '\0';
    int i = 3;
    while(tokens[i] != NULL){
        strcat(buf, tokens[i]);
        i++;
    }
    write(sendfd, buf, strlen(buf));

    close(sendfd);
    return 0;
}

ssize_t start_server(char **tokens){
    if(server != 0){
        display_error("ERROR: ", "A server already exists.");
        return -1;
    }
    if(tokens[1] == NULL){
        display_error("ERROR: ", "No port provided");
        return -1;
    }

    int pid;
    if((pid = fork()) < 0){
        display_error("ERROR: ", "Error while forking.");
    }else if(pid > 0){
        return 0;
    }

    // Pid == 0 - Child Process - Start Server
    int SERVER_PORT = atoi(tokens[1]);
    int listenfd;
    struct sockaddr_in addr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        display_error("ERROR: ", "Error creating socket.");
    }

    server = listenfd;

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if ((bind(listenfd, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
        display_error("ERROR: ", "Error binding.");
    }

    if ((listen(listenfd, MAX_BACKLOG)) < 0) {
        display_error("ERROR: ", "Error listening.");
    }

    int max_fd = listenfd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds);
    for(;;){
        char exit_stat[MAX_STR_LEN];
        if(read(listenfd, exit_stat, MAX_STR_LEN) == 0){
            exit(EXIT_SUCCESS);
        }
        listen_fds = all_fds;
        select(max_fd + 1, &listen_fds, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &listen_fds)) {
            int client_fd = accept(listenfd, (struct sockaddr *) NULL, NULL);
            if (client_fd < 0) {
                display_error("ERROR: ", "Failed to accept incoming connection.");
            }
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);
        }

        for(int i = 0; i <= max_fd; i++){
            if(FD_ISSET(i, &listen_fds)){
                char buf[MAX_USR_MSG + 2];
                buf[0] = '\0';
                int inbuf = 0;
                int client_closed = 0;
                int num_read = read(i, buf, MAX_USR_MSG + 2);
                //buf[strlen(buf) - 1] = '\0'; 
                // display_message("\n");
                // display_message(buf);
                if (num_read == 0) {
                    client_closed = 1;
                }   

                char *msg;
                while (client_closed == 0 && !get_message(&msg, buf, &inbuf)) {
                    display_message("here");
                    char write_buf[MAX_USR_MSG + 2];
                    write_buf[0] = '\0';
                    strncat(write_buf, msg, MAX_USR_MSG);
                    free(msg);
                    int data_len = strlen(write_buf);
                    write(STDOUT_FILENO, write_buf, data_len);
                }
                if (client_closed == 1) {
                    FD_CLR(i, &all_fds);
                    close(i);
                }
            }
        }
    }
    close(listenfd);
    exit(EXIT_SUCCESS);
}

ssize_t handle_piping(char *in_ptr){
    char* commands[MAX_STR_LEN];
    int bg = 0;
    in_ptr[strlen(in_ptr) - 1] = '\0';
    char input[MAX_STR_LEN + 1];
    strcpy(input, in_ptr);

    int process_num = 1;
    for(int i = 0; i < strlen(in_ptr); i++){
        if(in_ptr[i] == '|'){
            process_num ++;
        }
    }

    char *curr_ptr = strtok(in_ptr, "|");
    int size = 0;
    while (curr_ptr != NULL) {
        if(curr_ptr[0] == 32){
            int size = strlen(curr_ptr);
            for(int i = 0; i < size - 1; i++){
                curr_ptr[i] = curr_ptr[i + 1];
                curr_ptr[i + 1] = '\0';
            }
        }
        if(curr_ptr[strlen(curr_ptr) - 1] == 32){
            curr_ptr[strlen(curr_ptr) - 1] = '\0';
        }
       	commands[size] = curr_ptr;
	    curr_ptr = strtok(NULL, "|");
	    size++;
    }
    commands[size] = NULL;
    bg = check_bg(commands);

    // Handle Backgrounding
    int pid;
    int fd[2];
    if(bg == 1){
        input[strlen(input) - 1] = '\0';
        pipe(fd);
        num_items++;
        processes[num_items - 1].num = num_items;
        processes[num_items - 1].name[0] = '\0';
        strcat(processes[num_items - 1].name, input);
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
            close(fd[1]);
        }
    }

    if(bg == 1 && pid > 0){
        close(fd[1]);
        display_start();
        read(fd[0], &pid, sizeof(int));
        processes[num_items - 1].pid = pid;
        return 0;
    }


    int i;
    int pipes[process_num][2];
    int pids[process_num];
    for(i = 0; i < process_num; i++){
        if(pipe(pipes[i]) == -1){
            display_error("ERROR: ", "Error creating pipes.");
            return -1;
        }   
    }

    // Creating the processes.
    for(i = 0; i < process_num; i++){
        // Delete the command at element 0
        if(i != 0){
            for(int k = 0; k < size - 1; k++){
                commands[k] = commands[k + 1];
                commands[k + 1] = NULL;
            }
        }

        pids[i] = fork();
        if(pids[i] == -1){
            display_error("ERROR: ", "Error creating processes.");
            exit(EXIT_FAILURE);
        }
        // Handling child processes.
        if(pids[i] == 0){
            // Closing all unused pipe ends for a child process.
            for(int j = 0; j < process_num; j++){
                if(i != j){
                    close(pipes[j][0]);
                }
                if(i + 1 != j){
                    close(pipes[j][1]);
                }
            }
            int old_stdin = dup(STDIN_FILENO);
            int old_stdout = dup(STDOUT_FILENO);

            if(i != process_num - 1){
                dup2(pipes[i + 1][1], STDOUT_FILENO);
                close(pipes[i + 1][1]);
            }
            if(i != 0){
                dup2(pipes[i][0], STDIN_FILENO);
            }
            close(pipes[i][0]);

            char* child_tokens[MAX_STR_LEN];
            tokenize_input(commands[0], child_tokens);
            handle_variable(child_tokens);

            dup2(old_stdin, STDIN_FILENO);
            dup2(old_stdout, STDOUT_FILENO);
            close(pipes[i][0]);
            if(i != process_num - 1){
                close(pipes[i + 1][1]);
            }else{
                close(pipes[i][1]);
            }
            exit(EXIT_SUCCESS);
        }
    }
    for(int j = 0; j < process_num; j++){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
    if(!bg){
        for(i = 0; i < process_num; i++){
            waitpid(pids[i], NULL, WCONTINUED);
        }
    }
    if(pid == 0 && bg == 1){
        exit(EXIT_SUCCESS);
    }
    return 0;
}