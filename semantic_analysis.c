#include "semantic_analysis.h"
#include <stddef.h>
#include <ctype.h>

/* Global variables */
scopeNode* topStack = NULL;
int GlobalScope = 0;
int errorsCounter = 0;
int mainCounter = 0;
char* currentFunction = NULL;
char* func_called[256];
char* func_has_been_called[256];
int func_called_index = 0;
int func_has_been_called_index = 0;

/* Functions definitions */
/* AST - Part 1 */

/**
 * makeNode - Creates a new node with the specified token.
 * @param token: The token to assign to the new node.
 * @return A pointer to the newly created node.
 */
node* makeNode(char* token) {
	node *newnode = (node*)malloc(sizeof(node));
	char *newstr = (char*)malloc(sizeof(token) + 1);
	strcpy(newstr, token);
	newnode->token = newstr;
	newnode->nodes = NULL;
	newnode->node_type = NULL;
	newnode->father = NULL;
	newnode->count = 0;
	newnode->line = 0;
	return newnode;
}

/**
 * addSonNodeToFatherNode - Adds a son node to the father node.
 * @param father: The father node to which the son will be added.
 * @param son: The son node to add to the father.
 */
void addSonNodeToFatherNode(node** father, node* son) {
	if ((*father)->count == 0) {
		(*father)->nodes = (node**)malloc(sizeof(node*));
		(*father)->nodes[(*father)->count] = son;
		(*father)->count++;
	}
	else {
		(*father)->nodes = (node**)realloc((*father)->nodes, ((*father)->count+1)*sizeof(node*));
		(*father)->nodes[(*father)->count] = son;
		(*father)->count++;
	}
}

/**
 * printTree - Recursively prints the tree structure.
 * @param tree: The root node of the tree to print.
 * @param tab: The current indentation level for printing.
 */
void printTree(node* tree, int tab) {
	int i;
	char* token = tree->token;

	if (*token) {
		for (i = 0; i < tab; i++) {
			printf("\t");
		}
		printf("(%s\n", token);
	}
	else
		tab -= 1;
	if (tree->nodes) {
		for (int j = 0; j < tree->count; j++) {
			printTree(tree->nodes[j], tab + 1);
		}
	}
	for (i = 0; i < tab; i++) {
		printf("\t");
	}
	if (*token)
		printf(")\n");
}

/**
 * addSonsToFatherNode - Creates a list of nodes from a son node and adds them to the father node.
 * @param father: The father node to which the list will be added.
 * @param son: The son node from which the list is derived.
 */
void addSonsToFatherNode(node* father, node* son) {
	node* temp = son;
	addSonNodeToFatherNode(&father, makeNode(son->token));
	while (temp->nodes) {
		temp = temp->nodes[0];
		addSonNodeToFatherNode(&father, makeNode(temp->token));
	}
}

/**
 * combineNodes - Combines two nodes into a new node with the specified token.
 * @param token: The token for the new combined node.
 * @param one: The first node to combine.
 * @param two: The second node to combine.
 * @return A pointer to the newly created node.
 */
node* combineNodes(char* token, node* one, node* two) {
	int i = 0, j = 0;
	node* new = (node*)malloc(sizeof(node));
	new->token = strdup(token);
	new->count = one->count + two->count;
	if (one->count == 0)
		new->count++;
	if (two->count == 0)
		new->count++;
	new->nodes = (node**)malloc(sizeof(node*) * new->count);
	if (one->count == 0)
		new->nodes[j++] = one;
	else {
		for (j, i = 0; i < one->count; j++, i++) {
			new->nodes[j] = one->nodes[i];
		}
		free(one);
	}
	if (two->count == 0)
		new->nodes[j++] = two;
	else {
		for (j, i = 0; i < two->count; j++, i++) {
			new->nodes[j] = two->nodes[i];
		}
		free(two);
	}
	return new;
}

/**
 * addList - Adds a list of nodes to the root node.
 * @param root: The root node to which the list will be added.
 * @param arr: The array of nodes to add to the root.
 */
void addList(node* root, node* arr){
	if (arr->count == 0){
		addSonNodeToFatherNode(&root, arr);
	}
	else{
		for (int i =0; i< arr->count;i++){
			addSonNodeToFatherNode(&root,arr->nodes[i]);
		}
	}
}

/* Semantic Check - Part 2 */

/**
 * semanticAnalysis - Conducts semantic analysis on the given root node.
 * @param root: The root node of the abstract syntax tree.
 */
void semanticAnalysis(node* root) {
	pushStat(root, 0);
	isSymbolExist(topStack);
	findFunctionsCalled(root);
	if (errorsCounter > 0)
		printf("%d Errors found\n\n",errorsCounter);
	else {
		checkMainNonStaticCalls(root);
		checkStaticNonStaticCalls();
		printInfo(root);
	}
}

/**
 * pushStat - Pushes a statement node onto the stack.
 * @param root: The statement node to push.
 * @param level: The current scope level.
 */
