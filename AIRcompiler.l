%option yylineno
%{
#include "ast.h"
#include "y.tab.h" 
#include <string.h>
#include <stdlib.h>
%}

LETTER      [a-zA-Z]
DIGIT       [0-9]
WHITESPACE  [ \t\n\r]+
IDENTIFIER  {LETTER}({LETTER}|{DIGIT}|_)*
HEXDIGIT    [0-9a-fA-F]
INT_VAL     0|[1-9]{DIGIT}*|0[xX]{HEXDIGIT}+
REAL_VAL    {DIGIT}+(\.{DIGIT}+)?([eE][+-]?{DIGIT}+)?
STR_VAL     \"[^\"]*\"
COMMENT     \/\%([^%]|%+[^/])*\%\/

%%
"bool"      { return BOOL; }
"char"      { return CHAR; }
"int"       { return INT; }
"real"      { return REAL; }
"string"    { return STRING_TYPE; } 
"int*"      { return INT_PTR; }
"char*"     { return CHAR_PTR; }
"real*"     { return REAL_PTR; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"for"       { return FOR; }
"var"       { return VAR; }
"func"      { return FUNC; }
"proc"      { return PROC; }
"return"    { return RETURN; }
"null"      { return NULL_PTR; }

 /*Boolean Literals*/
"true"      { return TRUE_LITERAL; }
"false"     { return FALSE_LITERAL; }

 /*Operators*/
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULTIPLY; }
"/"         { return DIVIDE; }
"=="        { return EQUAL; }
"!="        { return NOT_EQUAL; }
">"         { return GREATER; }
">="        { return GREATER_EQUAL; }
"<"         { return LESS; }
"<="        { return LESS_EQUAL; }
"!"         { return NOT; }
"="         { return ASSIGN; }
"^"         { return DEREFERENCE; }
"&"         { return ADDRESS_OF; }
"|"         { return LENGTH_OP; }

 /*Punctuation*/
";"         { return ';'; }
":"         { return ':'; }
","         { return ','; }
"{"         { return '{'; }
"}"         { return '}'; }
"("         { return '('; }
")"         { return ')'; }
"["         { return '['; }
"]"         { return ']'; }

 /*COMPLEX RULES - FIXED TO USE MKNODE!*/

{IDENTIFIER} { 
    yylval.string_val = strdup(yytext); 
    return ID; 
}

{STR_VAL} {
    char *clean_str = strdup(yytext + 1);
    clean_str[strlen(clean_str) - 1] = '\0';
    yylval.string_val = clean_str;
    return STRING_LITERAL;
}

{INT_VAL} {
    yylval.int_val = atoi(yytext);
    return INT_LITERAL;
}

{REAL_VAL} {
    yylval.real_val = atof(yytext);
    return REAL_LITERAL;
}
 /*char*/
\'.\' {
    yylval.char_val = yytext[1]; 
    return CHAR_LITERAL;
}


 /*Whitespace & Error Handling*/
{WHITESPACE}+  {}
{COMMENT} {}
.   { printf("Lexical Error: Unknown character %s\n", yytext); }

%%

int yywrap() {
    return 1;
}