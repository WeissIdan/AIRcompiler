#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Symbol {
    char* name;         
    char* type;         
    char* kind;      
    int num_params;   
    char** param_types;
    struct Symbol* next; 
} Symbol;

typedef struct Scope {
    char* scope_name;
    char* return_type;
    Symbol* head;        
    struct Scope* next;  
} Scope;

extern Scope* current_scope;

/* Core Functions */
void push_scope(const char* name, const char* return_type);
void pop_scope();                                    
int insert_symbol(char* name, char* type, char* kind, int num_params, char** param_types);
Symbol* lookup_symbol(char* name);                   
Symbol* lookup_current_scope(char* name);            
void print_scope();          

#endif