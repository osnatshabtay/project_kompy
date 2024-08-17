#ifndef SEMANTIC_HEADER
#define SEMANTIC_HEADER

#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node {
	char* token;
	struct node** sons_nodes;
	int sons_count;
	char* node_type;
	int line_number;
	char* var;
	char* code;
} node;

typedef struct symbol {
    int is_func;
    int is_static;
    char* id;
    char* type;
    node* params;
    struct symbol* next;
} symbol;

typedef struct scope{
	int scope_level;
	symbol *symbol_table;
	struct scope *next;
} scope;

node* makeNode(char* token);
node* combineNodes(char* token, node* first_node, node* second_node);

void addSonNodeToFatherNode(node** father, node* son);
void addSonsNodesToFatherNode(node* father, node* son);
void addNodesList(node* add_to, node* to_add);
void printTree(node* curr_node, int num_of_spaces);
void freeNode(node* node_to_free, int free_sons);

symbol* symbolSearch (symbol* symbol_table, char* id);
symbol* scopeSearch(char* id);

void pushStatementToStack(node* root, int scope_level);
void pushScopeToScopeStack(scope** scope_stack_top, node* params, node** statments, int scope_level, int stat_size);
void pushStatementsToScope(node** statements, int size);
void pushSymbolsToSymbolTable(node* var_declaration_nosde);
void addSymbolToSymbolTable(scope** scope_stack_top, node* params, char* symbol_id, char* symbol_type, int is_func, int is_static);
void isValidPrtAssinment(node* ptr_node);
void checkForSymbolsDuplications(scope* scope_node);
void checkStaticNonStaticCallsViolation();
void checkMainNonStaticCalls(node* tree);
void checkStringAssignment(node* str_node, char* assigned_val_type);
void findCalledFunctions(node* astNode);

int isVarDeclared(char* var_name);
int isValidReturnType(node *func_node);
int isValidReturnStatement(node* func_node, char* expected_ret_type);
int isArithmeticType(char* type);
int isCompatibleForComparison(char* left, char* right);
int isEqualType(char* left, char* right);
int checkFunctionArgs(node* func_params, node* func_args);
int checkFunctionCall(node* func_args, char* func_name);

char* getPointerBaseType(char* type);
char* getArithmeticResultType(node* exp, char* left, char* right);
char* checkExpAndReturnItsType(node* exp);

#endif 