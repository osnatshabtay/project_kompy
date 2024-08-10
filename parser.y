%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lex.yy.c"
#include "semantic_analysis.c"
int yylex(void);
int yyerror(char *s);

%}
/* Token definitions */
%token PUBLIC PRIVATE VOID RETURN MAIN STATIC
%token INT DOUBLE FLOAT ID INT_LITERAL CHAR BOOL INT_P CHAR_P DOUBLE_P FLOAT_P STRING BOOL_TRUE BOOL_FALSE NULL_P DOUBLE_LITERAL FLOAT_LITERAL CHAR_LITERAL HEX_LITERAL STRING_LITERAL VAR SIZE ARGS
%token IF ELSE WHILE FOR DO
%token BIGER PLUS ASSIGN ',' DIV AND EQUAL GE LOWER LE MINUS NOT NOTEQUAL OR MULTI ADDRESS LEN
%token ';' '{' '}' '(' ')' '[' ']' ':' 

/* Non-associative tokens */
%nonassoc PREC_IF
%nonassoc ELSE

/* Operator precedence and associativity */
%right ASSIGN NOT ';' MAIN
%left '{' '}' '(' ')'
%left AND OR
%left EQUAL BIGER GE LE LOWER NOTEQUAL 
%left MINUS PLUS
%left MULL MULTI DIV
%start s

%%

/* Grammar rules */
s:
    code { semanticAnalysis($1); }
    ;

code:
    functions Main {$$ = makeNode ("CODE"); addNodesList($$, $1); addSonNodeToFatherNode(&$$, $2);}
    | Main {$$ = makeNode ("CODE"); addSonNodeToFatherNode(&$$, $1);} 
    ;

Main:
    PUBLIC VOID MAIN '(' args ')' ':' STATIC void_block {
        $$ = makeNode("MAIN"); addSonNodeToFatherNode(&$$, $9);}
    ;


functions:
    functions function_type {$$ = combineNodes("", $1, $2);}
    |function_type
    ;

function_type: 
    function_void  
    |function 
    ;

function_void: 
    visibility VOID id '(' args ')' void_block {$$ = makeNode("s"); node* s = makeNode ("FUNCTION"); s->line_number = $3->line_number; node* v = makeNode("VOID"); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$5); addSonNodeToFatherNode(&s, v); addSonNodeToFatherNode(&s, $7); addSonNodeToFatherNode(&$$,s);}
    |visibility VOID id '(' args ')' ':' STATIC void_block {$$ = makeNode("s"); node* s = makeNode ("STATIC-FN"); s->line_number = $3->line_number; node* v = makeNode("VOID");addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$5); addSonNodeToFatherNode(&s, v); addSonNodeToFatherNode(&s, $9); addSonNodeToFatherNode(&$$,s);}
    |visibility VOID id '(' ARGS args ')' void_block {$$ = makeNode("s"); node* s = makeNode ("FUNCTION"); s->line_number = $3->line_number; node* v = makeNode("VOID"); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$6); addSonNodeToFatherNode(&s, v); addSonNodeToFatherNode(&s, $8); addSonNodeToFatherNode(&$$,s);}
    |visibility VOID id '(' ARGS args ')' ':' STATIC void_block {$$ = makeNode("s"); node* s = makeNode ("STATIC-FN"); s->line_number = $3->line_number; node* v = makeNode("VOID"); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$6); addSonNodeToFatherNode(&s, v); addSonNodeToFatherNode(&s, $10); addSonNodeToFatherNode(&$$,s);}
    ; 

