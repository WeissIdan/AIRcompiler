#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"
int main_found = 0;
/* Helper function to extract all variables from an "arg" chain */
void process_var_list(node* var_list, char* type) {
    if (var_list == NULL) return;

    if (strcmp(var_list->token, "arg") == 0) {
        /* Left child is the variable name (e.g., 'x') */
        char* var_name = var_list->left->token;
        
        /* Attempt to insert it into the Symbol Table */
        int success = insert_symbol(var_name, type, "var");
        
        /* Check Rule #4: No duplicate variables in the same scope! */
        if (!success) {
            printf("Semantic Error: Variable '%s' is already declared in this scope!\n", var_name);
            exit(1);
        }

        /* Right child continues the list (e.g., the 'y' in 'x, y : int') */
        process_var_list(var_list->right, type);
    }
}

/* The Main Tree Walker */
/* The Main Tree Walker */
void build_table(node* tree) {
    if (tree == NULL) return;

    /* Check if it's an invisible glue node (empty string) */
    int is_glue = (tree->token != NULL && strcmp(tree->token, "") == 0);

    if (!is_glue) {
        /* --- 1. HANDLE VARIABLE DEFINITIONS --- */

        if (strcmp(tree->token, "var_def") == 0) {
            char* type_name = tree->right->token;
            process_var_list(tree->left, type_name);
            return; 
        }
        
        /* --- 2. HANDLE FUNCTIONS & PROCS --- */
        else if (strcmp(tree->token, "func") == 0 || strcmp(tree->token, "proc") == 0) {
            /* Extract data from the AST */
            char* name = tree->left->right->token;
            char* kind = tree->token; /* will be "func" or "proc" */
            node* arg_list = tree->right->left;
            node* body = tree->right->right;
            
            /* Extract return type (only functions have a return type, procs are "void") */
            char* return_type = "void";
            if (strcmp(kind, "func") == 0) {
                return_type = tree->left->left->token;
            }

            /* RULE #1 & #2: Check Main */
            if (strcmp(name, "Main") == 0) {
                /* NEW CHECK: Main cannot be a function! */
                if (strcmp(kind, "func") == 0) {
                    printf("Semantic Error: 'Main' must be a procedure (proc), not a function (func)!\n");
                    exit(1);
                } else {
                    main_found++;
                    
                    /* Check if there is more than 1 Main (Rule 1) */
                    if (main_found > 1) {
                        printf("Semantic Error: Procedure 'Main' is defined more than once!\n");
                        exit(1);
                    }
                    
                    /* Check if Main has arguments (Rule 2) */
                    if (arg_list != NULL) {
                        printf("Semantic Error: Procedure 'Main' cannot receive arguments!\n");
                        exit(1);
                    }
                }
            }

            /* Insert the func/proc into the current scope (Rule 3) */
            int success = insert_symbol(name, return_type, kind);
            if (!success) {
                printf("Semantic Error: '%s' is already declared in this scope!\n", name);
                exit(1);
            }

            /* Push a new local scope for the function/procedure body */
            push_scope(); 
            
            /* TODO next: process arg_list and insert arguments into this new scope */
            
            build_table(body); /* Walk the body of the function/procedure */
            
            pop_scope();  /* Destroy local scope when it ends */
            return;
        }
        
        /* --- 3. HANDLE CONTROL FLOW BLOCKS (Replaces "BLOCK") --- */
        else if (strcmp(tree->token, "if_stmt") == 0 || 
                 strcmp(tree->token, "while_stmt") == 0 || 
                 strcmp(tree->token, "for") == 0 ||
                 strcmp(tree->token, "else") == 0) {
            
            /* Every if/while/for loop creates its own local scope! */
            push_scope();
            build_table(tree->left);
            build_table(tree->right);
            pop_scope();
            return; /* We return here so we don't accidentally recurse down the tree twice */
        }
        else if (strcmp(tree->token, "call") == 0) {
            char* call_name = tree->left->token;
            
            /* Check the symbol table to see if we know this name */
            Symbol* sym = lookup_symbol(call_name);
            
            /* RULE #5: Is it completely missing? */
            if (sym == NULL) {
                printf("Semantic Error: Function or Procedure '%s' is not defined before it is called!\n", call_name);
                exit(1); /* HARD STOP */
            }
            
            /* EXTRA SAFETY: What if they tried to call a normal variable like a function? (e.g., x() ) */
            if (strcmp(sym->kind, "func") != 0 && strcmp(sym->kind, "proc") != 0) {
                printf("Semantic Error: '%s' is a %s, not a function or procedure!\n", call_name, sym->kind);
                exit(1); /* HARD STOP */
            }
            
            /* Recurse down the right side so we can scan the arguments passed into the call */
            build_table(tree->right); 
            
            return; /* Return here so we don't accidentally recurse down the left side (the ID node) */
        }
        /* --- 5. HANDLE VARIABLE USAGE (Rule #6) --- */
        /* If we hit a leaf node (no children), it is inside an expression/statement! */
        else if (tree->left == NULL && tree->right == NULL) {
            char f = tree->token[0];
            
            /* Variables in our language always start with a letter */
            if ((f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z')) {
                
                /* Ignore literal keywords that also start with letters */
                if (strcmp(tree->token, "true") != 0 &&
                    strcmp(tree->token, "false") != 0 &&
                    strcmp(tree->token, "null") != 0 &&
                    strcmp(tree->token, "NONE") != 0) {
                    
                    /* It's a variable! Check if it exists in the symbol table */
                    Symbol* sym = lookup_symbol(tree->token);
                    
                    /* RULE #6: If it's NULL, it was never declared! */
                    if (sym == NULL) {
                        printf("Semantic Error: Variable '%s' is used before it is defined!\n", tree->token);
                        exit(1); /* HARD STOP */
                    }
                }
            }
            return; /* We reached the bottom of this branch */
        }
    }
    

    /* Recurse down the tree for all other normal nodes */
    build_table(tree->left);
    build_table(tree->right);
}

void check_main_exists() {
    if (main_found == 0) {
        printf("Semantic Error: Procedure 'Main' is missing!\n");
        exit(1);
    }
}