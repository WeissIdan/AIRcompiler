#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Global counters for temporary variables and labels */
int temp_counter = 0;
int label_counter = 1;
char* generate_3ac(node* tree);
/* Helper: Generate a new temp variable (t0, t1, t2...) */
char* new_temp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", temp_counter++);
    return temp;
}

/* Helper: Generate a new label (L1, L2, L3...) */
char* new_label() {
    char* label = (char*)malloc(10);
    sprintf(label, "L%d", label_counter++);
    return label;
}
int push_args(node* arg_tree) {
    if (arg_tree == NULL) return 0;
    
    /* 1. Go down the right side of the tree FIRST (Right-to-Left evaluation) */
    int count = push_args(arg_tree->right);
    
    /* 2. Evaluate the actual expression on the left */
    char* arg_val = generate_3ac(arg_tree->left);
    
    /* 3. Push it to the stack */
    printf("PushParam %s\n", arg_val);
    
    /* Return the total number of arguments processed */
    return count + 1;
}

/* --- THE SHORT-CIRCUIT BOOLEAN GENERATOR --- */
void generate_bool_3ac(node* expr, char* true_label, char* false_label) {
    if (expr == NULL) return;

    /* Logical OR (||) */
    if (strcmp(expr->token, "||") == 0) {
        char* next_label = new_label();
        generate_bool_3ac(expr->left, true_label, next_label);
        printf("%s:\n", next_label);
        generate_bool_3ac(expr->right, true_label, false_label);
        return;
    }

    /* Logical AND (&&) */
    if (strcmp(expr->token, "&&") == 0) {
        char* next_label = new_label();
        generate_bool_3ac(expr->left, next_label, false_label);
        printf("%s:\n", next_label);
        generate_bool_3ac(expr->right, true_label, false_label);
        return;
    }

    /* Logical NOT (!) */
    if (strcmp(expr->token, "!") == 0) {
        generate_bool_3ac(expr->left, false_label, true_label);
        return;
    }

    /* Relational Operators */
    if (strcmp(expr->token, "==") == 0 || strcmp(expr->token, "!=") == 0 ||
        strcmp(expr->token, "<") == 0  || strcmp(expr->token, ">") == 0 ||
        strcmp(expr->token, "<=") == 0 || strcmp(expr->token, ">=") == 0) {
        
        char* left_val = generate_3ac(expr->left);
        char* right_val = generate_3ac(expr->right);
        
        printf("if %s %s %s Goto %s\n", left_val, expr->token, right_val, true_label);
        printf("Goto %s\n", false_label);
        return;
    }

    /* Boolean Literals */
    if (strcmp(expr->token, "true") == 0) {
        printf("Goto %s\n", true_label);
        return;
    }
    if (strcmp(expr->token, "false") == 0) {
        printf("Goto %s\n", false_label);
        return;
    }

    /* Fallback: A single boolean variable (e.g., 'if(flag)') */
    char* val = generate_3ac(expr);
    printf("if %s == true Goto %s\n", val, true_label);
    printf("Goto %s\n", false_label);
}