void pushStat(node* root, int level){
	if (root == NULL)
		return;
	else if (!strcmp(root->token, "CODE")){
		level++;
        pushScopeToStack(&topStack, "GLOBAL" ,NULL, root->nodes, level, root->count);
	}
	else if(!strcmp(root->token, "MAIN")) {
		level++;
		mainCounter++;
		pushScopeToStack(&topStack, "MAIN", NULL, root->nodes[0]->nodes, level, root->nodes[0]->count);
	}
	else if(!strcmp(root->token,"FUNCTION")){
        level++;
        pushScopeToStack(&topStack,"FUNCTION",root->nodes[1], root->nodes[3]->nodes, level, root->nodes[3]->count);
        checkFuncReturn(root);
	}
	else if(!strcmp(root->token,"STATIC-FN")){
        level++;
        pushScopeToStack(&topStack,"STATIC-FN",root->nodes[1], root->nodes[3]->nodes, level, root->nodes[3]->count);
        checkFuncReturn(root);
	}

	else if (!strcmp(root->token, "IF")){
		level++;
		pushScopeToStack(&topStack, "IF" ,NULL, root->nodes[1]->nodes, level, root->nodes[1]->count);
		char* evalType = evaluateExpression(root->nodes[0]);
		if(strcmp("BOOL" ,evalType)){
			errorsCounter++;
			printf("Error: Line %d: Invalid IF condition. Expected return type 'BOOL'.\n",root->line);
		}
	}
	else if (!strcmp(root->token, "WHILE")){
		level++;
		pushScopeToStack(&topStack, "WHILE" ,NULL, root->nodes[1]->nodes, level, root->nodes[1]->count);
		char* evalType = evaluateExpression(root->nodes[0]);
		if(strcmp("BOOL" ,evalType)){
			errorsCounter++;
			printf("Error: Line %d: Invalid WHILE condition. Expected return type 'BOOL'.\n", root->line);
		}
	}
	else if (!strcmp(root->token, "DO-WHILE")){
		level++;
		pushScopeToStack(&topStack, "DO-WHILE" ,NULL, root->nodes[0]->nodes, level, root->nodes[0]->count);
		char* evalType = evaluateExpression(root->nodes[1]);
		if(strcmp("BOOL" ,evalType)){
			errorsCounter++;
			printf("Error: Line %d: Invalid DO-WHILE condition. Expected return type 'BOOL'.\n", root->line);
		}
	}
	else if (!strcmp(root->token, "FOR")){
		level++;
		pushScopeToStack(&topStack, "FOR" ,NULL, root->nodes[3]->nodes, level, root->nodes[3]->count);
		char* initType = evaluateExpression(root->nodes[0]);
		if(strcmp("INT" ,initType)){
			errorsCounter++;
			printf("Error: Line %d: FOR initialization requires an 'INT' type.\n", root->nodes[0]->line);
		}
		char* evalType = evaluateExpression(root->nodes[1]);
		if(strcmp("BOOL" ,evalType)){
			errorsCounter++;
			printf("Error: Line %d: FOR-condition requires return type 'BOOL'.\n", root->nodes[0]->line);
		}
		char* incType = evaluateExpression(root->nodes[2]);
		if(strcmp("INT" ,incType)){
			errorsCounter++;
			printf("Error: Line %d: FOR-increment requires return type 'INT'.\n", root->nodes[0]->line);
		}
	}
	else if (!strcmp(root->token, "BLOCK") && root->father == NULL){
		level++;
		pushScopeToStack(&topStack, "BLOCK" ,NULL, root->nodes, level, root->count);
	}

	for (int i = 0; i < root->count; i++){
		pushStat(root->nodes[i], level);
	}
}

/**
 * pushScopeToStack - Pushes a new scope onto the scope stack.
 * @param top: The top of the scope stack.
 * @param name: The name of the new scope.
 * @param params: The parameters of the scope, if any.
 * @param statements: The statements in the scope.
 * @param level: The scope level.
 * @param stat_size: The number of statements in the scope.
 */
void pushScopeToStack(scopeNode** top, char* name, node* params, node** statements, int level, int stat_size){      
	scopeNode* new_scope = (scopeNode*) malloc(sizeof(scopeNode));
	new_scope->scopeName = (char*)(malloc (sizeof(name) + 1));
	strncpy(new_scope->scopeName, name, sizeof(name)+1);
	GlobalScope++;
	new_scope->scopeNum = GlobalScope;
	new_scope->scopeLevel=level-1;
	new_scope->next = (*top);
	(*top) = new_scope;
	if (params){
		pushSymbols(params);
	}
	setFatherForStatements(statements, stat_size, name); // New function to set 'father'
	pushScopeStatements(statements, stat_size);
}

/**
 * pushScopeStatements - Pushes statements of a scope onto the scope stack.
 * @param statements: The statements to push.
 * @param size: The number of statements.
 */
