#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Global counters for temporary variables and labels */
int temp_counter = 0;
int label_counter = 1;
char* generate_3ac(node* tree);
/*Generate a new temp variable*/
char* new_temp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", temp_counter++);
    return temp;
}

/*Generate a new label*/
char* new_label() {
    char* label = (char*)malloc(10);
    sprintf(label, "L%d", label_counter++);
    return label;
}
int push_args(node* arg_tree) {
    if (arg_tree == NULL) return 0;
    int count = push_args(arg_tree->right);
    char* arg_val = generate_3ac(arg_tree->left);
    printf("PushParam %s\n", arg_val);
    return count + 1;
}

void generate_bool_3ac(node* expr, char* true_label, char* false_label) {
    if (expr == NULL) return;
    if (strcmp(expr->token, "||") == 0) {
        char* next_label = new_label();
        generate_bool_3ac(expr->left, true_label, next_label);
        printf("%s:\n", next_label);
        generate_bool_3ac(expr->right, true_label, false_label);
        return;
    }

    if (strcmp(expr->token, "&&") == 0) {
        char* next_label = new_label();
        generate_bool_3ac(expr->left, next_label, false_label);
        printf("%s:\n", next_label);
        generate_bool_3ac(expr->right, true_label, false_label);
        return;
    }

    if (strcmp(expr->token, "!") == 0) {
        generate_bool_3ac(expr->left, false_label, true_label);
        return;
    }

    if (strcmp(expr->token, "==") == 0 || strcmp(expr->token, "!=") == 0 ||
        strcmp(expr->token, "<") == 0  || strcmp(expr->token, ">") == 0 ||
        strcmp(expr->token, "<=") == 0 || strcmp(expr->token, ">=") == 0) {
        
        char* left_val = generate_3ac(expr->left);
        char* right_val = generate_3ac(expr->right);
        
        printf("if %s %s %s Goto %s\n", left_val, expr->token, right_val, true_label);
        printf("Goto %s\n", false_label);
        return;
    }

    if (strcmp(expr->token, "true") == 0) {
        printf("Goto %s\n", true_label);
        return;
    }
    if (strcmp(expr->token, "false") == 0) {
        printf("Goto %s\n", false_label);
        return;
    }

    char* val = generate_3ac(expr);
    printf("if %s == true Goto %s\n", val, true_label);
    printf("Goto %s\n", false_label);
}