/* The Recursive 3AC Generator */
char* generate_3ac(node* tree) {
    if (tree == NULL) return NULL;

    /* --- 1. BASE CASES (Leaves) --- */
    /* If it's a raw number, variable name, or string, just return its name! */
    if (tree->left == NULL && tree->right == NULL) {
        return tree->token; 
    }

    /* --- 2. UNARY MINUS (Rule: tX = uminus Y) --- */
    if (strcmp(tree->token, "UMINUS") == 0) {
        char* child_val = generate_3ac(tree->left);
        char* result_temp = new_temp();
        printf("%s = - %s\n", result_temp, child_val);
        return result_temp;
    }

    /* --- 3. MATH OPERATORS (+, -, *, /) --- */
    if (strcmp(tree->token, "+") == 0 || strcmp(tree->token, "-") == 0 ||
        strcmp(tree->token, "*") == 0 || strcmp(tree->token, "/") == 0) {
        
        /* Evaluate left and right children first */
        char* left_val = generate_3ac(tree->left);
        char* right_val = generate_3ac(tree->right);
        
        /* Create a temporary variable to hold the result */
        char* result_temp = new_temp();
        
        /* Print the flat 3AC instruction */
        printf("%s = %s %s %s\n", result_temp, left_val, tree->token, right_val);
        
        /* Return the temp variable so the parent node can use it! */
        return result_temp;
    }

    /* --- 4. ASSIGNMENTS (=) --- */
    if (strcmp(tree->token, "assign_stmt") == 0) {
        /* Evaluate the right side equation first */
        char* right_val = generate_3ac(tree->right);
        
        /* Check if the left side is a Dereference (^y = ...) */
        if (strcmp(tree->left->token, "^") == 0) {
            char* ptr_val = generate_3ac(tree->left->left);
            printf("^%s = %s\n", ptr_val, right_val);
        } 
        /* Check if the left side is an Array Access (s1[i] = ...) */
        else if (strcmp(tree->left->token, "array_access") == 0) {
            char* arr_name = tree->left->left->token;
            char* idx_val = generate_3ac(tree->left->right);
            printf("%s[%s] = %s\n", arr_name, idx_val, right_val);
        } 
        /* Standard variable assignment (x = ...) */
        else {
            char* left_var = generate_3ac(tree->left);
            printf("%s = %s\n", left_var, right_val);
        }
        return NULL;
    }
    /* --- RULE: ADDRESS OF (&) --- */
    if (strcmp(tree->token, "&") == 0) {
        char* child_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = & %s\n", temp, child_val);
        return temp;
    }

    /* --- RULE: DEREFERENCE (^) --- */
    if (strcmp(tree->token, "^") == 0) {
        char* ptr_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = ^%s\n", temp, ptr_val);
        return temp;
    }

    /* --- RULE: ARRAY ACCESS (s1[i]) --- */
    if (strcmp(tree->token, "array_access") == 0) {
        char* arr_name = tree->left->token;
        char* idx_val = generate_3ac(tree->right);
        char* temp = new_temp();
        printf("%s = %s[%s]\n", temp, arr_name, idx_val);
        return temp;
    }

    /* --- RULE: STRING LENGTH (|length|) --- */
    if (strcmp(tree->token, "|length|") == 0) {
        char* str_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = length %s\n", temp, str_val);
        return temp;
    }
    /* --- 5. FUNCTION & PROC DEFINITIONS --- */
    if (strcmp(tree->token, "func") == 0 || strcmp(tree->token, "proc") == 0) {
        
        /* In your AST, the function name is hidden at left->right->token */
        char* func_name = tree->left->right->token;
        
        /* Print the Label and BeginFunc */
        /* (We will use 44 bytes as a placeholder frame size for now) */
        printf("%s:\n", func_name);
        printf("BeginFunc 44\n");
        
        /* Generate the 3AC for everything inside the function body */
        generate_3ac(tree->right->right);
        
        printf("EndFunc\n\n");
        return NULL;
    }

    /* --- 6. RETURN STATEMENTS --- */
    if (strcmp(tree->token, "return") == 0) {
        /* Check if it's an empty return */
        if (strcmp(tree->left->token, "NONE") == 0) {
            printf("Return\n");
        } else {
            /* Evaluate the math/variable, then return it! */
            char* ret_val = generate_3ac(tree->left);
            printf("Return %s\n", ret_val);
        }
        return NULL;
    }

    /* --- 7. FUNCTION CALLS --- */
    if (strcmp(tree->token, "call") == 0) {
        char* func_name = tree->left->token;
        
        /* Evaluate and push all arguments using our helper! */
        int num_args = push_args(tree->right);
        
        /* Make the call and store the result in a temp variable */
        char* result_temp = new_temp();
        printf("%s = LCall %s\n", result_temp, func_name);
        
        /* Clean up the stack (4 bytes per argument) */
        if (num_args > 0) {
            printf("PopParams %d\n", num_args * 4);
        }
        
        return result_temp;
    }
    /* --- 8. IF STATEMENTS --- */
    if (strcmp(tree->token, "if_stmt") == 0) {
        char* true_label = new_label();
        char* false_label = new_label();
        
        /* Check if it has an ELSE block */
        if (tree->right != NULL && tree->right->token != NULL && strcmp(tree->right->token, "else") == 0) {
            char* end_label = new_label();
            
            generate_bool_3ac(tree->left->left, true_label, false_label);
            
            printf("%s:\n", true_label);
            generate_3ac(tree->left->right); /* True body */
            printf("Goto %s\n", end_label);
            
            printf("%s:\n", false_label);
            generate_3ac(tree->right->left); /* Else body */
            
            printf("%s:\n", end_label);
        } else {
            /* Plain IF statement */
            generate_bool_3ac(tree->left, true_label, false_label);
            
            printf("%s:\n", true_label);
            generate_3ac(tree->right->left); /* True body */
            
            printf("%s:\n", false_label);
        }
        return NULL;
    }

    /* --- 9. WHILE LOOPS --- */
    if (strcmp(tree->token, "while_stmt") == 0) {
        char* start_label = new_label();
        char* true_label = new_label();
        char* false_label = new_label();
        
        printf("%s:\n", start_label);
        generate_bool_3ac(tree->left, true_label, false_label);
        
        printf("%s:\n", true_label);
        generate_3ac(tree->right); /* Loop body */
        printf("Goto %s\n", start_label);
        
        printf("%s:\n", false_label);
        return NULL;
    }

    /* --- 10. FOR LOOPS --- */
    if (strcmp(tree->token, "for") == 0) {
        generate_3ac(tree->left); /* inits */
        
        char* start_label = new_label();
        char* true_label = new_label();
        char* false_label = new_label();
        
        printf("%s:\n", start_label);
        generate_bool_3ac(tree->right->left, true_label, false_label); /* condition */
        
        printf("%s:\n", true_label);
        generate_3ac(tree->right->right->right); /* body */
        generate_3ac(tree->right->right->left);  /* updates */
        printf("Goto %s\n", start_label);
        
        printf("%s:\n", false_label);
        return NULL;
    }

    /* --- 11. INLINE BOOLEANS (Fixes the "x = (null)" bug!) --- */
    if (strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0 ||
        strcmp(tree->token, "<") == 0  || strcmp(tree->token, ">") == 0 ||
        strcmp(tree->token, "<=") == 0 || strcmp(tree->token, ">=") == 0 ||
        strcmp(tree->token, "&&") == 0 || strcmp(tree->token, "||") == 0 ||
        strcmp(tree->token, "!") == 0) {
        
        /* According to the slides, boolean assignments evaluate into a 1 or 0 temp variable */
        char* temp = new_temp();
        char* true_label = new_label();
        char* false_label = new_label();
        char* end_label = new_label();
        
        generate_bool_3ac(tree, true_label, false_label);
        
        printf("%s:\n", true_label);
        printf("%s = 1\n", temp);
        printf("Goto %s\n", end_label);
        
        printf("%s:\n", false_label);
        printf("%s = 0\n", temp);
        
        printf("%s:\n", end_label);
        return temp;
    }

    /* --- 5. TREE TRAVERSAL --- */
    /* For all other structural nodes (stmts, block, func, CODE), just keep walking */
    generate_3ac(tree->left);
    generate_3ac(tree->right);
    
    return NULL;
}