void pushScopeStatements(node** statements, int size){
	for (int i = 0;i < size;i++){
		if(!strcmp(statements[i]->token, "VAR")){
			pushSymbols(statements[i]);
		}

		else if (!strcmp(statements[i]->token, "FUNCTION")){
			pushToTable(&topStack,statements[i]->nodes[0]->token, statements[i]->nodes[2]->token,NULL,1, statements[i]->nodes[1], 0);
		}

		else if (!strcmp(statements[i]->token, "STATIC-FN")) {
			pushToTable(&topStack, statements[i]->nodes[0]->token, statements[i]->nodes[2]->token, NULL, 1, statements[i]->nodes[1], 1);
		}

		else if (!strcmp(statements[i]->token, "FUNC_CALL")){
			checkFunctionCall(statements[i]->nodes[0]->token, statements[i]->nodes[1]);
		}

		else if(!strcmp(statements[i]->token, "<-")){
        	if (isDeclared(statements[i]->nodes[0]->token) && strcmp(statements[i]->nodes[0]->token,"PTR")){
                char *left = scopeSearch(statements[i]->nodes[0]->token)->type;
                char *right = evaluateExpression(statements[i]->nodes[1]);
				if (!strcmp(left, "STRING"))
					checkString(statements[i], right);
				else if (strcmp(left, "STRING") && statements[i]->nodes[0]->count > 0){
					errorsCounter++;
					printf("Error: Line %d: %s cannot have index.\n", statements[i]->nodes[0]->line, left);
				}
				else if (!strcmp(right, "NULL") && (strcmp(left, "INT*") && strcmp(left, "CHAR*") && strcmp(left, "DOUBLE*")) && strcmp(left, "FLOAT*")){
					errorsCounter++;
					printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", statements[i]->line, right, left);
				}
            	else if (strcmp(right,left) && strcmp(right,"undefined")){
					errorsCounter++;
                    printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", statements[i]->line, right, left);
				}
			}
			else if (!strcmp(statements[i]->nodes[0]->token,"PTR") && isDeclared(statements[i]->nodes[0]->nodes[0]->nodes[0]->token))
				evalPtr(statements[i]);
			else{
				evaluateExpression(statements[i]->nodes[1]);
				errorsCounter++;
				printf("Error: Line %d: Undeclared variable [%s]\n", statements[i]->line, statements[i]->nodes[0]->token);
			}
		}
	}
}

/**
 * pushSymbols - Pushes symbol nodes from a declaration node.
 * @param decleration: The declaration node containing symbols.
 */
void pushSymbols(node* decleration){
	for(int i = 0; i<decleration->count;i++){
		pushNodesToSymtable(decleration->nodes[i]->token, decleration->nodes[i]->nodes, decleration->nodes[i]->count);
	}
}

/**
 * pushNodesToSymtable - Adds nodes to the symbol table.
 * @param type: The type of the variables.
 * @param vars: The variables to add.
 * @param size: The number of variables.
 */
void pushNodesToSymtable(char* type, node** vars, int size){
	for(int i = 0;i < size; i++){
		if (strcmp(vars[i]->token, "<-") && strcmp(type, "STRING") == 0){
			char* evalType = evaluateExpression(vars[i]->nodes[0]->nodes[0]);
			if(strcmp("INT", evalType)){
				errorsCounter++;
				printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", vars[i]->line, evalType);
			}
			else
				pushToTable(&topStack, vars[i]->token, type, NULL, 0, NULL, 0);
		}
		else if ((!strcmp(vars[i]->token, "<-") && strcmp(type, "STRING") == 0)){
			char* evalType = evaluateExpression(vars[i]->nodes[0]->nodes[0]->nodes[0]);
			if(strcmp("INT", evalType)){
				errorsCounter++;
				printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", vars[i]->nodes[0]->line, evalType);
			}
			else {
				evalType = evaluateExpression(vars[i]->nodes[1]);
				if (!strcmp(type,evalType))
					pushToTable(&topStack, vars[i]->nodes[0]->token, type, vars[i]->nodes[1]->token, 0, NULL, 0);
				else{
					errorsCounter++;
					printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", vars[i]->nodes[0]->line, evalType, type);
				}
			}
		}

		else if ((!strcmp(vars[i]->token, "<-") && vars[i]->nodes[1]->node_type != NULL && !strcmp("NULL", vars[i]->nodes[1]->node_type)))
			if (strcmp(type, "INT*") && strcmp(type, "CHAR*") && strcmp(type, "DOUBLE*") && strcmp(type, "FLOAT*")){
				errorsCounter++;
				printf("Error: Line %d: Assignment type mismatch: can not assign NULL to %s\n", vars[i]->line, type);
			}
			else
				pushToTable(&topStack, vars[i]->nodes[0]->token, type, vars[i]->nodes[1]->token, 0, NULL, 0);
		else{
			if (strcmp(vars[i]->token, "<-"))
				pushToTable(&topStack, vars[i]->token, type, NULL, 0, NULL, 0);
			else{
				char* evalType = evaluateExpression(vars[i]->nodes[1]);
				if (!strcmp(type,evalType))
					pushToTable(&topStack, vars[i]->nodes[0]->token, type, vars[i]->nodes[1]->token, 0, NULL, 0);
				else {
					errorsCounter++;
					printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", vars[i]->nodes[0]->line, evalType, type);
				}
			}	
		}
	}
}

