#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

/* Define node structure for the abstract syntax tree (AST) - Part 1 */
typedef struct node {
	char* token;
	int count;
	struct node** nodes;
	char* node_type;
	int line;
	char* father;
} node;

/* Define Symonl Table structures - Part 2 */
typedef struct symbolNode {
    int isFunc;
    int isStatic;
    char* id;
    char* type;
    char* data;
    int scopeID;
    node* params;
    struct symbolNode* next;
} symbolNode;

typedef struct scopeNode{
	char* scopeName;
	int scopeNum;
	int scopeLevel;
	symbolNode *symbolTable;
	struct scopeNode *next;
} scopeNode;

/* Functions declarations */
/* AST - Part 1 */
node* makeNode(char* token);
void addSonNodeToFatherNode(node** father, node* son);
void printTree(node* tree, int tab);
void addSonsToFatherNode(node* father, node* son);
node* combineNodes(char* token, node* one, node* two);
void addList(node* root, node* arr);

/* Semantic Check - Part 2 */
void semanticAnalysis(node* root);
void pushStat(node* root, int level);
void pushScopeToStack(scopeNode** topStack, char* name, node* params, node** statments, int level, int stat_size);
void pushScopeStatements(node** statements, int size);
void pushSymbols(node* decleration);
void pushNodesToSymtable(char* type, node** vars, int size);
void pushToTable(scopeNode** top, char* id, char* type, char* data, int isFunc, node* params, int isStatic);
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
void setFatherForStatements(node** statements, int size, char* fatherName);
void setFatherForChildNodes(node* root, char* fatherName);
void printScopes(struct scopeNode *node);
void printSymbolTable(struct scopeNode *node);
void printInfo(node *root);

#endif 