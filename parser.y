%{
#include "lex.yy.c"
#include "semantic.c"

int yylex(void);
int yyerror(char *s);

%}

%token PUBLIC PRIVATE VOID RETURN MAIN STATIC INT DOUBLE FLOAT VAR ARGS
%token ID INT_LITERAL CHAR BOOL INT_P CHAR_P DOUBLE_P FLOAT_P STRING BOOL_TRUE BOOL_FALSE NULL_P DOUBLE_LITERAL FLOAT_LITERAL CHAR_LITERAL HEX_LITERAL STRING_LITERAL 
%token IF ELSE WHILE FOR DO
%token BIGER PLUS ASSIGN ',' DIV AND EQUAL GE LOWER LE MINUS NOT NOTEQUAL OR MULTI ADDRESS LEN
%token ';' '{' '}' '(' ')' '[' ']' ':' 


%right ASSIGN NOT ';' MAIN
%left '{' '}' '(' ')'
%left AND OR
%left EQUAL BIGER GE LE LOWER NOTEQUAL 
%left MINUS PLUS
%left MULL MULTI DIV
%nonassoc PREC_IF
%nonassoc ELSE

%%
s:
    code {
        pushStatementToStack($1, 0);
        checkForSymbolsDuplications(SCOPE_STACK_TOP);
        findCalledFunctions($1);
        checkMainNonStaticCalls($1);
        checkStaticNonStaticCallsViolation();
        printTree ($1,0);
        }
    ;

code:
    functions Main {$$ = makeNode ("CODE"); addNodesList($$, $1); addSonNodeToFatherNode(&$$, $2);}
    | Main {$$ = makeNode ("CODE"); addSonNodeToFatherNode(&$$, $1);} 
    ;

Main:
    visibility VOID MAIN '(' args ')' ':' STATIC void_block {
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

var_type:  
    INT {$$ = makeNode ("INT");}
    |DOUBLE {$$ = makeNode ("DOUBLE");} 
    |FLOAT {$$ = makeNode ("FLOAT");} 
    |CHAR {$$ = makeNode ("CHAR");}
    |BOOL {$$ = makeNode ("BOOL");}
    |INT_P {$$ = makeNode ("INT*");}
    |DOUBLE_P {$$ = makeNode ("DOUBLE*");}
    |FLOAT_P {$$ = makeNode ("FLOAT*");}
    |CHAR_P {$$ = makeNode ("CHAR*");}

    ;   

function_void: 
    visibility VOID id '(' args ')' void_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("FUNCTION"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$5); 
    addSonNodeToFatherNode(&function, makeNode("VOID")); 
    addSonNodeToFatherNode(&function, $7); 
    addSonNodeToFatherNode(&$$,function);
    }

    |visibility VOID id '(' args ')' ':' STATIC void_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("STATIC"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$5); 
    addSonNodeToFatherNode(&function, makeNode("VOID")); 
    addSonNodeToFatherNode(&function, $9); 
    addSonNodeToFatherNode(&$$,function);
    }

    |visibility VOID id '(' ARGS args ')' void_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("FUNCTION"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$6); 
    addSonNodeToFatherNode(&function, makeNode("VOID")); 
    addSonNodeToFatherNode(&function, $8); 
    addSonNodeToFatherNode(&$$,function);
    }
    |visibility VOID id '(' ARGS args ')' ':' STATIC void_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("STATIC"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$6); 
    addSonNodeToFatherNode(&function, makeNode("VOID")); 
    addSonNodeToFatherNode(&function, $10); 
    addSonNodeToFatherNode(&$$,function);
    }
    ; 

