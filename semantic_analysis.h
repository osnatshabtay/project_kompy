#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

// TODO - chage param order
// TODO change all strings (for example - "ARGS_NONE")

typedef struct node {
	char* token;
	char* father;
	struct node** nodes; // TODO cahnge names (prob sons)
	int count; // TODO change name (prob num_of_sons)
	char* node_type;
	int line_number;
} node;

typedef struct symbolNode {
    int is_func;
    int is_static;
    char* id;
    char* type;
    char* data;
    int scopeID;
    node* params;
    struct symbolNode* next;
} symbolNode;

typedef struct scopeNode{
	int scopeLevel;
	symbolNode *symbolTable;
	struct scopeNode *next;
} scopeNode;

node* makeNode(char* token);
node* combineNodes(char* token, node* first_node, node* second_node);

void addSonNodeToFatherNode(node** father, node* son);
void addSonsNodesToFatherNode(node* father, node* son);
void addNodesList(node* add_to, node* to_add);
void printTree(node* curr_node, int num_of_spaces);
void freeNode(node* node_to_free, int free_sons);

/* Semantic Check - Part 2 */
symbolNode* symbolSearch (symbolNode* symbol_table, char* id);
symbolNode* scopeSearch(char* id);

void semanticAnalysis(node* root);
void pushStatementToStack(node* root, int scope_level);
void pushScopeToScopeStack(scopeNode** scope_stack_top, node* params, node** statments, int scope_level, int stat_size);
void pushStatementsToScope(node** statements, int size);
void pushSymbolsToSymbolTable(node* var_declaration_nosde);
void addSymbolToSymbolTable(scopeNode** scope_stack_top, char* symbol_id, char* symbol_type, char* data, int is_func, int is_static, node* params);
void isValidPrtAssinment(node* ptr_node);
void checkForSymbolsDuplications(scopeNode* scope);
void checkStaticNonStaticCallsViolation();
void checkMainNonStaticCalls(node* tree);
void checkStringAssignment(node* str_node, char* assigned_val_type);
void findCalledFunctions(node* astNode);

int isVarDeclared(char* var_name);
int checkFunctionArgs(node* func_params, node* func_args);
int isValidReturnType(node *func_node);
int isValidReturnStatement(node* func_node, char* expected_ret_type);
int checkFunctionCall(char* func_name, node* func_args);
int isArithmeticType(char* type);
int isCompatibleForComparison(char* left, char* right);
int isEqualType(char* left, char* right);

char* getPointerBaseType(char* type);
char* getArithmeticResultType(char* left, char* right, node* exp);
char* checkExpAndReturnItsType(node* exp);

#endif 