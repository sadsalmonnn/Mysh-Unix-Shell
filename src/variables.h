#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <unistd.h>

typedef struct Node
{
    char *key;
    char *value;
    struct Node *next;
} Node;

// Node *insert(Node **head, const char *key, const char *value);
Node *insert(Node **head, char *key, char *value);

char *find(Node *head, const char *key);

extern Node *myshfrontvar;

void free_list(Node *head);

#endif