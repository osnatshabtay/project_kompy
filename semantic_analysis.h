#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

// TODO - chage param order
// TODO - change all varNames to var_names
// TODO - pretiffy order of function declaration

/* Define node structure for the abstract syntax tree (AST) - Part 1 */
typedef struct node {
	char* token;
	char* father;
	struct node** nodes; // TODO cahnge names (prob sons)
	int count; // TODO change name (prob num_of_sons)
	char* node_type;
	int line; // TODO change to line_num
} node;

/* Define Symonl Table structures - Part 2 */
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

/* Functions declarations */
/* AST - Part 1 */
node* makeNode(char* token);
node* combineNodes(char* token, node* first_node, node* second_node);

void addSonNodeToFatherNode(node** father, node* son);
void addSonsNodesToFatherNode(node* father, node* son);
void addNodesList(node* add_to, node* to_add);
void printTree(node* curr_node, int num_of_spaces);
void freeNode(node* node_to_free, int free_sons);

/* Semantic Check - Part 2 */
void semanticAnalysis(node* root);
void pushStatementToStack(node* root, int scope_level);
void pushScopeToScopeStack(scopeNode** scope_stack_top, node* params, node** statments, int scope_level, int stat_size);
void pushScopeStatements(node** statements, int size);
void pushSymbols(node* var_declaration_nosde);
void addSymbolToSymbolTable(scopeNode** scope_stack_top, char* symbol_id, char* symbol_type, char* data, int is_func, int is_static, node* params);
int isDeclared(char* id);
symbolNode* symbolSearch (symbolNode* symTable, char* id);
symbolNode* scopeSearch(char* id);
char* evaluateExpression(node* exp);
int checkFuncReturn(node *funcNode);
int evalReturn(node* funcNode, char* type);
void evalPtr(node* ptrNode);
void isSymbolExist(scopeNode* scope);
void checkSymbols(scopeNode* scope);
int checkFunctionCall(char *func_name, node *args);
void checkStaticNonStaticCalls();
void checkMainNonStaticCalls(node* tree);
int checkFunctionArgs(node* params, node* callArgs);
void checkString(node* strNode, char* assType);
void findFunctionsCalled(node* astNode);
void printSymbolTable(struct scopeNode *node);

#endif 