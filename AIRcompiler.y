%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "ast.h"

extern int yylineno; 
extern char* yytext;

int yylex();
int yyerror(const char *s);
%}

%union {
    int int_val;
    double real_val;
    char char_val;
    char* string_val;
    struct node* ast_node;
}
%token <string_val> ID STRING_LITERAL
%token <int_val> INT_LITERAL 
%token <real_val> REAL_LITERAL
%token <char_val> CHAR_LITERAL

%token BOOL CHAR INT REAL STRING_TYPE INT_PTR CHAR_PTR REAL_PTR
%token IF ELSE WHILE FOR VAR FUNC PROC RETURN NULL_PTR
%token TRUE_LITERAL FALSE_LITERAL

%token PLUS MINUS MULTIPLY DIVIDE 
%token EQUAL NOT_EQUAL GREATER GREATER_EQUAL LESS LESS_EQUAL 
%token NOT ASSIGN DEREFERENCE ADDRESS_OF LENGTH_OP

%type <ast_node> program funcs func proc arg_list args type type_literals
%type <ast_node> var_definition gen_stmts stmts stmt if_stmt while_stmt
%type <ast_node> for_stmt for_body if_body assign_stmt expr inits updates

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGN 
%left EQUAL NOT_EQUAL
%left LESS LESS_EQUAL GREATER GREATER_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE
%right NOT DEREFERENCE ADDRESS_OF LENGTH_OP
%right UMINUS
%%


program: funcs { printtree($1, 0); printf("\n");};

funcs: func funcs { $$ = mknode("funcs", $1, $2); }
     | proc funcs { $$ = mknode("procs", $1, $2); }
     | func       { $$ = mknode("funcs", $1, NULL); }
     | proc       { $$ = mknode("procs", $1, NULL);};

/* fix mknode */
func: FUNC ID '(' arg_list ')' RETURN type '{' gen_stmts '}'{ $$ = mknode("func", mknode("", $7, mknode($2, NULL, NULL)), mknode("", $4, $9)); };
proc: PROC ID '(' arg_list ')' '{' gen_stmts '}' { $$ = mknode("proc", mknode("", NULL, mknode($2, NULL, NULL)), mknode("", $4, $7)); };
arg_list: args ':' type ';' arg_list { $$ = mknode("arg_list", mknode("", $1, $3), $5);}
        | args ':' type             { $$ = mknode("arg_list", $1, $3); }
        |                  { $$ = NULL; };


args: ID ',' args { $$ = mknode("arg", mknode($1, NULL, NULL), $3); }
    | ID { $$ = mknode("arg", mknode($1, NULL, NULL), NULL);};

type: INT         { $$ = mknode("int", NULL, NULL); }
    | REAL        { $$ = mknode("real", NULL, NULL); }
    | BOOL        { $$ = mknode("bool", NULL, NULL); }
    | CHAR        { $$ = mknode("char", NULL, NULL); }
    | STRING_TYPE { $$ = mknode("string", NULL, NULL); }
    | STRING_TYPE '[' INT_LITERAL ']'{   
        char buffer[50];
        sprintf(buffer, "string[%d]", $3);
        $$ = mknode(buffer, NULL, NULL); 
        }
    | INT_PTR     { $$ = mknode("int*", NULL, NULL); }
    | CHAR_PTR    { $$ = mknode("char*", NULL, NULL); }
    | REAL_PTR    { $$ = mknode("real*", NULL, NULL); };

type_literals: INT_LITERAL { 
                 char buffer[50];
                 sprintf(buffer, "%d", $1);
                 $$ = mknode(buffer, NULL, NULL);
             }
             | REAL_LITERAL { 
                 char buffer[50];
                 sprintf(buffer, "%g", $1);
                 $$ = mknode(buffer, NULL, NULL);
             }
             | STRING_LITERAL { $$ = mknode($1, NULL, NULL); }
             | CHAR_LITERAL { 
                 char buffer[4];
                 sprintf(buffer, "'%c'", $1);
                 $$ = mknode(buffer, NULL, NULL);
             }
             | TRUE_LITERAL { $$ = mknode("true", NULL, NULL);}
             | FALSE_LITERAL { $$ = mknode("false", NULL, NULL);};


var_definition: VAR args ':' type ';' {$$ = mknode("var_def", $2, $4);};

gen_stmts: var_definition gen_stmts {$$ = mknode("gen_statements", $1, $2);}
         | stmts {$$ = $1;};

stmts: stmt stmts { $$ = mknode("stmts", $1, $2); }
     | stmt       { $$ = mknode("stmts", $1, NULL); };

stmt: if_stmt  { $$ = $1; }
    | for_stmt { $$ = $1; }
    | assign_stmt {$$ = $1;}
    | while_stmt {$$ = $1;}
    | RETURN expr ';' { $$ = mknode("return", $2, NULL); }
    | RETURN ';'      { $$ = mknode("return", mknode("NONE", NULL, NULL), NULL); };

if_stmt: IF '(' expr ')' if_body %prec LOWER_THAN_ELSE { $$ = mknode("if_stmt", $3, mknode("", $5, NULL)); }
       | IF '(' expr ')' if_body ELSE if_body { $$ = mknode("if_stmt", mknode("",$3,$5), mknode("else", $7, NULL)); };

while_stmt: WHILE '(' expr ')' stmt {$$ = mknode("while_stmt", $3, $5);}
          | WHILE '(' expr ')' '{' gen_stmts '}' {$$ = mknode("while_stmt", $3, $6);};

for_stmt: FOR '(' inits ';' expr ';' updates ')' for_body{ $$ = mknode("for", $3, mknode("", $5, mknode("", $7, $9)));};