/**
 * pushToTable - Adds a symbol to the symbol table.
 * @param top: The top of the scope stack.
 * @param id: The identifier of the symbol.
 * @param type: The type of the symbol.
 * @param data: Additional data for the symbol.
 * @param isFunc: Indicates if the symbol is a function.
 * @param params: The parameters of the function, if any.
 * @param isStatic: Indicates if the function is static.
 */
void pushToTable(scopeNode** top, char* id, char* type, char* data, int isFunc, node* params, int isStatic) {
	symbolNode* new_node = (symbolNode*) malloc(sizeof(symbolNode));
	new_node->isFunc = isFunc;
	new_node->isStatic = isStatic;
	new_node->id = (char*)(malloc (sizeof(id) + 1));
	strncpy(new_node->id, id, sizeof(id)+1);
	if (data != NULL) {
		new_node->data = (char*)(malloc (sizeof(data) + 1));
		strncpy(new_node->data, data, sizeof(data)+1);
	}
	else
		new_node->data = NULL;
	new_node->type = (char*)(malloc (sizeof(type) + 1));
	strncpy(new_node->type, type, sizeof(type)+1);
	new_node->scopeID=(*top)->scopeNum;
	if (params != NULL) {
		new_node->params = (node*)(malloc(sizeof(node)));
		memcpy(new_node->params, params, sizeof(node));
	}
	else
		new_node->params = NULL;
	new_node->next =(*top)->symbolTable;
	(*top)->symbolTable = new_node;
}

/**
 * isDeclared - Checks if a symbol with the given ID is declared in the current scope or any parent scopes.
 * @param id: The identifier of the symbol to check.
 * @return: Returns 1 if the symbol is declared, 0 otherwise.
 */
int isDeclared(char* id){
	symbolNode *symbol = scopeSearch(id);
	if (symbol != NULL)
		return 1;
	return 0;
}

/**
 * symbolSearch - Searches for a symbol with the given ID in the specified symbol table.
 * @param symTable: The symbol table to search within.
 * @param id: The identifier of the symbol to search for.
 * @return: Returns a pointer to the symbolNode if the symbol is found, NULL otherwise.
 */
