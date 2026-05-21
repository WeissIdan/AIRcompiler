#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"

int main_found = 0;
/* Helper for Rule 16: Math Operators (+, -, *, /) */
char* check_math_op(char* op, char* left_type, char* right_type) {
    /* Check if both are valid numbers */
    int left_valid = (strcmp(left_type, "int") == 0 || strcmp(left_type, "real") == 0);
    int right_valid = (strcmp(right_type, "int") == 0 || strcmp(right_type, "real") == 0);

    if (!left_valid || !right_valid) {
        printf("Semantic Error: Invalid types for operator '%s'. Expected 'int' or 'real', but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); /* HARD STOP */
    }

    /* If both are int, the result is int. Otherwise, the result gets upgraded to real. */
    if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
        return "int";
    }
    return "real";
}
/* Helper for Rule 16: Relational Operators (<, >, <=, >=) */
char* check_relational_op(char* op, char* left_type, char* right_type) {
    /* Both must be numbers (int or real) */
    int left_valid = (strcmp(left_type, "int") == 0 || strcmp(left_type, "real") == 0);
    int right_valid = (strcmp(right_type, "int") == 0 || strcmp(right_type, "real") == 0);

    if (!left_valid || !right_valid) {
        printf("Semantic Error: Operator '%s' expects 'int' or 'real', but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); /* HARD STOP */
    }

    /* The result of a comparison is always a boolean */
    return "bool";
}

/* Helper for Rule 16: Equality Operators (==, !=) */
char* check_equality_op(char* op, char* left_type, char* right_type) {
    /* Rule 1: The types must match exactly */
    if (strcmp(left_type, right_type) != 0) {
        printf("Semantic Error: Operator '%s' requires matching types, but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); /* HARD STOP */
    }
    
    /* Rule 2: They must be valid comparable types (int, real, bool, char, or pointers) */
    /* (strstr checks if there is a '*' in the type name to allow pointers) */
    if (strcmp(left_type, "int") != 0 && strcmp(left_type, "real") != 0 &&
        strcmp(left_type, "bool") != 0 && strcmp(left_type, "char") != 0 &&
        strstr(left_type, "*") == NULL) {
        printf("Semantic Error: Type '%s' cannot be compared using '%s'!\n", left_type, op);
        exit(1); /* HARD STOP */
    }
    
    /* The result of checking equality is always a boolean */
    return "bool";
}

/* Helper for Rule 16: Logical Operators (&&, ||) */
char* check_logic_op(char* op, char* left_type, char* right_type) {
    if (strcmp(left_type, "bool") != 0 || strcmp(right_type, "bool") != 0) {
        printf("Semantic Error: Operator '%s' expects 'bool' on both sides, but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); /* HARD STOP */
    }
    return "bool";
}

/* THE TYPE EVALUATOR: Recursively figures out the type of any expression */
char* get_expr_type(node* expr) {
    if (expr == NULL) return "void";
    
    char f = expr->token[0];
    
    /* --- 1. BASE CASES (Leaf Nodes) --- */
    /* Is it a literal number? (e.g., "5" or "3.14") */
    if (f >= '0' && f <= '9') {
        if (strchr(expr->token, '.') != NULL) return "real";
        return "int";
    }
    /* Is it a char or string literal? */
    if (f == '\'') return "char";
    if (f == '"') return "string";
    
    /* Is it a boolean or null keyword? */
    if (strcmp(expr->token, "true") == 0 || strcmp(expr->token, "false") == 0) return "bool";
    if (strcmp(expr->token, "null") == 0) return "null";
    
    /* Is it a function call? Look up its return type! */
    if (strcmp(expr->token, "call") == 0) {
        Symbol* sym = lookup_symbol(expr->left->token);
        if (sym != NULL) return sym->type;
        return "error"; /* Rule 5 handles missing functions */
    }
    
    /* Is it a variable? Look up its saved type! */
    if (expr->left == NULL && expr->right == NULL) {
        if ((f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z')) {
            Symbol* sym = lookup_symbol(expr->token);
            if (sym != NULL) return sym->type;
            return "error"; /* Rule 6 handles missing variables */
        }
    }

    /* --- 2. RECURSIVE CASES (Operators) --- */
    
    /* Math Operators */
    if (strcmp(expr->token, "+") == 0 || strcmp(expr->token, "-") == 0 ||
        strcmp(expr->token, "*") == 0 || strcmp(expr->token, "/") == 0) {
        
        /* Recursively get the type of the left and right sides */
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        
        /* Delegate the logic to our clean helper function */
        return check_math_op(expr->token, left_type, right_type);
    }
    
    /* Unary Minus (e.g., -5 or -x) */
    if (strcmp(expr->token, "UMINUS") == 0) {
        char* child_type = get_expr_type(expr->left);
        if (strcmp(child_type, "int") != 0 && strcmp(child_type, "real") != 0) {
            printf("Semantic Error: UMINUS (-) expects 'int' or 'real', but got '%s'!\n", child_type);
            exit(1);
        }
        return child_type;
    }
    
    /* Relational Operators */
    if (strcmp(expr->token, "<") == 0 || strcmp(expr->token, ">") == 0 ||
        strcmp(expr->token, "<=") == 0 || strcmp(expr->token, ">=") == 0) {
        
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        return check_relational_op(expr->token, left_type, right_type);
    }

    /* Equality Operators */
    if (strcmp(expr->token, "==") == 0 || strcmp(expr->token, "!=") == 0) {
        
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        return check_equality_op(expr->token, left_type, right_type);
    }
    /* ... inside get_expr_type, below the Equality check ... */

    /* Logical Operators (&&, ||) */
    if (strcmp(expr->token, "&&") == 0 || strcmp(expr->token, "||") == 0) {
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        return check_logic_op(expr->token, left_type, right_type);
    }
    
    /* Logical NOT (!) */
    /* Note: In your Yacc file, mknode("!", $2, NULL) puts the expression on the LEFT */
    if (strcmp(expr->token, "!") == 0) {
        char* child_type = get_expr_type(expr->left); 
        if (strcmp(child_type, "bool") != 0) {
            printf("Semantic Error: Operator '!' expects a 'bool', but got '%s'!\n", child_type);
            exit(1); /* HARD STOP */
        }
        return "bool";
    }
    return "error"; 
}

/* Helper function to extract all variables from an "arg" chain */
void process_var_list(node* var_list, char* type) {
    if (var_list == NULL) return;

    if (strcmp(var_list->token, "arg") == 0) {
        char* var_name = var_list->left->token;
        
        int success = insert_symbol(var_name, type, "var", 0,NULL);
        if (!success) {
            printf("Semantic Error: Variable '%s' is already declared in this scope!\n", var_name);
            exit(1);
        }
        process_var_list(var_list->right, type);
    }
}

/* Counts formal parameters in a func/proc definition */
int count_formal_args(node* arg_list) {
    if (arg_list == NULL) return 0;
    int count = 0;
    if (strcmp(arg_list->token, "arg_list") == 0) {
        node* args_wrapper = arg_list->left;
        if (args_wrapper != NULL) {
            node* curr_arg = args_wrapper->left;
            while (curr_arg != NULL && strcmp(curr_arg->token, "arg") == 0) {
                count++;
                curr_arg = curr_arg->right;
            }
        }
        count += count_formal_args(arg_list->right);
    }
    return count;
}

/* Counts actual arguments passed in a function call */
int count_actual_args(node* args) {
    if (args == NULL) return 0;
    int count = 0;
    node* curr = args;
    while (curr != NULL && (strcmp(curr->token, "arg") == 0 || strcmp(curr->token, "literal") == 0)) {
        count++;
        curr = curr->right;
    }
    return count;
}

/* Helper function to extract function/proc arguments and add them to the LOCAL scope */
void process_arg_list(node* arg_list) {
    if (arg_list == NULL) return;

    if (strcmp(arg_list->token, "arg_list") == 0) {
        node* args_wrapper = arg_list->left;
        node* next_arg_list = arg_list->right;

        if (args_wrapper != NULL) {
            node* variables = args_wrapper->left;
            char* type_name = args_wrapper->right->token;

            node* curr_arg = variables;
            while (curr_arg != NULL && strcmp(curr_arg->token, "arg") == 0) {
                char* arg_name = curr_arg->left->token;
                
                int success = insert_symbol(arg_name, type_name, "arg", 0, NULL);
                if (!success) {
                    printf("Semantic Error: Argument/Variable '%s' is already declared in this scope!\n", arg_name);
                    exit(1); 
                }
                curr_arg = curr_arg->right;
            }
        }
        process_arg_list(next_arg_list);
    }
}

/* Helper to build the param_types array for a function definition */
void fill_param_types(node* arg_list, char** types_array, int* index) {
    if (arg_list == NULL) return;
    if (strcmp(arg_list->token, "arg_list") == 0) {
        node* args_wrapper = arg_list->left;
        if (args_wrapper != NULL) {
            node* curr_arg = args_wrapper->left;
            char* type_name = args_wrapper->right->token;
            while (curr_arg != NULL && strcmp(curr_arg->token, "arg") == 0) {
                types_array[*index] = strdup(type_name);
                (*index)++;
                curr_arg = curr_arg->right;
            }
        }
        fill_param_types(arg_list->right, types_array, index);
    }
}

/* The Main Tree Walker */
void build_table(node* tree) {
    if (tree == NULL) return;

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
            char* name = tree->left->right->token;
            char* kind = tree->token; 
            node* arg_list = tree->right->left;
            node* body = tree->right->right;
            
            char* return_type = "void";
            if (strcmp(kind, "func") == 0) {
                return_type = tree->left->left->token;
            }

            if (strcmp(name, "Main") == 0) {
                if (strcmp(kind, "func") == 0) {
                    printf("Semantic Error: 'Main' must be a procedure (proc), not a function (func)!\n");
                    exit(1);
                } else {
                    main_found++;
                    if (main_found > 1) {
                        printf("Semantic Error: Procedure 'Main' is defined more than once!\n");
                        exit(1);
                    }
                    if (arg_list != NULL) {
                        printf("Semantic Error: Procedure 'Main' cannot receive arguments!\n");
                        exit(1);
                    }
                }
            }

            int expected_args = count_formal_args(arg_list);
            
            /* --- NEW: Build the parameter types array --- */
            char** param_types_array = NULL;
            if (expected_args > 0) {
                param_types_array = (char**)malloc(expected_args * sizeof(char*));
                int idx = 0;
                fill_param_types(arg_list, param_types_array, &idx);
            }

            /* Insert the func/proc WITH the new array */
            int success = insert_symbol(name, return_type, kind, expected_args, param_types_array);
            if (!success) {
                printf("Semantic Error: '%s' is already declared in this scope!\n", name);
                exit(1);
            }

            push_scope(name); 
            process_arg_list(arg_list);
            build_table(body); 
            pop_scope();  
            return;
        }
        
        /* --- 3. HANDLE CONTROL FLOW BLOCKS --- */
        else if (strcmp(tree->token, "if_stmt") == 0 || 
                 strcmp(tree->token, "while_stmt") == 0 || 
                 strcmp(tree->token, "for") == 0 ||
                 strcmp(tree->token, "else") == 0) {
            
            push_scope(tree->token);
            build_table(tree->left);
            build_table(tree->right);
            pop_scope();
            return; 
        }
        
        /* --- 4. HANDLE CALLS (Rules 5 & 7) --- */
        else if (strcmp(tree->token, "call") == 0) {
            char* call_name = tree->left->token;
            Symbol* sym = lookup_symbol(call_name);
            
            if (sym == NULL) {
                printf("Semantic Error: Function or Procedure '%s' is not defined before it is called!\n", call_name);
                exit(1); 
            }
            
            if (strcmp(sym->kind, "func") != 0 && strcmp(sym->kind, "proc") != 0) {
                printf("Semantic Error: '%s' is a %s, not a function or procedure!\n", call_name, sym->kind);
                exit(1); 
            }
            
            int provided_args = count_actual_args(tree->right);
            if (provided_args != sym->num_params) {
                printf("Semantic Error: Function/Procedure '%s' expects %d arguments, but %d were provided!\n", 
                       call_name, sym->num_params, provided_args);
                exit(1);
            }
            /* --- RULE 8 CHECK: Verify Argument Types --- */
            node* curr_actual_arg = tree->right; /* The args passed in */
            int arg_index = 0;
            
            while (curr_actual_arg != NULL && 
                  (strcmp(curr_actual_arg->token, "arg") == 0 || strcmp(curr_actual_arg->token, "literal") == 0)) {
                
                /* ACTIVATE THE TYPE ENGINE: What type is the user passing in? */
                char* actual_type = get_expr_type(curr_actual_arg->left);
                char* expected_type = sym->param_types[arg_index];
                
                /* If they don't match exactly, throw an error! */
                if (strcmp(actual_type, expected_type) != 0) {
                    printf("Semantic Error: Argument #%d of '%s' expects type '%s', but got '%s'!\n", 
                           arg_index + 1, call_name, expected_type, actual_type);
                    exit(1); /* HARD STOP */
                }
                
                curr_actual_arg = curr_actual_arg->right;
                arg_index++;
            }
            
            build_table(tree->right); 
            return; 
        }
        
        /* --- 5. HANDLE VARIABLE USAGE (Rule 6) --- */
        else if (tree->left == NULL && tree->right == NULL) {
            char f = tree->token[0];
            if ((f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z')) {
                if (strcmp(tree->token, "true") != 0 &&
                    strcmp(tree->token, "false") != 0 &&
                    strcmp(tree->token, "null") != 0 &&
                    strcmp(tree->token, "NONE") != 0) {
                    
                    Symbol* sym = lookup_symbol(tree->token);
                    if (sym == NULL) {
                        printf("Semantic Error: Variable '%s' is used before it is defined!\n", tree->token);
                        exit(1); 
                    }
                }
            }
            return; 
        }
    } 

    build_table(tree->left);
    build_table(tree->right);
}

void check_main_exists() {
    if (main_found == 0) {
        printf("Semantic Error: Procedure 'Main' is missing!\n");
        exit(1);
    }
}