function: 
    visibility func_type id '(' args ')' function_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("FUNCTION"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$5); 
    addSonNodeToFatherNode(&function, $2); 
    addSonNodeToFatherNode(&function, $7); 
    addSonNodeToFatherNode(&$$,function);
    }
    |visibility func_type id '(' args ')' ':' STATIC function_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("STATIC"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$5); 
    addSonNodeToFatherNode(&function, $2); 
    addSonNodeToFatherNode(&function, $9); 
    addSonNodeToFatherNode(&$$,function);
    }
    |visibility func_type id '(' ARGS args ')' function_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("FUNCTION"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$6); 
    addSonNodeToFatherNode(&function, $2); 
    addSonNodeToFatherNode(&function, $8); 
    addSonNodeToFatherNode(&$$,function);
    }
    |visibility func_type id '(' ARGS args ')' ':' STATIC function_block 
    {$$ = makeNode("s"); 
    node* function = makeNode ("STATIC"); 
    function->line_number = $3->line_number; 
    addSonNodeToFatherNode(&function,$3); 
    addSonNodeToFatherNode(&function,$6); 
    addSonNodeToFatherNode(&function, $2); 
    addSonNodeToFatherNode(&function, $10); 
    addSonNodeToFatherNode(&$$,function);
    }
    ;

visibility: 
    PUBLIC {$$ = makeNode ("PUBLIC");}
    | PRIVATE {$$ = makeNode ("PRIVATE");}
    ;

func_type:
    var_type
    |STRING {$$ = makeNode("STRING");}
    ;

 void_block:
    '{' '}' {$$ = makeNode ("BODY"); }
    |'{' declerations statments '}' 
    {
        $$ = makeNode ("BODY"); 
        node* var_node = makeNode("VAR"); 
        addNodesList(var_node,$2);
        addSonNodeToFatherNode(&$$,var_node); 
        addNodesList($$, $3);
    }
    |'{' statments '}' {$$ = makeNode ("BODY"); addNodesList($$,$2);}
    |'{' functions declerations statments '}' 
    {
        $$ = makeNode("BODY"); 
        addNodesList($$, $2); 
        node* var_node = makeNode("VAR"); 
        addNodesList(var_node,$3);
        addSonNodeToFatherNode(&$$,var_node);
        addNodesList($$,$4);
    }
    |'{' functions statments '}' {$$ = makeNode("BODY"); addNodesList($$, $2); addNodesList($$,$3);}
    |'{' functions '}' {$$ = makeNode("BODY"); addNodesList($$, $2);}
    |'{' declerations '}' 
    {
        $$ = makeNode ("BODY"); 
        node* var_node = makeNode("VAR"); 
        addNodesList(var_node,$2);
        addSonNodeToFatherNode(&$$,var_node);
    }
    ;
    
function_block:
    '{' RETURN expression ';' '}' 
    {
        $$ = makeNode ("BODY"); 
        node* ret = makeNode("RETURN"); 
        addSonNodeToFatherNode(&ret,$3); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' statments RETURN expression ';' '}' 
    {
        $$ = makeNode ("BODY"); 
        node* ret = makeNode("RETURN"); 
        addNodesList($$, $2); 
        addSonNodeToFatherNode(&ret,$4); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' functions statments RETURN expression ';' '}' 
    {
        $$ = makeNode("BODY"); 
        node* ret = makeNode("RETURN"); 
        addNodesList($$, $2); 
        addNodesList($$,$3); 
        addSonNodeToFatherNode(&ret,$5); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' declerations statments RETURN expression ';' '}' 
    {
        $$ = makeNode ("BODY");
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList(var_node, $2); 
        addSonNodeToFatherNode(&$$,var_node); 
        addNodesList($$, $3); 
        addSonNodeToFatherNode(&ret,$5); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' functions declerations statments RETURN expression ';' '}' 
    {
        $$ = makeNode("BODY");
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN");  
        addNodesList($$, $2);  
        addNodesList(var_node, $3); 
        addSonNodeToFatherNode(&$$,var_node); 
        addNodesList($$, $4); 
        addSonNodeToFatherNode(&ret,$6); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' declerations functions statments RETURN expression ';' '}' 
    {
        $$ = makeNode("BODY");
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList($$, $2);  
        addNodesList(var_node, $3); 
        addSonNodeToFatherNode(&$$,var_node); 
        addNodesList($$, $4); 
        addSonNodeToFatherNode(&ret,$6); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' declerations functions RETURN expression ';' '}' 
    {
        $$ = makeNode("BODY"); 
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList($$, $2);  
        addNodesList(var_node, $3); 
        addSonNodeToFatherNode(&$$,var_node); 
        addSonNodeToFatherNode(&ret,$5); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' functions RETURN expression ';' '}' 
    {
        $$ = makeNode("BODY"); 
        node* ret = makeNode("RETURN"); 
        addNodesList($$, $2); 
        addSonNodeToFatherNode(&ret,$4); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' declerations RETURN expression ';' '}' 
    {
        $$ = makeNode ("BODY"); 
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList(var_node,$2);
        addSonNodeToFatherNode(&$$,var_node); 
        addSonNodeToFatherNode(&ret,$4); 
        addSonNodeToFatherNode(&$$,ret);
    }
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
    |RETURN expression ';' 
    {
        $$ = makeNode("s"); 
        node* ret = makeNode("RETURN"); 
        addSonNodeToFatherNode(&ret, $2); 
        addSonNodeToFatherNode(&$$, ret);
    }
    |RETURN ';' {$$ = makeNode("s"); node* ret = makeNode("RETURN"); addSonNodeToFatherNode(&$$, ret);}
    ;

