#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "variables.h"
#include "io_helpers.h"

void init_list() {
	first = NULL;
}

int name_exists(char *name){
	Node *curr = first;
	while(curr != NULL){
		if(strcmp(curr->names, name) == 0){
			return 0;
		}else{
			curr = curr->next;
		}
	}
	return 1;
}

void reassign_value(char *name, char *value){
	Node *curr = first;
	while(curr != NULL){
		if(strcmp(curr->names, name) == 0){
			strcpy(curr->data, value);
			break;
		}else{
			curr = curr->next;
		}
	}
}

void set_variable(char *name, char *value){
    Node *var = malloc(sizeof(Node));
	var->names = malloc(sizeof(char[64]));
	strcpy(var->names, name);
	var->data = malloc(sizeof(char[64]));
	strcpy(var->data, value);
	var->next = NULL;
	if(first == NULL){
		first = var;
	}else if(name_exists(name) == 0){
		reassign_value(name, value);
		free(var->names);
		free(var->data);
		free(var);
	}else{
		Node *curr = first;
		while(curr->next != NULL){
			curr = curr->next;
		}
		curr->next = var;
	}
}