symbolNode* symbolSearch (symbolNode* symTable, char* id){
    symbolNode* current = symTable;
    while (current != NULL)
    {
        if (!strcmp(current->id, id)){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * scopeSearch - Searches for a symbol with the given ID in the current scope and all parent scopes.
 * @param id: The identifier of the symbol to search for.
 * @return: Returns a pointer to the symbolNode if the symbol is found, NULL otherwise.
 */
symbolNode* scopeSearch(char* id){
	scopeNode* current = topStack;
	symbolNode* res;
	int currLevel;
	while (current != NULL){
		currLevel = current->scopeLevel;
		res = symbolSearch(current->symbolTable, id);
		if (res != NULL)
			return res;
		if (currLevel == 0)
			return NULL;
		
		while (current->scopeLevel > 0 && current->scopeLevel >= currLevel)
			current = current->next;
	}
	return NULL;
}

/**
 * evaluateExpression - Evaluates an expression node in the AST and returns the data type of the result.
 * This function performs semantic analysis on expressions, checking types, and ensuring compatibility.
 * It handles various cases, such as identifiers, function calls, arithmetic operations, comparison operators, logical operators, pointer dereferencing, and more.
 * Errors are reported for type mismatches, undeclared variables, and other semantic issues.
 * @param exp: The AST node representing the expression to evaluate.
 * @return: The data type of the expression result as a string. Returns "NULL" for null expressions or errors, "BOOL" for boolean results, and specific types like "INT", "DOUBLE", "FLOAT", "CHAR", or pointer types.
 */
char* evaluateExpression(node* exp){
	if (exp->node_type != NULL && !strcmp(exp->node_type, "ID")){
		symbolNode* node = scopeSearch(exp->token);
		if(node != NULL){
			if(!strcmp(node->type, "STRING") && exp->count > 0){
				char* indexType = evaluateExpression(exp->nodes[0]->nodes[0]);
				if(strcmp("INT", indexType)){
					errorsCounter++;
					printf("Error: Line %d: Size of string must be type 'INT' not '%s'.\n", exp->line ,indexType);
					return "NULL";
				}
			return "CHAR";
			}
			return node->type;
		}
        else{
			errorsCounter++;
			printf("Error: Line %d: Undeclared variable '%s'.\n", exp->line, exp->token);
            return "undefined";
		}
	}

	else if (exp->node_type != NULL && !strcmp(exp->node_type, "NULL"))
		return "NULL";

	else if (exp->node_type != NULL)
		return exp->node_type;

	else if (!strcmp(exp->token, "FUNC_CALL")){
		symbolNode *funcSymbol;
		if(checkFunctionCall(exp->nodes[0]->token, exp->nodes[1])){
			funcSymbol = scopeSearch(exp->nodes[0]->token);
			return funcSymbol->type;
		}
	}

	else if (!strcmp(exp->token,"+")||!strcmp(exp->token,"-")||!strcmp(exp->token,"*")||!strcmp(exp->token,"/")){
		char* left, *right;
        left = evaluateExpression(exp->nodes[0]);
        right = evaluateExpression(exp->nodes[1]);
		if (!strcmp(left, "NULL") || !strcmp(right,"NULL"))
			return "NULL";
        if (!strcmp(left,"INT") && !strcmp(right,"INT"))
			return "INT";
		else if (!strcmp(left,"DOUBLE") && !strcmp(right,"DOUBLE"))
			return "DOUBLE";
		else if (!strcmp(left,"FLOAT") && !strcmp(right,"FLOAT"))
			return "FLOAT";
		else if ((!strcmp(left,"DOUBLE") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"DOUBLE")))
			return "DOUBLE";
		else if ((!strcmp(left,"FLOAT") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"FLOAT")))
			return "FLOAT";
		else if (((!strcmp(left,"INT*") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"INT*"))) && strcmp(exp->token,"*") && strcmp(exp->token,"/"))
			return "INT*";
		else if (((!strcmp(left,"CHAR*") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"CHAR*"))) && strcmp(exp->token,"*") && strcmp(exp->token,"/"))
			return "CHAR*";
		else if (((!strcmp(left,"DOUBLE*") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"DOUBLE*"))) && strcmp(exp->token,"*") && strcmp(exp->token,"/"))
			return "DOUBLE*";
		else if (((!strcmp(left,"FLOAT*") && !strcmp(right,"INT")) || (!strcmp(left,"INT") && !strcmp(right,"FLOAT*"))) && strcmp(exp->token,"*") && strcmp(exp->token,"/"))
			return "FLOAT*";
		else {
			errorsCounter++;
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line, exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
		}
	}

	else if(!strcmp(exp->token,">")||!strcmp(exp->token,"<")||!strcmp(exp->token,">=")||!strcmp(exp->token,"<=")){
        char* left, *right;
        left = evaluateExpression(exp->nodes[0]);
        right = evaluateExpression(exp->nodes[1]);
        if((!strcmp(left,"INT") && !strcmp(right,"INT")) || (!strcmp(left,"DOUBLE") && !strcmp(right,"DOUBLE")) || (!strcmp(left,"FLOAT") && !strcmp(right,"FLOAT")))
            return "BOOL";
		else{
			errorsCounter++;
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line,exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
		}
	}

    else if(!strcmp(exp->token,"&&")||!strcmp(exp->token,"||")){
        char* left, *right;
        left = evaluateExpression(exp->nodes[0]);
        right = evaluateExpression(exp->nodes[1]);
        if(!strcmp(left,"BOOL") && !strcmp(right,"BOOL"))
            return "BOOL";
		else {
			errorsCounter++;
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line, exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
		}
	}

	else if(!strcmp(exp->token,"==")||!strcmp(exp->token,"!=")){
        char* left, *right;
        left = evaluateExpression(exp->nodes[0]);
        right = evaluateExpression(exp->nodes[1]);
        if(!strcmp(left,"INT")&&!strcmp(right,"INT"))
            return "BOOL";
        else if(!strcmp(left,"BOOL")&&!strcmp(right,"BOOL"))
            return "BOOL";
        else if(!strcmp(left,"CHAR")&&!strcmp(right,"CHAR"))
            return "BOOL";  
        else if(!strcmp(left,"DOUBLE")&&!strcmp(right,"DOUBLE"))
            return "BOOL";
		else if(!strcmp(left,"FLOAT")&&!strcmp(right,"FLOAT"))
            return "BOOL";
        else if(!strcmp(left,"INT*")&&!strcmp(right,"INT*"))
            return "BOOL";
		else if(!strcmp(left,"CHAR*")&&!strcmp(right,"CHAR*"))
			return "BOOL";
		else if(!strcmp(left,"DOUBLE*")&&!strcmp(right,"DOUBLE*"))
			return "BOOL";
		else if(!strcmp(left,"FLOAT*")&&!strcmp(right,"FLOAT*"))
			return "BOOL";
		else{
			errorsCounter++;
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line ,exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
		}
	}

	else if(!strcmp(exp->token,"NOT")){
        char* left;
        left = evaluateExpression(exp->nodes[0]);
        if(!strcmp(left,"BOOL"))
            return "BOOL";
		else{
			errorsCounter++;
			printf("Error: Line %d: Cannot perform ! on '%s' - [!%s]\n", exp->line ,left,exp->nodes[0]->token);
		}
	}

	else if(!strcmp(exp->token,"PTR")){
        char* left;
        left = evaluateExpression(exp->nodes[0]->nodes[0]);
        if(!strcmp(left,"INT*"))
            return "INT";
		else if(!strcmp(left,"CHAR*"))
            return "CHAR";
		else if(!strcmp(left,"DOUBLE*"))
            return "DOUBLE";
		else if(!strcmp(left,"FLOAT*"))
            return "FLOAT";
		else{
			errorsCounter++;
			printf("Error: Line %d: '%s' is not pointer\n", exp->nodes[0]->nodes[0]->line ,left);
		}
	}

	else if (!strcmp(exp->token,"LEN")){
		char* left;
        left = evaluateExpression(exp->nodes[0]);
		if(!strcmp(left,"STRING"))
            return "INT";
		else{
			errorsCounter++;
			printf("Error: Line %d: Cannot perform || on '%s' - [|%s|]\n", exp->nodes[0]->line, left,exp->nodes[0]->token);
		}
	}

	else if (!strcmp(exp->token,"&")){
		char* left;
        left = evaluateExpression(exp->nodes[0]);
		if(!strcmp(left,"INT"))
            return "INT*";
		else if(!strcmp(left,"CHAR"))
            return "CHAR*";
		else if(!strcmp(left,"DOUBLE"))
            return "DOUBLE*";
		else if(!strcmp(left,"FLOAT"))
            return "FLOAT*";
		else{
			errorsCounter++;
			printf("Error: Line %d: Cannot perform '&' on '%s' - [&%s]\n", exp->nodes[0]->line, left,exp->nodes[0]->token);
		}
	}

	else if(!strcmp(exp->token, "<-")){
        	if (isDeclared(exp->nodes[0]->token)){
                char *left = scopeSearch(exp->nodes[0]->token)->type;
                char *right = evaluateExpression(exp->nodes[1]);
            	if (strcmp(right,left) && strcmp(right,"NULL")){
					errorsCounter++;
                    printf("Error: Line %d: Assignment type mismatch: cannot assign '%s' to '%s'.\n", exp->line, right, left);
				}
				else
					return left;
			}
		}
	return "NULL";
}