if_statment:
    IF '(' expression ')' statment 
    {
        $$ = makeNode("s"); 
        node* if_node = makeNode("IF"); 
        if_node->line_number = $3->line_number; 
        addSonNodeToFatherNode(&if_node, $3); 
        addNodesList(if_node,$5); 
        addSonNodeToFatherNode(&$$,if_node);
    }  %prec PREC_IF
    ;

if_else_statment:
    IF '(' expression ')' statment ELSE statment 
    {
        $$ = makeNode("s"); node* if_node = makeNode("IF-ELSE"); 
        if_node->line_number = $3->line_number; 
        addSonNodeToFatherNode(&if_node, $3); 
        addNodesList(if_node,$5); 
        addNodesList(if_node, $7); 
        addSonNodeToFatherNode(&$$,if_node);}
    ;

loop: 
    FOR '(' init_for ';' expression ';' init_for ')' statment 
    {
        $$ = makeNode("s"); 
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
        $$ = makeNode("s"); 
        node* while_node = makeNode("WHILE"); 
        while_node->line_number = $3->line_number; 
        addSonNodeToFatherNode(&while_node, $3); 
        addSonsNodesToFatherNode(while_node, $5); 
        addSonNodeToFatherNode(&$$,while_node);}
 
    | DO block WHILE '(' expression ')' ';' 
    {
        $$ = makeNode("s"); 
        node* while_node = makeNode("DO-WHILE"); 
        while_node->line_number = $5->line_number; 
        addSonsNodesToFatherNode(while_node, $2); 
        addSonNodeToFatherNode(&while_node, $5); 
        addSonNodeToFatherNode(&$$,while_node);
    }

    ;



init_for:
    id ASSIGN expression 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        ass_node->line_number = number_line; 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    | pointer ASSIGN expression 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        ass_node->line_number = number_line; 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    ;

ass_string:
    id '[' expression ']' ASSIGN expression 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        node* index = makeNode("INDEX"); 
        addSonNodeToFatherNode(&index,$3); 
        addSonNodeToFatherNode(&$1, index); 
        addSonNodeToFatherNode(&ass_node, $1); 
        addSonNodeToFatherNode(&ass_node,$6); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    ;

