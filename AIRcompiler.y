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


program: funcs { printtree($1, 0); };

funcs: func funcs { $$ = mknode("func", $1, $2); }
     | proc funcs { $$ = mknode("proc", $1, $2); }
     | func       { $$ = mknode("func", $1, NULL); }
     | proc       { $$ = mknode("proc", $1, NULL);};

/* fix mknode */
func: FUNC ID '(' arg_list ')' RETURN type '{' gen_stmts '}'{ $$ = mknode("func", mknode("", $7, $2), mknode("", $4, $9)); };
proc: PROC ID '(' arg_list ')' '{' gen_stmts '}' { $$ = mknode("proc", mknode("", NULL, $2), mknode("", $4, $7)); };
arg_list: args ':' type ';' arg_list { $$ = mknode("arg_list", mknode("", $1, $3), $5);}
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

type_literals: INT_LITERAL { $$ = mknode("int_lit", NULL, NULL);}
             | REAL_LITERAL { $$ = mknode("real_lit", NULL, NULL);}
             | STRING_LITERAL { $$ = mknode("string_lit", NULL, NULL);}
             | CHAR_LITERAL { $$ = mknode("char_lit", NULL, NULL);}
             | TRUE_LITERAL { $$ = mknode("true_lit", NULL, NULL);}
             | FALSE_LITERAL { $$ = mknode("false_lit", NULL, NULL);}


var_definition: VAR args ':' type ';' {$$ = mknode("var_def", $2, $4);};

gen_stmts: var_definition gen_stmts {$$ = mknode("gen_statements", $1, $2);}
         | stmts {$$ = $1;};

stmts: stmt stmts { $$ = mknode("stmts", $1, $2); }
     | stmt       { $$ = mknode("stmts", $1, NULL); };

stmt: if_stmt  { $$ = $1; }
    | for_stmt { $$ = $1; }
    | assign_stmt {$$ = $1;}
    | while_stmt {$$ = $1;}
    /* You will add while_stmt, assign_stmt, etc. here later */
    ;

if_stmt: IF '(' expr ')' if_body %prec LOWER_THAN_ELSE { $$ = mknode("if_stmt", $3, mknode("", $5, NULL)); }
       | IF '(' expr ')' if_body ELSE if_body { $$ = mknode("if_stmt", $3, mknode("", $5, $7)); };

while_stmt: WHILE '(' expr ')' stmt {$$ = mknode("while_stmt", $3, $5);}
          | WHILE '(' expr ')' '{' gen_stmts '}' {$$ = mknode("while_stmt", $3, $6);};

for_stmt: FOR '(' inits ';' expr ';' updates ')' for_body{ $$ = mknode("for", $3, mknode("", $5, mknode("", $7, $9)));};

for_body: stmt          { $$ = $1; }
        | '{' gen_stmts '}' { $$ = $2; };

if_body: stmt          { $$ = $1; }
       | '{' gen_stmts '}' { $$ = $2; };

assign_stmt: ID ASSIGN expr ';' {$$ = mknode("assign_stmt", $1, $3);};

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
    | ID                   { $$ = $1; }
    | ID '(' args ')'      { $$ = mknode("call", $1, $3); }
    | '(' expr ')'         { $$ = $2; }
    | DEREFERENCE expr     { $$ = mknode("^", $2, NULL); }
    | ADDRESS_OF expr      { $$ = mknode("&", $2, NULL); }
    | LENGTH_OP expr LENGTH_OP { $$ = mknode("|length|", $2, NULL); }
    | NULL_PTR             { $$ = mknode("null", NULL, NULL); }
    | MINUS expr %prec UMINUS { $$ = mknode("UMINUS", $2, NULL); }; 

inits: assign_stmt { $$ = $1; }
     | { $$ = NULL; };
updates: ID ASSIGN expr { $$ = mknode("assign_stmt", $1, $3); } 
       | { $$ = NULL; };



%%
#include "lex.yy.c"
int main()
{
    return yyparse();
}

void printtree(node *tree, int depth) {
    if (tree == NULL) return;

    /* Check if it is an invisible glue node */
    int is_glue = (tree->token != NULL && strcmp(tree->token, "") == 0);

    /* If it's a real node, print the directory-style branches */
    if (!is_glue) {
        for (int i = 0; i < depth; i++) {
            printf("|   "); // Print the vertical lines for depth
        }
        printf("|-- %s\n", tree->token); // Print the actual token
    }

    /* Increase depth only if it wasn't a glue node */
    int next_depth = is_glue ? depth : depth + 1;

    /* Visit left and right children */
    printtree(tree->left, next_depth);
    printtree(tree->right, next_depth);
}
int yyerror(const char* s)
{
    printf("MY ERROR\n");
    return 0;
}
node *mknode(char* token, node* left, node* right) {
    node* newnode = (node*)malloc(sizeof(node));
    char* newstr = (char*)malloc(sizeof(token) +1);
    strcpy(newstr, token);
    newnode->left = left;
    newnode->right = right;
    newnode->token = newstr;
    return newnode;



    // node *newnode = (node*)malloc(sizeof(node));
    // if (token != NULL) {
    //     newnode->token = strdup(token);
    // } else {
    //     newnode->token = NULL;
    // }
    // newnode->left = left;
    // newnode->right = right;
    // return newnode;
}