function: 
    visibility func_type id '(' args ')' function_block {$$ = makeNode("s"); node* s = makeNode ("FUNCTION"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$5); addSonNodeToFatherNode(&s, $2); addSonNodeToFatherNode(&s, $7); addSonNodeToFatherNode(&$$,s);}
    |visibility func_type id '(' args ')' ':' STATIC function_block {$$ = makeNode("s"); node* s = makeNode ("STATIC-FN"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$5); addSonNodeToFatherNode(&s, $2); addSonNodeToFatherNode(&s, $9); addSonNodeToFatherNode(&$$,s);}
    |visibility func_type id '(' ARGS args ')' function_block {$$ = makeNode("s"); node* s = makeNode ("FUNCTION"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$6); addSonNodeToFatherNode(&s, $2); addSonNodeToFatherNode(&s, $8); addSonNodeToFatherNode(&$$,s);}
    |visibility func_type id '(' ARGS args ')' ':' STATIC function_block {$$ = makeNode("s"); node* s = makeNode ("STATIC-FN"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&s,$6); addSonNodeToFatherNode(&s, $2); addSonNodeToFatherNode(&s, $10); addSonNodeToFatherNode(&$$,s);}
    ;

visibility: 
    PUBLIC {$$ = makeNode ("PUBLIC");}
    | PRIVATE {$$ = makeNode ("PRIVATE");}
    ;

 void_block:
    '{' '}' {$$ = makeNode ("BODY"); }
    |'{' declerations statments '}' {$$ = makeNode ("BODY"); node* v = makeNode("VAR"); addNodesList(v,$2);addSonNodeToFatherNode(&$$,v); addNodesList($$, $3);}
    |'{' statments '}' {$$ = makeNode ("BODY"); addNodesList($$,$2);}
    |'{' functions declerations statments '}' {$$ = makeNode("BODY"); addNodesList($$, $2); node* v = makeNode("VAR"); addNodesList(v,$3);addSonNodeToFatherNode(&$$,v);addNodesList($$,$4);}
    |'{' functions statments '}' {$$ = makeNode("BODY"); addNodesList($$, $2); addNodesList($$,$3);}
    |'{' functions '}' {$$ = makeNode("BODY"); addNodesList($$, $2);}
    |'{' declerations '}' {$$ = makeNode ("BODY"); node* v = makeNode("VAR"); addNodesList(v,$2);addSonNodeToFatherNode(&$$,v);}
    ;
    
function_block:
    '{' RETURN expression ';' '}' {$$ = makeNode ("BODY"); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$3); addSonNodeToFatherNode(&$$,ret);}
    |'{' statments RETURN expression ';' '}' {$$ = makeNode ("BODY"); addNodesList($$, $2); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$4); addSonNodeToFatherNode(&$$,ret);}
    |'{' functions statments RETURN expression ';' '}' {$$ = makeNode("BODY"); addNodesList($$, $2); addNodesList($$,$3); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$5); addSonNodeToFatherNode(&$$,ret);}
    |'{' declerations statments RETURN expression ';' '}' {$$ = makeNode ("BODY");node* v = makeNode("VAR"); addNodesList(v, $2); addSonNodeToFatherNode(&$$,v); addNodesList($$, $3); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$5); addSonNodeToFatherNode(&$$,ret);}
    |'{' functions declerations statments RETURN expression ';' '}' {$$ = makeNode("BODY"); addNodesList($$, $2);  node* v = makeNode("VAR"); addNodesList(v, $3); addSonNodeToFatherNode(&$$,v); addNodesList($$, $4); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$6); addSonNodeToFatherNode(&$$,ret);}
    |'{' declerations functions statments RETURN expression ';' '}' {$$ = makeNode("BODY"); addNodesList($$, $2);  node* v = makeNode("VAR"); addNodesList(v, $3); addSonNodeToFatherNode(&$$,v); addNodesList($$, $4); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$6); addSonNodeToFatherNode(&$$,ret);}
    |'{' declerations functions RETURN expression ';' '}' {$$ = makeNode("BODY"); addNodesList($$, $2);  node* v = makeNode("VAR"); addNodesList(v, $3); addSonNodeToFatherNode(&$$,v); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$5); addSonNodeToFatherNode(&$$,ret);}
    |'{' functions RETURN expression ';' '}' {$$ = makeNode("BODY"); addNodesList($$, $2); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$4); addSonNodeToFatherNode(&$$,ret);}
    |'{' declerations RETURN expression ';' '}' {$$ = makeNode ("BODY"); node* v = makeNode("VAR"); addNodesList(v,$2);addSonNodeToFatherNode(&$$,v); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$4); addSonNodeToFatherNode(&$$,ret);}
    ; 
 