/**
 * checkFuncReturn - Checks the return type of a function.
 * This function validates the return type of a function by comparing the function's declared return type
 * with the actual return value types present in the function body. It handles special cases for "VOID"
 * functions and reports errors if there are type mismatches or if a void function returns a value.
 * @param funcNode: The AST node representing the function.
 * @return: 1 if the return type is valid, 0 otherwise.
 */
int checkFuncReturn(node *funcNode){
    char *funcType = funcNode->nodes[2]->token;
    int ans = evalReturn(funcNode->nodes[3], funcType);	
    if (!strcmp(funcType,"VOID") && ans == 0){
		errorsCounter++;
        printf ("Error: Line %d: Void function '%s' cannot return value.\n", funcNode->line, funcNode->nodes[0]->token);
        return 0;
    }
    else if (ans == 0){
		errorsCounter++;
        printf ("Error: Line %d: Function '%s' returns an invalid value.\n", funcNode->line ,funcNode->nodes[0]->token);
        return 0;
    }
    return 1;
}

/**
 * evalReturn - Evaluates return statements in a function for type correctness.
 * This function traverses the function body to find return statements and checks if the returned values
 * match the declared return type of the function. It ensures that non-void functions return appropriate values
 * and that the return type is consistent with the function's declaration.
 * @param funcNode: The AST node representing the function body.
 * @param type: The declared return type of the function.
 * @return: 1 if all return statements are valid, 0 otherwise.
 */
int evalReturn(node* funcNode, char* type){
	for (int i=0;i<funcNode->count;i++){
		if(!strcmp(funcNode->nodes[i]->token, "RET")){
			if(funcNode->nodes[i]->count == 0 && strcmp(type, "VOID")){
				return 0;
			}
			else if (funcNode->nodes[i]->count > 0){
				char* val = evaluateExpression(funcNode->nodes[i]->nodes[0]);
				if (strcmp(val, type))
					return 0;
			}
		}
		if(strcmp(funcNode->nodes[i]->token, "FUNCTION"))
			if(evalReturn(funcNode->nodes[i], type)==0)
				return 0;
	}
	return 1;
}

/**
 * evalPtr - Evaluates pointer assignment for type consistency.
 * This function checks if the left-hand side and right-hand side types in a pointer assignment are compatible.
 * It reports an error if there is a type mismatch, ensuring that pointers are assigned correctly.
 * @param ptrNode: The AST node representing the pointer assignment.
 */
void evalPtr(node* ptrNode){
	char *left = evaluateExpression(ptrNode->nodes[0]);
	char *right = evaluateExpression(ptrNode->nodes[1]);
	if (strcmp(right,left)){
		errorsCounter++;
		printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", ptrNode->line, right, left);
	}
}

/**
 * isSymbolExist - Checks for duplicate symbols in a scope and its ancestors.
 * This function iterates through all symbols in the current scope and its parent scopes to detect and report
 * any duplicate symbol declarations, including functions and variables. It helps maintain the uniqueness of
 * identifiers within a scope.
 * @param scope: The current scope in the symbol table hierarchy.
 */
void isSymbolExist(scopeNode* scope){
    scopeNode * current = scope;
    while(current != NULL){
        checkSymbols(current);
        current=current->next;
    }
}

/**
 * checkSymbols - Checks for duplicate symbols within a single scope.
 * This function iterates through the symbol table of a given scope and checks for duplicate identifiers.
 * It distinguishes between functions and variables, reporting re-declarations of either.
 * @param scope: The scope node containing the symbol table to check.
 */
void checkSymbols(scopeNode* scope){
    symbolNode* s1 = scope->symbolTable;
	symbolNode* s2;
 	while(s1!= NULL){
        s2 = s1->next;
        while (s2 != NULL){
            if (!strcmp(s1->id, s2->id)){
                if (s1->isFunc){
					errorsCounter++;
                    printf ("Re-declaration of function (%s)\n", s1->id);
				}
                else{
					errorsCounter++;
                    printf ("Re-declaration of variable (%s)\n", s1->id);
				}
        	}
            s2 =s2->next;
        }
        s1 =s1->next;
    }
}

