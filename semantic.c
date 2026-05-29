#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"

int main_found = 0;
/* Helper for Rule 16: Math Operators*/
char* check_math_op(char* op, char* left_type, char* right_type) {

    int left_is_ptr  = (strstr(left_type,  "*") != NULL);
    int right_is_ptr = (strstr(right_type, "*") != NULL);

    if (left_is_ptr || right_is_ptr) {
        /* Rule: only + and - are allowed with pointers */
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0) {
            printf("Semantic Error: Operator '%s' cannot be used with pointer types!\n", op);
            exit(1);
        }
        /* ptr + int is valid */
        if (left_is_ptr && strcmp(right_type, "int") == 0) return left_type;
        printf("Semantic Error: Can only add or subtract 'int' from a pointer, but got '%s' and '%s'!\n",
            left_type, right_type);
        exit(1);
    }

    /* Check if both are valid numbers */
    int left_valid = (strcmp(left_type, "int") == 0 || strcmp(left_type, "real") == 0);
    int right_valid = (strcmp(right_type, "int") == 0 || strcmp(right_type, "real") == 0);

    if (!left_valid || !right_valid) {
        printf("Semantic Error: Invalid types for operator '%s'. Expected 'int' or 'real', but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); 
    }

    /* If both are int, the result is int. Otherwise, the result gets upgraded to real. */
    if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
        return "int";
    }
    return "real";
}
/* Helper for Rule 16: Relational Operators*/
char* check_relational_op(char* op, char* left_type, char* right_type) {
    /* Both must be numbers (int or real) */
    int left_valid = (strcmp(left_type, "int") == 0 || strcmp(left_type, "real") == 0);
    int right_valid = (strcmp(right_type, "int") == 0 || strcmp(right_type, "real") == 0);

    if (!left_valid || !right_valid) {
        printf("Semantic Error: Operator '%s' expects 'int' or 'real', but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); 
    }
    return "bool";
}

/* Helper for Rule 16: Equality Operators */
char* check_equality_op(char* op, char* left_type, char* right_type) {
    /* Rule 1: The types must match exactly */
    if (strcmp(left_type, right_type) != 0) {
        printf("Semantic Error: Operator '%s' requires matching types, but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1); 
    }
    
    /* Rule 2: They must be valid comparable types */
    if (strcmp(left_type, "int") != 0 && strcmp(left_type, "real") != 0 &&
        strcmp(left_type, "bool") != 0 && strcmp(left_type, "char") != 0 &&
        strstr(left_type, "*") == NULL) {
        printf("Semantic Error: Type '%s' cannot be compared using '%s'!\n", left_type, op);
        exit(1); 
    }
    return "bool";
}

/* Helper for Rule 16: Logical Operators (&&, ||) */
char* check_logic_op(char* op, char* left_type, char* right_type) {
    if (strcmp(left_type, "bool") != 0 || strcmp(right_type, "bool") != 0) {
        printf("Semantic Error: Operator '%s' expects 'bool' on both sides, but got '%s' and '%s'!\n",
               op, left_type, right_type);
        exit(1);
    }
    return "bool";
}

char* get_expr_type(node* expr) {
    if (expr == NULL) return "void";
    
    char f = expr->token[0];
    
    if (f >= '0' && f <= '9') {
        if (strchr(expr->token, '.') != NULL) return "real";
        return "int";
    }
    if (f == '\'') return "char";
    if (f == '"') return "string";
    
    if (strcmp(expr->token, "true") == 0 || strcmp(expr->token, "false") == 0) return "bool";
    if (strcmp(expr->token, "null") == 0) return "null";
    
    if (strcmp(expr->token, "call") == 0) {
        Symbol* sym = lookup_symbol(expr->left->token);
        if (sym != NULL) return sym->type;
        return "error"; 
    }
    
    if (expr->left == NULL && expr->right == NULL) {
        if ((f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z')) {
            Symbol* sym = lookup_symbol(expr->token);
            if (sym != NULL) return sym->type;
            return "error"; 
        }
    }

    
    /* Math Operators */
    if (strcmp(expr->token, "+") == 0 || strcmp(expr->token, "-") == 0 ||
        strcmp(expr->token, "*") == 0 || strcmp(expr->token, "/") == 0) {
        
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        
        return check_math_op(expr->token, left_type, right_type);
    }
    
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

    /* Logical Operators (&&, ||) */
    if (strcmp(expr->token, "&&") == 0 || strcmp(expr->token, "||") == 0) {
        char* left_type = get_expr_type(expr->left);
        char* right_type = get_expr_type(expr->right);
        return check_logic_op(expr->token, left_type, right_type);
    }
    if (strcmp(expr->token, "array_access") == 0) {
        Symbol* sym = lookup_symbol(expr->left->token);
        if (sym == NULL) {
            printf("Semantic Error: Array '%s' is used before it is defined!\n", expr->left->token);
            exit(1);
        }
        
        if (strstr(sym->type, "string") == NULL) {
            printf("Semantic Error: Operator [] can only be used on strings! Got type '%s'\n", sym->type);
            exit(1);
        }
        
        char* index_type = get_expr_type(expr->right);
        if (strcmp(index_type, "int") != 0) {
            printf("Semantic Error: Array index must be an 'int', but got '%s'!\n", index_type);
            exit(1);
        }
        
        return "char"; 
    }
    if (strcmp(expr->token, "|length|") == 0) {
        char* child_type = get_expr_type(expr->left);
        if (strstr(child_type, "string") == NULL) {
            printf("Semantic Error: Length operator || can only be used on strings! Got type '%s'\n", child_type);
            exit(1); 
        }
        return "int";
    }
    /* Logical NOT (!) */
    if (strcmp(expr->token, "!") == 0) {
        char* child_type = get_expr_type(expr->left); 
        if (strcmp(child_type, "bool") != 0) {
            printf("Semantic Error: Operator '!' expects a 'bool', but got '%s'!\n", child_type);
            exit(1); 
        }
        return "bool";
    }
    /* --- RULE 18: ADDRESS OF (&) --- */
    if (strcmp(expr->token, "&") == 0) {
        
        /* Rule 18: Must be a variable OR an array access! */
        int is_plain_var = (expr->left->left == NULL && expr->left->right == NULL);
        int is_array = (strcmp(expr->left->token, "array_access") == 0);
        
        if (!is_plain_var && !is_array) {
            printf("Semantic Error: '&' can only be applied to a variable or array cell!\n");
            exit(1); /* HARD STOP */
        }

        char* child_type = get_expr_type(expr->left);
        
        /* Can only be used on standard types, not existing pointers */
        if (strcmp(child_type, "int") != 0 && strcmp(child_type, "real") != 0 &&
            strcmp(child_type, "char") != 0 && strstr(child_type, "string") == NULL) {
            printf("Semantic Error: Operator '&' requires int, real, char, or string. Got '%s'!\n", child_type);
            exit(1); /* HARD STOP */
        }
        
        /* Return the pointer type (e.g., "int" -> "int*") */
        char* ptr_type = (char*)malloc(strlen(child_type) + 2);
        sprintf(ptr_type, "%s*", child_type);
        return ptr_type;
    }


    /* Dereference operator ^ */
    if (strcmp(expr->token, "^") == 0) {
        char* operand_type = get_expr_type(expr->left);

        /* Must be a pointer type (contains '*') */
        if (strstr(operand_type, "*") == NULL) {
            printf("Semantic Error: '^' can only be applied to a pointer, but got '%s'!\n", operand_type);
            exit(1);
        }

        if (strcmp(operand_type, "int*")  == 0) return "int";
        if (strcmp(operand_type, "real*") == 0) return "real";
        if (strcmp(operand_type, "char*") == 0) return "char";
    }
    return "error"; 
}

/* Helper function to extract all variables from an arg chain */
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

/* Helper function to extract function/proc arguments and add them to the local scope */
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
        
        /* veriable def  */
        if (strcmp(tree->token, "var_def") == 0) {
            char* type_name = tree->right->token;
            process_var_list(tree->left, type_name);
            return; 
        }
        
        /* funcs and procs*/
        else if (strcmp(tree->token, "func") == 0 || strcmp(tree->token, "proc") == 0) {
            char* name = tree->left->right->token;
            char* kind = tree->token; 
            node* arg_list = tree->right->left;
            node* body = tree->right->right;
            
            char* return_type = "void";
            if (strcmp(kind, "func") == 0) {
                return_type = tree->left->left->token;
                if (strcmp(return_type, "string") == 0) {
                    printf("Semantic Error: Function '%s' cannot have a 'string' return type!\n", name);
                    exit(1);
                }
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
            
            char** param_types_array = NULL;
            if (expected_args > 0) {
                param_types_array = (char**)malloc(expected_args * sizeof(char*));
                int idx = 0;
                fill_param_types(arg_list, param_types_array, &idx);
            }

            /* Insert the func/proc */
            int success = insert_symbol(name, return_type, kind, expected_args, param_types_array);
            if (!success) {
                printf("Semantic Error: '%s' is already declared in this scope!\n", name);
                exit(1);
            }

            push_scope(name, return_type); 
            process_arg_list(arg_list);
            build_table(body); 
            pop_scope();  
            return;
        }
        
        /* if,while,for,else */
        else if (strcmp(tree->token, "if_stmt") == 0 || 
                 strcmp(tree->token, "while_stmt") == 0 || 
                 strcmp(tree->token, "for") == 0 ||
                 strcmp(tree->token, "else") == 0 ||
                 strcmp(tree->token, "block") == 0){
            
            if (strcmp(tree->token, "if_stmt") == 0 || strcmp(tree->token, "while_stmt") == 0) {
                char* cond_type;
                
                if (tree->left != NULL && tree->left->token != NULL && strcmp(tree->left->token, "") == 0) {
                    cond_type = get_expr_type(tree->left->left); 
                } else {
                    cond_type = get_expr_type(tree->left); 
                }
                
                if (strcmp(cond_type, "bool") != 0) {
                    printf("Semantic Error: '%s' condition must evaluate to 'bool', but got '%s'!\n", tree->token, cond_type);
                    exit(1); 
                }
            } else if (strcmp(tree->token, "for") == 0) {
                char* cond_type = get_expr_type(tree->right->left); 
                if (strcmp(cond_type, "bool") != 0) {
                    printf("Semantic Error: 'for' loop condition must evaluate to 'bool', but got '%s'!\n", cond_type);
                    exit(1); 
                }
            }

            push_scope(tree->token, current_scope->return_type);
            build_table(tree->left);
            build_table(tree->right);
            pop_scope();
            return; 
        }
        /*assignment*/
        /* --- 6. HANDLE ASSIGNMENTS (Rules 10 & 15) --- */
        /* --- 6. HANDLE ASSIGNMENTS (Rules 10 & 15) --- */
        else if (strcmp(tree->token, "assign_stmt") == 0) {
            
            /* --- NEW: L-VALUE CHECK --- */
            node* l_node = tree->left;
            int is_lvalue = 0;
            
            /* It is a valid memory location if it is a pointer or array... */
            if (strcmp(l_node->token, "^") == 0 || strcmp(l_node->token, "array_access") == 0) {
                is_lvalue = 1;
            } 
            /* ...or if it is a plain variable name! */
            else if (l_node->left == NULL && l_node->right == NULL) {
                char f = l_node->token[0];
                if ((f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z')) {
                    if (strcmp(l_node->token, "true") != 0 && strcmp(l_node->token, "false") != 0 && strcmp(l_node->token, "null") != 0) {
                        is_lvalue = 1;
                    }
                }
            }
            
            if (!is_lvalue) {
                printf("Semantic Error: Left side of assignment must be a variable, pointer, or array cell!\n");
                exit(1); /* HARD STOP */
            }
            /* --------------------------- */

            char* left_type = get_expr_type(tree->left);
            char* right_type = get_expr_type(tree->right);
            
            /* Rule 15 Special Case: 'null' can only be assigned to pointers */
            if (strcmp(right_type, "null") == 0) {
                if (strstr(left_type, "*") == NULL) {
                    printf("Semantic Error: Cannot assign 'null' to a non-pointer!\n");
                    exit(1);
                }
            } 
            else if (strcmp(left_type, right_type) != 0) {
                
                int is_valid_cast = (strcmp(left_type, "real") == 0 && strcmp(right_type, "int") == 0);
                
                int is_string_assign = (strstr(left_type, "string") != NULL && strcmp(right_type, "string") == 0);
                
                if (!is_valid_cast && !is_string_assign) {
                    printf("Semantic Error: Cannot assign type '%s' to type '%s'!\n", right_type, left_type);
                    exit(1);
                }
            }
            
            build_table(tree->right);
            return;
        }
        
        /* function calls*/
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
            node* curr_actual_arg = tree->right; 
            int arg_index = 0;
            
            while (curr_actual_arg != NULL && 
                  (strcmp(curr_actual_arg->token, "arg") == 0 || strcmp(curr_actual_arg->token, "literal") == 0)) {
                
                char* actual_type = get_expr_type(curr_actual_arg->left);
                char* expected_type = sym->param_types[arg_index];
                
                if (strcmp(actual_type, expected_type) != 0) {
                    printf("Semantic Error: Argument #%d of '%s' expects type '%s', but got '%s'!\n", 
                           arg_index + 1, call_name, expected_type, actual_type);
                    exit(1); 
                }
                
                curr_actual_arg = curr_actual_arg->right;
                arg_index++;
            }
            
            build_table(tree->right); 
            return; 
        }
        /*function returns */
        else if (strcmp(tree->token, "return") == 0) {
            char* actual_return_type = "void";
            
            if (tree->left != NULL && strcmp(tree->left->token, "NONE") != 0) {
                actual_return_type = get_expr_type(tree->left);
            }

            if (strcmp(actual_return_type, current_scope->return_type) != 0) {
                printf("Semantic Error: Scope '%s' expects return type '%s', but got '%s'!\n",
                       current_scope->scope_name, current_scope->return_type, actual_return_type);
                exit(1); 
            }
            
            return;
        }
        
        /* veriable usage*/
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
        else if (strcmp(tree->token, "assign_stmt") == 0) {
            /* First walk left (the variable) to check it exists */
            build_table(tree->left);
            /* Then type-check the right-hand side expression */
            get_expr_type(tree->right);
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