expression:
    expression PLUS expression {$$ = makeNode("+"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression MINUS expression {$$ = makeNode("-"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression MULTI expression {$$ = makeNode("*"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);} 
    | expression DIV expression {$$ = makeNode("/"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression BIGER expression { $$ = makeNode (">"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression GE expression { $$ = makeNode (">="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression LOWER expression { $$ = makeNode ("<"); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression LE expression { $$ = makeNode ("<="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
    | expression EQUAL expression { $$ = makeNode ("=="); $$->line_number = $1->line_number; addSonNodeToFatherNode(&$$,$1); addSonNodeToFatherNode(&$$, $3);}
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

pointer:
    MULTI expression {$$ = makeNode("POINTER"); node* pointer = makeNode("*"); addSonNodeToFatherNode(&pointer, $2); addSonNodeToFatherNode(&$$,pointer);}

func_call: 
    id '(' parameter_list ')' 
    {
        $$ = makeNode("s"); 
        node* fun_call = makeNode ("FUNCTION_CALL");  
        node* args = makeNode("ARGS"); 
        addSonNodeToFatherNode(&fun_call,$1); 
        args->line_number = $1->line_number; 
        addNodesList(args, $3); 
        addSonNodeToFatherNode(&fun_call, args); 
        addSonNodeToFatherNode(&$$,fun_call);
    }
    |id ASSIGN id '(' parameter_list ')' 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode ("<-"); 
        node* args = makeNode("ARGS"); 
        node* fun_call = makeNode("FUNCTION_CALL"); 
        ass_node->line_number = $3->line_number; 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&fun_call,$3); 
        args->line_number = $1->line_number; 
        addNodesList(args, $5); 
        addSonNodeToFatherNode(&fun_call, args); 
        addSonNodeToFatherNode(&ass_node, fun_call); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    |id '(' ')' 
    {
        $$ = makeNode("s"); 
        node* fun_call = makeNode ("FUNCTION_CALL"); 
        node* args = makeNode("WITHOT ARGS"); 
        addSonNodeToFatherNode(&fun_call,$1); 
        args->line_number = $1->line_number; 
        addSonNodeToFatherNode(&fun_call, args); 
        addSonNodeToFatherNode(&$$,fun_call);
    }
    |id ASSIGN id '(' ')' 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode ("<-"); 
        node* fun_call = makeNode("FUNCTION_CALL"); 
        node* args = makeNode("WITHOT ARGS"); 
        ass_node->line_number = $3->line_number; 
        args->line_number = $1->line_number; 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&fun_call,$3); 
        addSonNodeToFatherNode(&fun_call, args); 
        addSonNodeToFatherNode(&ass_node, fun_call); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
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
    |'{' statments '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        addNodesList(block,$2); 
        addSonNodeToFatherNode(&$$,block);
    }
    |'{' declerations '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        addNodesList(makeNode("VAR"),$2); 
        addSonNodeToFatherNode(&block,block); 
        addSonNodeToFatherNode(&$$,block);
    }
    |'{' declerations statments '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        node* var_node = makeNode("VAR"); 
        addNodesList(var_node,$2); 
        addSonNodeToFatherNode(&block,var_node); 
        addNodesList(block, $3); 
        addSonNodeToFatherNode(&$$,block);
    }    
    |'{' RETURN expression ';' '}' 
    {
        $$ = makeNode("BLOCK"); 
        node* ret = makeNode("RETURN"); 
        addSonNodeToFatherNode(&ret,$3); 
        addSonNodeToFatherNode(&$$,ret);
    }
    |'{' statments RETURN expression ';' '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        node* ret = makeNode("RETURN"); 
        addNodesList(block,$2);
        addSonNodeToFatherNode(&ret,$4); 
        addSonNodeToFatherNode(&block,ret); 
        addSonNodeToFatherNode(&$$,block);
    }
    |'{' declerations RETURN expression ';' '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList(var_node,$2); 
        addSonNodeToFatherNode(&block,var_node); 
        addSonNodeToFatherNode(&ret,$4); 
        addSonNodeToFatherNode(&block,ret); 
        addSonNodeToFatherNode(&$$,block);
    }
    |'{' declerations statments RETURN expression ';' '}' 
    {
        $$ = makeNode("s"); 
        node* block = makeNode("BLOCK"); 
        node* var_node = makeNode("VAR"); 
        node* ret = makeNode("RETURN"); 
        addNodesList(var_node,$2); 
        addSonNodeToFatherNode(&block,var_node); 
        addNodesList(block, $3); 
        addSonNodeToFatherNode(&ret,$5); 
        addSonNodeToFatherNode(&block,ret); 
        addSonNodeToFatherNode(&$$,block);
    }  
    ;   

 
args:
    {$$ = makeNode("WITHOT ARGS");}
    | more_args {$$ = makeNode("ARGS"); addNodesList($$,$1);}
    | more_args ';' args {$$ = makeNode("ARGS"); addNodesList($$,$1); addNodesList($$, $3);}
    ;

more_args:
    var_type params 
    {
        $$ = makeNode("s"); node* args_node = $1; addSonsNodesToFatherNode(args_node, $2); addSonNodeToFatherNode(&$$,args_node);
    }
    |var_type ':' params 
    {
        $$ = makeNode("s"); node* args_node = $1; addSonsNodesToFatherNode(args_node, $3); addSonNodeToFatherNode(&$$,args_node);}
    ;
    
params:
    id ',' params {addSonNodeToFatherNode(&$1,$3);}
    |id 
    ;


declerations:
    declerations var_decleration {$$ = combineNodes("fd",$1,$2);}
    |var_decleration
    |declerations string_decleration {$$ = combineNodes("fd",$1,$2);}
    |string_decleration
    ;

var_decleration:
    VAR vars ';' {$$ = $2;}
    ;

vars:
    var_type variables {$$ = makeNode("s"); node* var_node = $1; addNodesList(var_node,$2); addSonNodeToFatherNode(&$$,var_node);}
    |var_type ':' variables {$$ = makeNode("s"); node* var_node = $1; addNodesList(var_node,$3); addSonNodeToFatherNode(&$$,var_node);}
    ;

string_decleration:
    STRING str_args ';' 
    {
        $$ = makeNode("s"); node* str = makeNode("STRING"); addNodesList(str,$2); addSonNodeToFatherNode(&$$,str);}
    ;

str_args:
    id '[' expression ']' 
    {
        $$ = makeNode("s"); node* size = makeNode("SIZE"); 
        addSonNodeToFatherNode(&size,$3); 
        addSonNodeToFatherNode(&$1,size); 
        addSonNodeToFatherNode(&$$,$1);
    }

    |id '[' expression ']' ',' str_args 
    {
        $$ = makeNode("s"); 
        node* size = makeNode("SIZE"); 
        addSonNodeToFatherNode(&size,$3); 
        addSonNodeToFatherNode(&$1,size); 
        addSonNodeToFatherNode(&$$,$1);
         addNodesList($$,$6);
    }
    |id '[' expression ']' ASSIGN expression 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        node* size = makeNode("SIZE"); 
        addSonNodeToFatherNode(&size,$3); 
        addSonNodeToFatherNode(&$1,size); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$6);
        addSonNodeToFatherNode(&$$,ass_node);
    }
    |id '[' expression ']' ASSIGN expression ',' str_args 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        node* size = makeNode("SIZE"); 
        addSonNodeToFatherNode(&size,$3); 
        addSonNodeToFatherNode(&$1,size); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$6);
        addSonNodeToFatherNode(&$$,ass_node); 
        addNodesList($$,$8);
    }
    ; 




variables:
    id 
    |id ',' variables {$$ = makeNode("s"); addSonNodeToFatherNode(&$$,$1); addNodesList($$,$3);}
    |id ASSIGN expression 
    {
        $$ = makeNode("s"); 
        node* ass_node= makeNode("<-"); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    |id ASSIGN expression ',' variables 
    {
        $$ = makeNode("s"); 
        node* ass_node = makeNode("<-"); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addSonNodeToFatherNode(&ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node); 
        addNodesList($$,$5);
    }
    |id ASSIGN func_call 
    {
        $$ = makeNode("s"); 
        node* ass_node= makeNode("<-"); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addNodesList(ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node);
    }
    |id ASSIGN func_call ',' variables 
    {
        $$ = makeNode("s"); node* ass_node = makeNode("<-"); 
        addSonNodeToFatherNode(&ass_node,$1); 
        addNodesList(ass_node,$3); 
        addSonNodeToFatherNode(&$$,ass_node); 
        addNodesList($$,$5);
    }
    ;


string_id:
    id '[' expression ']' {$$ = $1; node* index = makeNode("INDEX"); addSonNodeToFatherNode(&index,$3); addSonNodeToFatherNode(&$$, index);}


id: 
    ID {$$ = makeNode(yytext); $$->node_type = "ID"; $$->line_number = number_line;}
    ;

%%


int yyerror(char* s){
    if(NUM_OF_MAIN_FUNCTIONS == 1){
        printf("Error need just one main function.\n");
    } else if(NUM_OF_MAIN_FUNCTIONS == 0){
        printf("Error must one main function.\n");
    }
    printf("%s: ERROR line %d - '%s'\n", s, number_line, yytext);
    exit(1);
    return 0;
}

int main(){
    return yyparse();
}