for_body: stmt          { $$ = $1; }
        | '{' gen_stmts '}' { $$ = $2; };

if_body: stmt          { $$ = $1; }
       | '{' gen_stmts '}' { $$ = $2; };

assign_stmt: ID ASSIGN expr ';' {$$ = mknode("assign_stmt", mknode($1, NULL, NULL), $3);};

updates: ID ASSIGN expr { $$ = mknode("assign_stmt", mknode($1, NULL, NULL), $3); } 
       | { $$ = NULL; };

expr: expr PLUS expr       { $$ = mknode("+", $1, $3); }
    | expr MINUS expr      { $$ = mknode("-", $1, $3); }
    | expr MULTIPLY expr   { $$ = mknode("*", $1, $3); }
    | expr DIVIDE expr     { $$ = mknode("/", $1, $3); }
    | expr EQUAL expr      { $$ = mknode("==", $1, $3); }
    | expr NOT_EQUAL expr  { $$ = mknode("!=", $1, $3); }
    | expr GREATER expr    { $$ = mknode(">", $1, $3); }
    | expr GREATER_EQUAL expr { $$ = mknode(">=", $1, $3); }
    | expr LESS expr       { $$ = mknode("<", $1, $3); }
    | expr LESS_EQUAL expr { $$ = mknode("<=", $1, $3); }
    | NOT expr             { $$ = mknode("!", $2, NULL); }
    | type_literals        { $$ = $1; }
    | ID                   { $$ = mknode($1, NULL, NULL); }
    | ID '(' args ')'      { $$ = mknode("call", mknode($1, NULL, NULL), $3); }
    | '(' expr ')'         { $$ = $2; }
    | DEREFERENCE expr     { $$ = mknode("^", $2, NULL); }
    | ADDRESS_OF expr      { $$ = mknode("&", $2, NULL); }
    | LENGTH_OP expr LENGTH_OP { $$ = mknode("|length|", $2, NULL); }
    | NULL_PTR             { $$ = mknode("null", NULL, NULL); }
    | MINUS expr %prec UMINUS { $$ = mknode("UMINUS", $2, NULL); }; 

inits: assign_stmt { $$ = $1; }
     | { $$ = NULL; };




%%
#include "lex.yy.c"
int main()
{
    return yyparse();
}
/*=======================
PRINT TREE WITH ONLY ()
=========================*/
// void printtree(node *tree, int depth) {
//     if (tree == NULL) return;

//     int is_glue = (tree->token != NULL && strcmp(tree->token, "") == 0);
//     int has_children = (tree->left != NULL || tree->right != NULL);

//     int children_are_leaves = 1;
//     if (tree->left != NULL && (tree->left->left != NULL || tree->left->right != NULL)) {
//         children_are_leaves = 0; 
//     }
//     if (tree->right != NULL && (tree->right->left != NULL || tree->right->right != NULL)) {
//         children_are_leaves = 0; 

//     if (!is_glue) {
//         if (has_children) {
//             printf("\n");
//             for (int i = 0; i < depth; i++) printf("  ");
//             printf("(%s", tree->token);
//         } else {
//             printf(" %s", tree->token);
//             return; 
//         }
//     }

//     int next_depth = is_glue ? depth : depth + 1;
//     printtree(tree->left, next_depth);
//     printtree(tree->right, next_depth);

//     if (!is_glue && has_children) {
//         if (children_are_leaves) {
//             printf(")"); 
//         } else {
//             printf("\n");
//             for (int i = 0; i < depth; i++) printf("  ");
//             printf(")");
//         }
//     }
// }
int yyerror(const char* s)
{
    printf("Syntax Error on line %d: %s at or near '%s'\n", yylineno, s, yytext);
    return 0;
}
node *mknode(char* token, node* left, node* right) {
    node* newnode = (node*)malloc(sizeof(node));
    char* newstr = (char*)malloc(strlen(token) + 1); 
    strcpy(newstr, token);
    newnode->left = left;
    newnode->right = right;
    newnode->token = newstr;
    return newnode;
}


void printtree(node *tree, int depth) {
    if (tree == NULL) return;

    int is_glue = (tree->token != NULL && strcmp(tree->token, "") == 0);
    int has_children = (tree->left != NULL || tree->right != NULL);

    int children_are_leaves = 1;
    if (tree->left != NULL && (tree->left->left != NULL || tree->left->right != NULL)) {
        children_are_leaves = 0; 
    }
    if (tree->right != NULL && (tree->right->left != NULL || tree->right->right != NULL)) {
        children_are_leaves = 0; 
    }

    char open_b = '(';
    char close_b = ')';
    
    if (!is_glue && tree->token != NULL) {
        if (strcmp(tree->token, "func") == 0 || strcmp(tree->token, "proc") == 0 ||
            strcmp(tree->token, "funcs") == 0 || strcmp(tree->token, "procs") == 0) {
            open_b = '{';
            close_b = '}';
        }
    }

    if (!is_glue) {
        if (has_children) {
            printf("\n");
            for (int i = 0; i < depth; i++) printf("  ");
            printf("%c%s", open_b, tree->token);
        } else {
            printf(" %s", tree->token);
            return; 
        }
    }

    int next_depth = is_glue ? depth : depth + 1;
    printtree(tree->left, next_depth);
    printtree(tree->right, next_depth);

    if (!is_glue && has_children) {
        if (children_are_leaves) {
            printf("%c", close_b); 
        } else {
            printf("\n");
            for (int i = 0; i < depth; i++) printf("  ");
            printf("%c", close_b);
        }
    }
}

