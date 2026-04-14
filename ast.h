#ifndef AST_H
#define AST_H

/* The AST Node Structure */
typedef struct node {
    char *token;
    struct node *left;
    struct node *right;
} node;

/* Function Prototypes */
node *mknode(char *token, node *left, node *right);
void printtree(node *tree);

#endif