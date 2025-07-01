#include <string.h>
#include <stdlib.h>
#include "variables.h"
#include "io_helpers.h"

Node *myshfrontvar = NULL; // Define the variable here

// Node *insert(Node **head, const char *key, const char *value)
// {
//     Node *current = *head;
//     while (current != NULL)
//     {
//         if (strcmp(current->key, key) == 0)
//         {
//             free(current->value);
//             current->value = strdup(value);
//             return current;
//         }
//         current = current->next;
//     }

//     Node *newNode = malloc(sizeof(Node));
//     newNode->key = strdup(key);
//     newNode->value = strdup(value);
//     newNode->next = *head;
//     *head = newNode;

//     return newNode;
// }

Node *insert(Node **head, char *key, char *value)
{
    Node *current = *head;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            current->value = value;
            return current;
        }
        current = current->next;
    }
    Node *newNode = malloc(sizeof(Node));
    newNode->key = key;
    newNode->value = value;
    newNode->next = *head;
    *head = newNode;

    return newNode;
}

char *find(Node *head, const char *key)
{
    Node *current = head;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    return "";
}

// void free_list(Node *head)
// {
//     Node *current = head;
//     Node *temp;
//     while (current != NULL)
//     {
//         temp = current;
//         free(current->key);      // Free the key string
//         free(current->value);    // Free the value string
//         current = current->next; // Move to the next node
//         free(temp);              // Free the node itself
//     }
// }

void free_list(Node *head)
{
    Node *current = head;
    Node *temp;

    while (current != NULL)
    {
        // Free the key and value if they were dynamically allocated
        free(current->key);
        free(current->value);

        // Store the next node before freeing the current one
        temp = current->next;

        // Free the current node
        free(current);

        // Move to the next node
        current = temp;
    }
}