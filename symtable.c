#include "symtable.h"

Scope* current_scope = NULL;

void print_scope() {
    if (current_scope == NULL) return;
    
    printf("\n=== SCOPE SYMBOL TABLE ===\n");
    printf("%-15s | %-15s | %-15s\n", "NAME", "TYPE", "KIND");
    printf("--------------------------------------------------\n");
    
    Symbol* runner = current_scope->head;
    if (runner == NULL) {
        printf(" (Empty Scope)\n");
    }
    
    while (runner != NULL) {
        printf("%-15s | %-15s | %-15s\n", 
               runner->name, 
               runner->type ? runner->type : "N/A", 
               runner->kind ? runner->kind : "N/A");
        runner = runner->next;
    }
    printf("==========================\n");
}

void push_scope() {
    Scope* new_scope = (Scope*)malloc(sizeof(Scope));
    new_scope->head = NULL;
 
    new_scope->next = current_scope; 
    
    current_scope = new_scope; 
}

/* Exit a scope (Pops the top table and deletes it) */
void pop_scope() {
    if (current_scope == NULL) return;

    print_scope(); 

    Scope* temp_scope = current_scope;
    current_scope = current_scope->next; /* Drop down to the parent scope */

    Symbol* curr_sym = temp_scope->head;
    while (curr_sym != NULL) {
        Symbol* temp_sym = curr_sym;
        curr_sym = curr_sym->next;
        free(temp_sym->name);
        if (temp_sym->type) free(temp_sym->type);
        if (temp_sym->kind) free(temp_sym->kind);
        free(temp_sym);
    }
    free(temp_scope);
}

int insert_symbol(char* name, char* type, char* kind) {
    if (current_scope == NULL) push_scope(); 

    if (lookup_current_scope(name) != NULL) {
        return 0;
    }

    Symbol* new_sym = (Symbol*)malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->type = type ? strdup(type) : NULL;
    new_sym->kind = kind ? strdup(kind) : NULL;

    new_sym->next = current_scope->head;
    current_scope->head = new_sym;

    return 1; 
}

Symbol* lookup_symbol(char* name) {
    Scope* scope_runner = current_scope;

    while (scope_runner != NULL) {
        Symbol* sym_runner = scope_runner->head;
        while (sym_runner != NULL) {
            if (strcmp(sym_runner->name, name) == 0) {
                return sym_runner; 
            }
            sym_runner = sym_runner->next;
        }
        scope_runner = scope_runner->next;
    }
    return NULL; 
}

Symbol* lookup_current_scope(char* name) {
    if (current_scope == NULL) return NULL;

    Symbol* sym_runner = current_scope->head;
    while (sym_runner != NULL) {
        if (strcmp(sym_runner->name, name) == 0) {
            return sym_runner;
        }
        sym_runner = sym_runner->next;
    }
    return NULL;
}

