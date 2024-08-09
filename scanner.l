%{
    /* Include necessary standard libraries and header file */
    #define YYSTYPE struct node *

    #include <stdio.h> 
    #include <string.h> 
    #include "y.tab.h"
    
    int number_line = 1;
%}

%%

	/* Rules for keywords and types */
"bool" return BOOL;
"char" return CHAR;
"int" return INT;
"double" return DOUBLE;
"float" return FLOAT;
"string" return STRING;
"int*" return INT_P;
"char*" return CHAR_P;
"double*" return DOUBLE_P;
"float*" return FLOAT_P;

	/* Rules for control flow and function keywords */
"if" return IF;
"else" return ELSE;
"while" return WHILE;
"for" return FOR;
"var" return VAR;
"args>>" return ARGS;
"public" return PUBLIC;
"private" return PRIVATE;
"static" return STATIC;
"return" return RETURN;
"null" return NULL_P;
"void" return VOID;
"do" return DO;
"main" return MAIN;
"true" return BOOL_TRUE;
"false" return BOOL_FALSE;

	/* Rules for operators and symbols */
"&&" return AND;
\/ return DIV;
"<-" return ASSIGN;
"==" return EQUAL;
">" return BIGER;
">=" return GE;
"<" return LOWER;
"<=" return LE;
"-" return MINUS;
"!" return NOT;
"!=" return NOTEQUAL;
"||" return OR;
"+" return PLUS;
"*" return MULTI;
"&" return ADDRESS;

	/* Rules for punctuation and delimiters */
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

	/* Rules for identifiers and literals */
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