typedef struct Node {
		char *names;
       	char *data;
    	struct Node *next;
}Node;

struct Node *first;

void init_list();
void set_variable(char *name, char *value);