statments:
    statments statment {$$ = combineNodes("", $1, $2);}
    |statment
    ;
    
statment:
    init_for ';'
    |ass_string ';'
    |if_statment 
    |if_else_statment
    |loop
    |block 
    |func_call ';'
    |RETURN expression ';' {$$ = makeNode("s"); node* s = makeNode("RET"); addSonNodeToFatherNode(&s, $2); addSonNodeToFatherNode(&$$, s);}
    |RETURN ';' {$$ = makeNode("s"); node* s = makeNode("RET"); addSonNodeToFatherNode(&$$, s);}
    ;

if_statment:
    IF '(' expression ')' statment {$$ = makeNode("s"); node* s = makeNode("IF"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s, $3);  addNodesList(s,$5); s->sons_nodes[1]->father = "IF"; addSonNodeToFatherNode(&$$,s);}  %prec PREC_IF
    ;

if_else_statment:
    IF '(' expression ')' statment ELSE statment {$$ = makeNode("s"); node* s = makeNode("IF-ELSE"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s, $3); addNodesList(s,$5); s->sons_nodes[1]->father = "IF"; addNodesList(s, $7); addSonNodeToFatherNode(&$$,s);}
    ;

loop: 
    FOR '(' init_for ';' expression ';' init_for ')' statment 
    {
        $$ = makeNode(""); 
        node* for_node = makeNode("FOR"); 
        for_node->line_number = $3->line_number; 
        addSonsNodesToFatherNode(for_node, $3); 
        addSonNodeToFatherNode(&for_node, $5); 
        addSonsNodesToFatherNode(for_node, $7); 
        addSonsNodesToFatherNode(for_node, $9); 
        addSonNodeToFatherNode(&$$,for_node);
    }
    | WHILE '(' expression ')' statment 
    {
        $$ = makeNode(""); 
        node* while_node = makeNode("WHILE"); 
        while_node->line_number = $3->line_number; 
        addSonNodeToFatherNode(&while_node, $3); 
        addSonsNodesToFatherNode(while_node, $5); 
        addSonNodeToFatherNode(&$$,while_node);}
 
    | DO block WHILE '(' expression ')' ';' 
    {
        $$ = makeNode(""); 
        node* while_node = makeNode("DO-WHILE"); 
        while_node->line_number = $5->line_number; 
        addSonsNodesToFatherNode(while_node, $2); 
        addSonNodeToFatherNode(&while_node, $5); 
        addSonNodeToFatherNode(&$$,while_node);
    }

    ;



init_for:
    id ASSIGN expression {$$ = makeNode("s"); node* s = makeNode("<-"); s->line_number = number_line; addSonNodeToFatherNode(&s,$1); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&$$,s);}
    | pointer ASSIGN expression {$$ = makeNode("s"); node* s = makeNode("<-"); s->line_number = number_line; addSonNodeToFatherNode(&s,$1); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&$$,s);}
    ;

ass_string:
    id '[' expression ']' ASSIGN expression {$$ = makeNode("s"); node* s = makeNode("<-"); node* index = makeNode("INDEX"); addSonNodeToFatherNode(&index,$3); addSonNodeToFatherNode(&$1, index); addSonNodeToFatherNode(&s, $1); addSonNodeToFatherNode(&s,$6); addSonNodeToFatherNode(&$$,s);}
    ;

