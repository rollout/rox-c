// linked list implementation of stack

#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

struct ROX_INTERNAL Stack {
    void *data;
    Stack *next;
};

Stack *ROX_INTERNAL stack_create(void *value) {
    Stack *stackNode = (Stack *) malloc(sizeof(Stack));
    stackNode->data = value;
    stackNode->next = NULL;
    return stackNode;
}

void ROX_INTERNAL stack_free(Stack *stack) {
    Stack *next = stack->next;
    free(stack->data);
    free(stack);
    if (next) {
        stack_free(next);
    }
}

int ROX_INTERNAL stack_is_empty(Stack *root) {
    return !root;
}

void ROX_INTERNAL stack_push(Stack **root, void *item) {
    Stack *stackNode = stack_create(item);
    stackNode->next = *root;
    *root = stackNode;
}

void *ROX_INTERNAL stack_pop(struct Stack **root) {
    if (stack_is_empty(*root))
        return NULL;
    Stack *temp = *root;
    *root = (*root)->next;
    void *popped = temp->data;
    free(temp);

    return popped;
}

void *ROX_INTERNAL stack_peek(Stack *root) {
    if (stack_is_empty(root))
        return NULL;
    return root->data;
}