char* generate_3ac(node* tree) {
    if (tree == NULL) return NULL;

    if (tree->left == NULL && tree->right == NULL) {
        return tree->token; 
    }

    if (strcmp(tree->token, "UMINUS") == 0) {
        char* child_val = generate_3ac(tree->left);
        char* result_temp = new_temp();
        printf("%s = - %s\n", result_temp, child_val);
        return result_temp;
    }

    if (strcmp(tree->token, "+") == 0 || strcmp(tree->token, "-") == 0 ||
        strcmp(tree->token, "*") == 0 || strcmp(tree->token, "/") == 0) {
        char* left_val = generate_3ac(tree->left);
        char* right_val = generate_3ac(tree->right);
        char* result_temp = new_temp();
        printf("%s = %s %s %s\n", result_temp, left_val, tree->token, right_val);
        return result_temp;
    }
    if (strcmp(tree->token, "assign_stmt") == 0) {
        char* right_val = generate_3ac(tree->right);
        if (strcmp(tree->left->token, "^") == 0) {
            char* ptr_val = generate_3ac(tree->left->left);
            printf("^%s = %s\n", ptr_val, right_val);
        } 
        else if (strcmp(tree->left->token, "array_access") == 0) {
            char* arr_name = tree->left->left->token;
            char* idx_val = generate_3ac(tree->left->right);
            printf("%s[%s] = %s\n", arr_name, idx_val, right_val);
        } 
        else {
            char* left_var = generate_3ac(tree->left);
            printf("%s = %s\n", left_var, right_val);
        }
        return NULL;
    }
    if (strcmp(tree->token, "&") == 0) {
        char* child_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = & %s\n", temp, child_val);
        return temp;
    }

    if (strcmp(tree->token, "^") == 0) {
        char* ptr_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = ^%s\n", temp, ptr_val);
        return temp;
    }
    if (strcmp(tree->token, "array_access") == 0) {
        char* arr_name = tree->left->token;
        char* idx_val = generate_3ac(tree->right);
        char* temp = new_temp();
        printf("%s = %s[%s]\n", temp, arr_name, idx_val);
        return temp;
    }
    if (strcmp(tree->token, "|length|") == 0) {
        char* str_val = generate_3ac(tree->left);
        char* temp = new_temp();
        printf("%s = length %s\n", temp, str_val);
        return temp;
    }
    if (strcmp(tree->token, "func") == 0 || strcmp(tree->token, "proc") == 0) {
        
        char* func_name = tree->left->right->token;
        printf("%s:\n", func_name);
        printf("BeginFunc 44\n");
        generate_3ac(tree->right->right);
        printf("EndFunc\n\n");
        return NULL;
    }

    if (strcmp(tree->token, "return") == 0) {
        if (strcmp(tree->left->token, "NONE") == 0) {
            printf("Return\n");
        } else {
            char* ret_val = generate_3ac(tree->left);
            printf("Return %s\n", ret_val);
        }
        return NULL;
    }

    if (strcmp(tree->token, "call") == 0) {
        char* func_name = tree->left->token;
        int num_args = push_args(tree->right);
        char* result_temp = new_temp();
        printf("%s = LCall %s\n", result_temp, func_name);
        if (num_args > 0) {
            printf("PopParams %d\n", num_args * 4);
        }
        return result_temp;
    }
    if (strcmp(tree->token, "if_stmt") == 0) {
        char* true_label = new_label();
        char* false_label = new_label();
        
        if (tree->right != NULL && tree->right->token != NULL && strcmp(tree->right->token, "else") == 0) {
            char* end_label = new_label();
            generate_bool_3ac(tree->left->left, true_label, false_label);
            printf("%s:\n", true_label);
            generate_3ac(tree->left->right); 
            printf("Goto %s\n", end_label);
            printf("%s:\n", false_label);
            generate_3ac(tree->right->left); 
            
            printf("%s:\n", end_label);
        } else {
            generate_bool_3ac(tree->left, true_label, false_label);
            printf("%s:\n", true_label);
            generate_3ac(tree->right->left);
            printf("%s:\n", false_label);
        }
        return NULL;
    }
    if (strcmp(tree->token, "while_stmt") == 0) {
        char* start_label = new_label();
        char* true_label = new_label();
        char* false_label = new_label();
        printf("%s:\n", start_label);
        generate_bool_3ac(tree->left, true_label, false_label);
        printf("%s:\n", true_label);
        generate_3ac(tree->right);
        printf("Goto %s\n", start_label);
        printf("%s:\n", false_label);
        return NULL;
    }

    if (strcmp(tree->token, "for") == 0) {
        generate_3ac(tree->left); 
        
        char* start_label = new_label();
        char* true_label = new_label();
        char* false_label = new_label();
        
        printf("%s:\n", start_label);
        generate_bool_3ac(tree->right->left, true_label, false_label); 
        
        printf("%s:\n", true_label);
        generate_3ac(tree->right->right->right); 
        generate_3ac(tree->right->right->left);  
        printf("Goto %s\n", start_label);
        
        printf("%s:\n", false_label);
        return NULL;
    }

    if (strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0 ||
        strcmp(tree->token, "<") == 0  || strcmp(tree->token, ">") == 0 ||
        strcmp(tree->token, "<=") == 0 || strcmp(tree->token, ">=") == 0 ||
        strcmp(tree->token, "&&") == 0 || strcmp(tree->token, "||") == 0 ||
        strcmp(tree->token, "!") == 0) {
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
    generate_3ac(tree->left);
    generate_3ac(tree->right);
    
    return NULL;
}