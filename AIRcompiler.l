%{
#include "y.tab.h" 
%}

%%
"bool"      { return BOOL; }
"char"      { return CHAR; }
"int"       { return INT; }
"real"      { return REAL; }
"string"    { return STRING; } 
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
"null"      { return NULL; }

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
";"         { return .; }
":"         { return .; }
","         { return .; }
"{"         { return .; }
"}"         { return .; }
"("         { return .; }
")"         { return .; }
"["         { return .; }
"]"         { return .; }

/*Whitespace & Error Handling*/
[ \t\n\r]+  {}
.           { printf("Lexical Error: Unknown character %s\n", yytext); }


%%