/**
 * checkFunctionCall - Validates a function call for correctness.
 * This function checks if a function call has been made to a declared function with the correct arguments.
 * It ensures the function exists and the provided arguments match the function's parameters in number and type.
 * Reports errors if the function is undeclared or the arguments do not match.
 * @param funcName: The name of the function being called.
 * @param callArgs: The AST node representing the arguments passed to the function.
 * @return: 1 if the function call is valid, 0 otherwise.
 */
int checkFunctionCall(char *funcName, node *callArgs){
    symbolNode *funcSymbol = scopeSearch(funcName);
	if (funcSymbol !=NULL)
		if (checkFunctionArgs(funcSymbol->params, callArgs))
			return 1;
	errorsCounter++;		
    printf ("Error: Line %d: Undeclared function '%s' with unmatching arguements\n", callArgs->line,funcName);
    return 0; 
}

/**
 * checkStaticNonStaticCalls - Checks for illegal calls from static functions to non-static functions.
 * This function ensures that static functions do not call non-static functions. It iterates through all recorded
 * function calls and checks if any static functions have called non-static functions, reporting errors if such
 * calls are found.
 */
void checkStaticNonStaticCalls() {
    int i = 0, j = 0;
    for (j = 0; j < func_has_been_called_index; j++) {
        symbolNode* calledFuncSymbol = scopeSearch(func_has_been_called[j]);
        if (calledFuncSymbol != NULL) {
            for (i = 0; i < func_called_index; i++) {
                symbolNode* callingFuncSymbol = scopeSearch(func_called[i]);

                if (callingFuncSymbol != NULL) {
                    if (callingFuncSymbol->isStatic && !calledFuncSymbol->isStatic) {
                        errorsCounter++;
                        printf("Error: Static function '%s' cannot call non-static function '%s'.\n", callingFuncSymbol->id, func_has_been_called[j]);
                        exit(1);
                    }
				}
			}
		}
    }
}

/**
 * checkMainNonStaticCalls - Ensures the 'main' function does not call non-static functions.
 * This function traverses the AST to find the main function and checks that it does not call non-static functions.
 * Reports an error if the main function makes such calls.
 * @param tree: The AST node to check.
 */
void checkMainNonStaticCalls(node* tree) {
    if (tree == NULL) return;

    char* token = tree->token;

    static int inMain = 0;

    if (strcmp(token, "MAIN") == 0) {
        inMain = 1;
    }

    if (strcmp(token, "FUNC_CALL") == 0) {
        if (inMain) {
            symbolNode* calledFuncSymbol = scopeSearch(tree->nodes[0]->token);
            if (calledFuncSymbol != NULL && !calledFuncSymbol->isStatic) {
                errorsCounter++;
                printf("Error: 'main' function cannot call non-static function '%s'.\n", calledFuncSymbol->id);
                exit(1);
            }
        }
    }

    for (int j = 0; j < tree->count; j++) {
        checkMainNonStaticCalls(tree->nodes[j]);
    }

    if (strcmp(token, "MAIN") == 0) {
        inMain = 0;
    }
}

/**
 * checkFunctionArgs - Checks if the provided function call arguments match the function's parameters.
 * This function compares the actual arguments provided in a function call with the expected parameters
 * of the function, checking for both count and type. Reports an error if there is a mismatch.
 * @param params: The AST node representing the function's parameters.
 * @param callArgs: The AST node representing the arguments passed in the function call.
 * @return: 1 if the arguments match the parameters, 0 otherwise.
 */
int checkFunctionArgs(node* params, node* callArgs){
	if (!strcmp(params->token,"ARGS_NONE") && !strcmp(callArgs->token,"ARGS_NONE"))
		return 1;
	int count = 0;
	for (int i = 0;i<params->count;i++)
		count += params->nodes[i]->count;
	if(count != callArgs->count)
		return 0;
	int k = 0;
	for (int i = 0;i<params->count;i++){
		for(int j = 0; j < params->nodes[i]->count;j++){
			if (strcmp(params->nodes[i]->token, evaluateExpression(callArgs->nodes[k++])))
				return 0;
		}
	}
	return 1;
}

/**
 * checkString - Checks type consistency in string assignments.
 * This function validates assignments involving strings, ensuring that string array cells are assigned 'CHAR'
 * types and that indices are of type 'INT'. It also checks that only 'STRING' types are assigned to string variables.
 * Reports errors if type mismatches are found.
 * @param strNode: The AST node representing the string assignment.
 * @param assType: The type of the assigned value.
 */