expression:
    expression PLUS expression {$$ = makeNode("+"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression MINUS expression {$$ = makeNode("-"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression MULTI expression {$$ = makeNode("*"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);} 
    | expression DIV expression {$$ = makeNode("/"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression EQUAL expression { $$ = makeNode ("=="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression BIGER expression { $$ = makeNode (">"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression GE expression { $$ = makeNode (">="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression LOWER expression { $$ = makeNode ("<"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression LE expression { $$ = makeNode ("<="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression NOTEQUAL expression { $$ = makeNode ("!="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression AND expression {$$ = makeNode("&&"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$,$3);} 
    | expression OR expression {$$ = makeNode("||"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$,$3);} 
    | NOT expression {$$ = makeNode ("NOT"); $$->line_number = $2->line_number; addSonNodeToFatherNode(&$$,$2);}
    | '(' expression ')' {$$ = $2; $$->line_number = $2->line_number;}
    | LEN expression LEN {$$ = makeNode("LEN"); addSonNodeToFatherNode(&$$,$2);}
    | ADDRESS id {$$ = makeNode("&"); addSonNodeToFatherNode(&$$, $2);}
    | ADDRESS string_id {$$ = makeNode("&"); addSonNodeToFatherNode(&$$, $2);}
        | INT_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "INT"; 
        $$->line_number = number_line;
    }
    | HEX_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "INT"; 
        $$->line_number = number_line;
    }
    | DOUBLE_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "DOUBLE"; 
        $$->line_number = number_line;
    }
    | FLOAT_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "FLOAT"; 
        $$->line_number = number_line;
    }
    | string_id 
    | BOOL_TRUE 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "BOOL"; 
        $$->line_number = number_line;
    }
    | BOOL_FALSE 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "BOOL"; 
        $$->line_number = number_line;
    }
    | NULL_P  
    { 
        $$ = makeNode (yytext); 
        $$->node_type = "NULL"; 
        $$->line_number = number_line;
    }
    | id
    | CHAR_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "CHAR"; 
        $$->line_number = number_line;
    }
    | STRING_LITERAL 
    {
        $$ = makeNode(yytext); 
        $$->node_type = "STRING"; 
        $$->line_number = number_line;
    }
    | pointer %prec MULL
    ;

    ;

func_call: 
    id '(' parameter_list ')' {$$ = makeNode("s"); node* s = makeNode ("FUNC_CALL");  addSonNodeToFatherNode(&s,$1); node* args = makeNode("ARGS"); args->line_number = $1->line_number; addNodesList(args, $3); addSonNodeToFatherNode(&s, args); addSonNodeToFatherNode(&$$,s);}
    |id ASSIGN id '(' parameter_list ')' {$$ = makeNode("s"); node* s = makeNode ("<-"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$1); node* call = makeNode("FUNC_CALL"); addSonNodeToFatherNode(&call,$3); node* args = makeNode("ARGS"); args->line_number = $1->line_number; addNodesList(args, $5); addSonNodeToFatherNode(&call, args); addSonNodeToFatherNode(&s, call); addSonNodeToFatherNode(&$$,s);}
    |id '(' ')' {$$ = makeNode("s"); node* s = makeNode ("FUNC_CALL"); addSonNodeToFatherNode(&s,$1); node* args = makeNode("ARGS NONE"); args->line_number = $1->line_number; addSonNodeToFatherNode(&s, args); addSonNodeToFatherNode(&$$,s);}
    |id ASSIGN id '(' ')' {$$ = makeNode("s"); node* s = makeNode ("<-"); s->line_number = $3->line_number; addSonNodeToFatherNode(&s,$1); node* call = makeNode("FUNC_CALL"); addSonNodeToFatherNode(&call,$3); node* args = makeNode("ARGS NONE"); args->line_number = $1->line_number; addSonNodeToFatherNode(&call, args); addSonNodeToFatherNode(&s, call); addSonNodeToFatherNode(&$$,s);}
    ; 

parameter_list:
    parameter_list ',' parm {$$ = combineNodes("", $1, $3);}
    | parm {$$ = $1;}
    ;

parm:
    expression {$$ = makeNode("s"); addSonNodeToFatherNode(&$$,$1);}
    ;

block:
    '{' '}' {$$ =  makeNode("BLOCK");}
    |'{' statments '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); addNodesList(s,$2); addSonNodeToFatherNode(&$$,s);}
    |'{' declerations '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); node* v = makeNode("VAR"); addNodesList(v,$2); addSonNodeToFatherNode(&s,v); addSonNodeToFatherNode(&$$,s);}
    |'{' declerations statments '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); node* v = makeNode("VAR"); addNodesList(v,$2); addSonNodeToFatherNode(&s,v); addNodesList(s, $3); addSonNodeToFatherNode(&$$,s);}    
    |'{' RETURN expression ';' '}' {$$ = makeNode("BLOCK"); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$3); addSonNodeToFatherNode(&$$,ret);}
    |'{' statments RETURN expression ';' '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); addNodesList(s,$2);node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$4); addSonNodeToFatherNode(&s,ret); addSonNodeToFatherNode(&$$,s);}
    |'{' declerations RETURN expression ';' '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); node* v = makeNode("VAR"); addNodesList(v,$2); addSonNodeToFatherNode(&s,v); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$4); addSonNodeToFatherNode(&s,ret); addSonNodeToFatherNode(&$$,s);}
    |'{' declerations statments RETURN expression ';' '}' {$$ = makeNode("s"); node* s = makeNode("BLOCK"); node* v = makeNode("VAR"); addNodesList(v,$2); addSonNodeToFatherNode(&s,v); addNodesList(s, $3); node* ret = makeNode("RET"); addSonNodeToFatherNode(&ret,$5); addSonNodeToFatherNode(&s,ret); addSonNodeToFatherNode(&$$,s);}  
    ;   

func_type:
    var_type
    |STRING {$$ = makeNode("STRING");}
    ;

var_type:  
    INT {$$ = makeNode ("INT");}
    |DOUBLE {$$ = makeNode ("DOUBLE");} 
    |FLOAT {$$ = makeNode ("FLOAT");} 
    |CHAR {$$ = makeNode ("CHAR");}
    |BOOL {$$ = makeNode ("BOOL");}
    |INT_P {$$ = makeNode ("INT*");}
    |CHAR_P {$$ = makeNode ("CHAR*");}
    |DOUBLE_P {$$ = makeNode ("DOUBLE*");}
    |FLOAT_P {$$ = makeNode ("FLOAT*");}
    ;    
args:
    /* empty */ {$$ = makeNode("ARGS NONE");}
    | args_decleration {$$ = makeNode("ARGS"); addNodesList($$,$1);}
    | args_decleration ';' args {$$ = makeNode("ARGS"); addNodesList($$,$1); addNodesList($$, $3);}
    ;

declerations:
    declerations var_decleration {$$ = combineNodes("fd",$1,$2);}
    |var_decleration
    |declerations string_decleration {$$ = combineNodes("fd",$1,$2);}
    |string_decleration
    ;

var_decleration:
    VAR params_decleration ';' {$$ = $2;}
    ;

string_decleration:
    STRING string_params ';' {$$ = makeNode("s"); node* s = makeNode("STRING"); addNodesList(s,$2); addSonNodeToFatherNode(&$$,s);}
    ;

string_params:
    id '[' expression ']' {$$ = makeNode("s"); node* len = makeNode("LENGTH"); addSonNodeToFatherNode(&len,$3); addSonNodeToFatherNode(&$1,len); addSonNodeToFatherNode(&$$,$1);}
    |id '[' expression ']' ',' string_params {$$ = makeNode("s"); node* len = makeNode("LENGTH"); addSonNodeToFatherNode(&len,$3); addSonNodeToFatherNode(&$1,len); addSonNodeToFatherNode(&$$,$1); addNodesList($$,$6);}
    |id '[' expression ']' ASSIGN expression {$$ = makeNode("s"); node* ass = makeNode("<-"); node* len = makeNode("LENGTH"); addSonNodeToFatherNode(&len,$3); addSonNodeToFatherNode(&$1,len); addSonNodeToFatherNode(&ass,$1); addSonNodeToFatherNode(&ass,$6);addSonNodeToFatherNode(&$$,ass);}
    |id '[' expression ']' ASSIGN expression ',' string_params {$$ = makeNode("s"); node* ass = makeNode("<-"); node* len = makeNode("LENGTH"); addSonNodeToFatherNode(&len,$3); addSonNodeToFatherNode(&$1,len); addSonNodeToFatherNode(&ass,$1); addSonNodeToFatherNode(&ass,$6);addSonNodeToFatherNode(&$$,ass); addNodesList($$,$8);}
    ; 

args_decleration:
    var_type params {$$ = makeNode("s"); node* s = $1; addSonsNodesToFatherNode(s, $2); addSonNodeToFatherNode(&$$,s);}
    |var_type ':' params {$$ = makeNode("s"); node* s = $1; addSonsNodesToFatherNode(s, $3); addSonNodeToFatherNode(&$$,s);}
    ;
    ;

params_decleration:
    var_type variables {$$ = makeNode("s"); node* s = $1; addNodesList(s,$2); addSonNodeToFatherNode(&$$,s);}
    |var_type ':' variables {$$ = makeNode("s"); node* s = $1; addNodesList(s,$3); addSonNodeToFatherNode(&$$,s);}
    ;

variables:
    id 
    |id ',' variables {$$ = makeNode("s"); addSonNodeToFatherNode(&$$,$1); addNodesList($$,$3);}
    |id ASSIGN expression {$$ = makeNode("s"); node* s= makeNode("<-"); addSonNodeToFatherNode(&s,$1); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&$$,s);}
    |id ASSIGN expression ',' variables {$$ = makeNode("s"); node* s= makeNode("<-"); addSonNodeToFatherNode(&s,$1); addSonNodeToFatherNode(&s,$3); addSonNodeToFatherNode(&$$,s); addNodesList($$,$5);}
    |id ASSIGN func_call {$$ = makeNode("s"); node* s= makeNode("<-"); addSonNodeToFatherNode(&s,$1); addNodesList(s,$3); addSonNodeToFatherNode(&$$,s);}
    |id ASSIGN func_call ',' variables {$$ = makeNode("s"); node* s= makeNode("<-"); addSonNodeToFatherNode(&s,$1); addNodesList(s,$3); addSonNodeToFatherNode(&$$,s); addNodesList($$,$5);}
    ;

params:
    id ',' params {addSonNodeToFatherNode(&$1,$3);}
    |id 
    ;

string_id:
    id '[' expression ']' {$$ = $1; node* index = makeNode("INDEX"); addSonNodeToFatherNode(&index,$3); addSonNodeToFatherNode(&$$, index);}


id: 
    ID {$$ = makeNode(yytext); $$->node_type = "ID"; $$->line_number = number_line;}
    ;

pointer:
    MULTI expression {$$ = makeNode("pointer"); node* m = makeNode("*"); addSonNodeToFatherNode(&m, $2); addSonNodeToFatherNode(&$$,m);}

%%

/* Error handling function */
int yyerror(char* s){
    if(mainCounter == 1){
        printf("Error: The program cannot have more than one 'main' function.\n");
        exit(1);
    } else if(mainCounter == 0){
        printf("Error: The program must contain a 'main' function.\n");
        exit(1);
    }
    printf("%s: found line:%d token '%s'\n", s, number_line, yytext);
    return 0;
}

/* Main function */
int main(){
    return yyparse();
}