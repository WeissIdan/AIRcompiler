%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "ast.h"
#define YYSTYPE node*
int yylex();
int yyerror(const char *s);
%}
%token ID STRING_LITERAL INT_LITERAL REAL_LITERAL CHAR_LITERAL
%token BOOL CHAR INT REAL STRING_TYPE INT_PTR CHAR_PTR REAL_PTR
%token IF ELSE WHILE FOR VAR FUNC PROC RETURN NULL_PTR
%token TRUE_LITERAL FALSE_LITERAL

%token PLUS MINUS MULTIPLY DIVIDE 
%token EQUAL NOT_EQUAL GREATER GREATER_EQUAL LESS LESS_EQUAL 
%token NOT ASSIGN DEREFERENCE ADDRESS_OF LENGTH_OP

%right ASSIGN 
%left EQUAL NOT_EQUAL
%left LESS LESS_EQUAL GREATER GREATER_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE
%right NOT DEREFERENCE ADDRESS_OF LENGTH_OP
%%


program: funcs { printtree($1); };

funcs: func funcs { $$ = mknode("funcs", $1, $2); }
     | func       { $$ = mknode("funcs", $1, NULL); }
     | proc       { $$ = mknode("funcs", $1, NULL);};

/* fix mknode */
func: FUNC ID '(' arg_list ')' RETURN type '{' gen_stmts '}'{ $$ = mknode("func", mknode("", $1, $2), mknode("", $4, $7)); };
proc: PROC ID '(' arg_list ')' '{' gen_stmts '}' { $$ = mknode("proc", mknode("", $1, $2), mknode("", $4, $7)); };
arg_list: args ':' type ';' arg_list { $$ = mknode("arg_list", $1, $3); }
        | args ':' type             { $$ = mknode("arg_list", $1, NULL); }
        |                  { $$ = NULL; };


args: ID ',' args { $$ = mknode("arg", $1, $3); }
    | ID { $$ = mknode("arg", $1, NULL);};

type: INT         { $$ = mknode("int", NULL, NULL); }
    | REAL        { $$ = mknode("real", NULL, NULL); }
    | BOOL        { $$ = mknode("bool", NULL, NULL); }
    | CHAR        { $$ = mknode("char", NULL, NULL); }
    | STRING_TYPE { $$ = mknode("string", NULL, NULL); }
    | INT_PTR     { $$ = mknode("int*", NULL, NULL); }
    | CHAR_PTR    { $$ = mknode("char*", NULL, NULL); }
    | REAL_PTR    { $$ = mknode("real*", NULL, NULL); };

type_literals: INT_LITERAL { $$ = mknode("int", NULL, NULL);}
             | REAL_LITERAL { $$ = mknode("real_literal", NULL, NULL);}
             | STRING_LITERAL { $$ = mknode("int", NULL, NULL);}
             | CHAR_LITERAL { $$ = mknode("int", NULL, NULL);}
             | TRUE_LITERAL { $$ = mknode("int", NULL, NULL);}
             | FALSE_LITERAL { $$ = mknode("int", NULL, NULL);}


var_definition: VAR args ':' type ';' {$$ = mknode("var_def", )};

gen_stmts: var_definition gen_stmts {$$ = mknode("gen_statements", $1, $2)}
         | stmts {$$ = $1};

stmts: stmt stmts { $$ = mknode("stmts", $1, $2); }
     | stmt       { $$ = mknode("stmts", $1, NULL); };

stmt: if_stmt  { $$ = $1; }
    | for_stmt { $$ = $1; }
    | assign_stmt {$$ = $1;}
    | while_stmt {$$ = $1;}
    | for_stmt {$$ = $1}
    /* You will add while_stmt, assign_stmt, etc. here later */
    ;

if_stmt: IF '(' expr ')' if_body { $$ = mknode("if_stmt", $3, mknode("", $5, NULL)); }
       | IF '(' expr ')' if_body ELSE if_body { $$ = mknode("if_stmt", $3, mknode("", $5, $7)); };

while_stmt: WHILE '(' expr ')' stmt {$$ = mknode("while_stmt", $3, $5)}
          | WHILE '(' expr ')' '{' gen_statements '}' {$$ = mknode("while_stmt", $3, $6)};

for_stmt: FOR '(' inits ';' expr ';' updates ')' for_body{ $$ = mknode("for", $3, mknode("", $5, mknode("", $7, $9)));};

for_body: stmt          { $$ = $1; }
        | '{' gen_statements '}' { $$ = $2; };

if_body: stmt          { $$ = $1; }
       | '{' gen_stmts '}' { $$ = $2; };

assign_stmt: ID '=' expr ';' {$$ = mknode("assign_stmt", $1, $3);};

expr: expr operator expr
    | type_literals
    | ID
    | ID '(' args ')'




%%
#include "lex.yy.c"
int main()
{
    return yyparse();
}
void printtree(node *tree)
{
    printf("%s\n", tree->token);
    if(tree->left)
    printtree(tree->left);
    if(tree->right)
    printtree(tree->right);
}
int yyerror()
{
    printf("MY ERROR\n");
    return 0;
}
node *mknode(char* token, node* left, node* right) {
    node *newnode = (node*)malloc(sizeof(node));
    if (token != NULL) {
        newnode->token = strdup(token);
    } else {
        newnode->token = NULL;
    }
    newnode->left = left;
    newnode->right = right;
    return newnode;
}