void checkString(node* strNode, char* assType){
	if(strcmp(assType,"CHAR") && strNode->nodes[0]->count != 0 && !strcmp(strNode->nodes[0]->nodes[0]->token, "INDEX")){
		errorsCounter++;
		printf("Error: Line %d: Assignment type mismatch - expected 'CHAR' for string array cell, but found '%s'.\n", strNode->nodes[0]->line ,assType);
	}
	if (strNode->nodes[0]->count != 0 && !strcmp(strNode->nodes[0]->nodes[0]->token, "INDEX")){
		char* indexType = evaluateExpression(strNode->nodes[0]->nodes[0]->nodes[0]);
		if(strcmp("INT", indexType)){
			errorsCounter++;
			printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", strNode->nodes[0]->line ,indexType);
		}
	}
	if (strNode->nodes[0]->count == 0){
		if(strcmp("STRING", assType)){
			errorsCounter++;
			printf("Error: Line %d: Assignment type mismatch: cannot assign '%s' to STRING\n", strNode->nodes[0]->line ,assType);
		}
	}
}

/**
 * findFunctionsCalled - Records functions called within the AST.
 * This function traverses the AST to find function call nodes and records the functions that are called.
 * It tracks the current function being parsed to associate calls with their caller functions, aiding in analysis
 * of function interactions.
 * @param tree: The AST node to check for function calls.
 */
void findFunctionsCalled(node* tree) {
    if (tree == NULL) return;

    char* token = tree->token;

    if (strcmp(token, "FUNCTION") == 0 || strcmp(token, "STATIC-FN") == 0) {
        if (tree->nodes[0] && tree->nodes[0]->token) {
            currentFunction = tree->nodes[0]->token;
		}
    }

    if (strcmp(token, "FUNC_CALL") == 0) {
        if (currentFunction) {
			func_has_been_called[func_has_been_called_index++] = tree->nodes[0]->token;
            func_called[func_called_index++] = currentFunction; 
        }
    }

    for (int j = 0; j < tree->count; j++) {
        findFunctionsCalled(tree->nodes[j]);
    }
}

/**
 * setFatherForStatements - Assigns a father (parent function) to each statement in the AST.
 * This function sets the father attribute for an array of statement nodes, assigning them to a specific function
 * or block. It recursively updates the father attribute for all child nodes of each statement.
 * @param statements: Array of AST nodes representing statements.
 * @param size: Number of statements in the array.
 * @param fatherName: The name of the parent function or block.
 */
void setFatherForStatements(node** statements, int size, char* fatherName) {
    for (int i = 0; i < size; i++) {
        statements[i]->father = fatherName;
        setFatherForChildNodes(statements[i], fatherName);
    }
}

/**
 * setFatherForChildNodes - Recursively sets the father attribute for all child nodes.
 * This function assigns the father attribute to all child nodes of a given AST node, indicating the parent function
 * or block for proper scope management and analysis.
 * @param root: The root AST node whose children will be updated.
 * @param fatherName: The name of the parent function or block.
 */
void setFatherForChildNodes(node* root, char* fatherName) {
    if (root == NULL) return;

    root->father = fatherName;
    for (int i = 0; i < root->count; i++) {
        setFatherForChildNodes(root->nodes[i], fatherName);
    }
}

/**
 * printSymbolTable - Prints the symbol table for debugging and analysis.
 * This function traverses the symbol table from the top scope and prints information about each symbol,
 * including its identifier, type, value, and the scope in which it is declared. It helps visualize the structure
 * and contents of the symbol table.
 * @param node: The top scope node in the symbol table hierarchy.
 */
void printSymbolTable(scopeNode *node)
{  
   scopeNode *currentScope = node;
   symbolNode *currentSymbol;
   while(currentScope != NULL){
		currentSymbol = currentScope->symbolTable;
		while (currentSymbol != NULL) {
			printf("ID: %s\t|\tVariable Type: %s\t|\tValue: %s\t|\tLocated In Scope: %d", currentSymbol->id, currentSymbol->type, currentSymbol->data,currentSymbol->scopeID);
			printf("\n");
			currentSymbol = currentSymbol->next;
		}
	currentScope=currentScope->next;       
	}
}

/**
 * printScopes - Prints information about all scopes in the program.
 * This function prints details about each scope, including its name, number, and level in the hierarchy.
 * It provides a summary of all scopes, aiding in understanding the scope structure and scope-specific details.
 * @param node: The top scope node in the scope hierarchy.
 */
void printScopes(scopeNode *node){
    scopeNode *current=node;
    while (current != NULL)	{
		printf("Scope ID: %s\t|\tScope Number: %d\t|\tScope Level: %d\n", current->scopeName,current->scopeNum,current->scopeLevel);
		current = current->next;
	}
    printf("Total Scopes: %d\n",GlobalScope);
}

/**
 * printInfo - Prints the Abstract Syntax Tree (AST), symbol table, and scopes for analysis.
 * This function provides a complete overview of the program's structure by printing the AST, symbol table,
 * and scope information. It helps in debugging and understanding the program's flow and symbol usage.
 * @param root: The root node of the AST.
 */
void printInfo(node *root){
    printf ("Abstract Syntax Tree:\n"); 
	printTree (root,0);
	printf("-------------------------\n"); 
    printf("Symbol Table Information:\n");
    printSymbolTable(topStack);
	printf("-------------------------\n"); 
    printf("Scopes Information:\n");
    printScopes(topStack);
    printf("\n"); 
}