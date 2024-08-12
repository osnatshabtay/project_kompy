%{
    #define YYSTYPE struct node *
    #include "y.tab.h"
    int number_line = 1;
%}

%%
"int" return INT;
"double" return DOUBLE;
"float" return FLOAT;
"char" return CHAR;
"bool" return BOOL;
"int*" return INT_P;
"double*" return DOUBLE_P;
"float*" return FLOAT_P;
"char*" return CHAR_P;
"string" return STRING;
"void" return VOID;
"public" return PUBLIC;
"private" return PRIVATE;
"static" return STATIC;
"main" return MAIN;
"return" return RETURN;
"args>>" return ARGS;
"if" return IF;
"else" return ELSE;
"for" return FOR;
"while" return WHILE;
"do" return DO;
"var" return VAR;
"null" return NULL_P;
"true" return BOOL_TRUE;
"false" return BOOL_FALSE;
"&&" return AND;
"<-" return ASSIGN;
">" return BIGER;
">=" return GE;
"<" return LOWER;
"<=" return LE;
"!" return NOT;
"==" return EQUAL;
"!=" return NOTEQUAL;
"||" return OR;
"+" return PLUS;
"-" return MINUS;
"*" return MULTI;
\/ return DIV;
"&" return ADDRESS;
";" return ';';
"," return ',';
":" return ':';
"{" return '{' ;
"}" return '}';
"(" return '(';
")" return ')';
"[" return '[';
"]" return ']';
"|" return LEN;


[a-zA-Z][a-zA-Z0-9_]* return ID;  
[-]?[1-9][0-9]*|0 return INT_LITERAL;
[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)? return DOUBLE_LITERAL;
[0-9]*\.?[0-9]+f return FLOAT_LITERAL;
\'.\' return CHAR_LITERAL;
0(x|X)[0-9A-F]+ return HEX_LITERAL;
\"[^\"]*\"  return STRING_LITERAL;


"/*"([^*]|\*+[^*/])*\*+"/"

\n      ;number_line++;

[ \t\r] ;

. return yytext[0];


%%