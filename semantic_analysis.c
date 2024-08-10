#include "semantic_analysis.h"
#include <stddef.h>
#include <ctype.h>

/* Global variables */
scopeNode* SCOPE_STACK_TOP = NULL;
int mainCounter = 0;
char* currentFunction = NULL;
char* func_called[256];
char* func_has_been_called[256];
int func_called_index = 0;
int func_has_been_called_index = 0;

node* makeNode(char* token) {
	node *newnode = (node*)malloc(sizeof(node));
	char *new_token = (char*)malloc(sizeof(token) + 1);
	strcpy(new_token, token);
	newnode->token = new_token;
	newnode->father = NULL;
	newnode->nodes = NULL;
	newnode->count = 0;
	newnode->node_type = NULL;
	newnode->line = 0;
	return newnode;
}

node* combineNodes(char* token, node* first_node, node* second_node) {
	if(!first_node){
		return second_node;
	}
	if(!second_node){
		return first_node;
	}
	node* combined_node = (node*)malloc(sizeof(node));
	int i = 0;
	int j = 0;
	int new_count = first_node->count + second_node->count; // TODO chage to num_of_sons
	if (first_node->count == 0) // leaf
		new_count += 1;
	if (second_node->count == 0) // leaf
		new_count += 1;

	combined_node->token = strdup(token);
	combined_node->count = new_count;
	combined_node->nodes = (node**)malloc(sizeof(node*) * combined_node->count);
	if (first_node->count == 0)
		combined_node->nodes[j++] = first_node;
	else {
		for (i = 0; i < first_node->count; i++) {
			combined_node->nodes[j] = first_node->nodes[i];
			j++;
		}
		freeNode(first_node, 0);
	}
	if (second_node->count == 0)
		combined_node->nodes[j++] = second_node;
	else {
		for (i = 0; i < second_node->count; i++) {
			combined_node->nodes[j] = second_node->nodes[i];
			j++;
		}
		freeNode(second_node, 0);
	}
	return combined_node;
}

void addSonNodeToFatherNode(node** father, node* son) {
	// maloc
	// set son
	// add to number of sons

	if (!father || !(*father) || !son){
		return;
	}
	if ((*father)->count != 0) {
		(*father)->nodes = (node**)realloc((*father)->nodes, ((*father)->count+1)*sizeof(node*));
		(*father)->nodes[(*father)->count] = son;
		(*father)->count++;
	}
	else {
		(*father)->nodes = (node**)malloc(sizeof(node*));
		(*father)->nodes[(*father)->count] = son;
		(*father)->count++;
	}
}

void addSonsNodesToFatherNode(node* father, node* sons){
	addSonNodeToFatherNode(&father, sons);
	for (int i = 0; i < sons->count; i++){
		addSonNodeToFatherNode(&father, sons->nodes[i]);
	}
	
}

void addNodesList(node* add_to, node* to_add){
	if (to_add->count != 0){
		int i;
		for (i = 0; i < to_add->count; i++){
			addSonNodeToFatherNode(&add_to, to_add->nodes[i]);
		}
	}
	else{
		addSonNodeToFatherNode(&add_to, to_add);
	}
}

void printTree(node* curr_node, int num_of_spaces) {
	int i;
	int j;
	int spaces_multiplier = 3; // to emphesize spaces between each line/scope
	char* curr_token = curr_node->token;
	
	if (!*curr_token) {
		num_of_spaces -= 1;
	}
	else{
		for (i = 0; i < num_of_spaces; i++) {
			for(j = 0; j < spaces_multiplier; j++){
				printf(" ");
			}
		}
		printf("(%s\n", curr_token);
	}

	if (curr_node->nodes) {
		for (int j = 0; j < curr_node->count; j++) {
			printTree(curr_node->nodes[j], num_of_spaces + 1);
		}
	}

	for (i = 0; i < num_of_spaces; i++) {
		for(j = 0; j < spaces_multiplier; j++){
				printf(" ");
			}
	}
	if (*curr_token)
		printf(")\n");
}

void freeNode(node* node_to_free, int free_sons){
	if (!node_to_free)
		return;

	int i;
	for(i = 0; i < node_to_free->count && free_sons == 1; i++){
		free(node_to_free->nodes[i]);
	}

	free(node_to_free);
}


/* Semantic Check - Part 2 */

/**
 * semanticAnalysis - Conducts semantic analysis on the given root node.
 * @param root: The root node of the abstract syntax tree.
 */
void semanticAnalysis(node* root) {
	pushStatementToStack(root, 0);
	checkForSymbolsDuplications(SCOPE_STACK_TOP);
	findFunctionsCalled(root);
	checkMainNonStaticCalls(root);
	checkStaticNonStaticCalls();
	printTree (root,0);
}

void pushStatementToStack(node* root, int scope_level){
	if (root == NULL)
		return;
	
	switch (root->token[0]) {
		case 'F': // FUNCTION or FOR
			if (!strcmp(root->token, "FUNCTION")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, root->nodes[1], root->nodes[3]->nodes, scope_level, root->nodes[3]->count);
				checkFuncReturn(root);
			}
			else if (!strcmp(root->token, "FOR")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes[3]->nodes, scope_level, root->nodes[3]->count);
				char* initType = checkExpAndReturnItsType(root->nodes[0]);
				if(strcmp("INT" ,initType)){
					printf("Error: Line %d: FOR initialization requires an 'INT' type.\n", root->nodes[0]->line);
					exit(1);
				}
				char* exp_type = checkExpAndReturnItsType(root->nodes[1]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error: Line %d: FOR-condition requires return type 'BOOL'.\n", root->nodes[0]->line);
					exit(1);
				}
				char* incType = checkExpAndReturnItsType(root->nodes[2]);
				if(strcmp("INT" ,incType)){
					printf("Error: Line %d: FOR-increment requires return type 'INT'.\n", root->nodes[0]->line);
					exit(1);
				}
			}
			break;

		case 'S': // STATIC-FN
			if (!strcmp(root->token, "STATIC-FN")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, root->nodes[1], root->nodes[3]->nodes, scope_level, root->nodes[3]->count);
				checkFuncReturn(root);
			}
			break;

		case 'W': // WHILE
			if (!strcmp(root->token, "WHILE")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes[1]->nodes, scope_level, root->nodes[1]->count);
				char* exp_type = checkExpAndReturnItsType(root->nodes[0]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error: Line %d: Invalid WHILE condition. Expected return type 'BOOL'.\n", root->line);
					exit(1);
				}
			}
			break;

		case 'D': // DO-WHILE
			if (!strcmp(root->token, "DO-WHILE")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes[0]->nodes, scope_level, root->nodes[0]->count);
				char* exp_type = checkExpAndReturnItsType(root->nodes[1]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error: Line %d: Invalid DO-WHILE condition. Expected return type 'BOOL'.\n", root->line);
					exit(1);
				}
			}
			break;

		case 'C': // CODE
			if (!strcmp(root->token, "CODE")) {
				scope_level++;
        		pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes, scope_level, root->count);
			}
			break;

		case 'M': // MAIN
			if (!strcmp(root->token, "MAIN")) {
				scope_level++;
				mainCounter++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes[0]->nodes, scope_level, root->nodes[0]->count);
			}
			break;

		case 'I': // IF
			if (!strcmp(root->token, "IF")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes[1]->nodes, scope_level, root->nodes[1]->count);
				char* exp_type = checkExpAndReturnItsType(root->nodes[0]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error: Line %d: Invalid IF condition. Expected return type 'BOOL'.\n",root->line);
					exit(1);
				}
			}
			break;

		case 'B': // BLOCK
			if (!strcmp(root->token, "BLOCK")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->nodes, scope_level, root->count);
			}
			break;

		default:
			break;
	}

	for (int i = 0; i < root->count; i++){
		pushStatementToStack(root->nodes[i], scope_level);
	}
}

void pushScopeToScopeStack(scopeNode** scope_stack_top, node* scope_params, node** statements, int scope_level, int num_of_statements){      
	scopeNode* new_scope = (scopeNode*) malloc(sizeof(scopeNode));
	new_scope->scopeLevel = scope_level-1;
	new_scope->next = (*scope_stack_top);
	(*scope_stack_top) = new_scope;
	if (scope_params){
		pushSymbolsToSymbolTable(scope_params); 
	}
	pushStatementsToScope(statements, num_of_statements);
}

void pushStatementsToScope(node** statements, int num_of_statements){
	for (int i = 0;i < num_of_statements;i++){
		if(!strcmp(statements[i]->token, "VAR")){
			pushSymbolsToSymbolTable(statements[i]);
		}

		else if (!strcmp(statements[i]->token, "FUNCTION")){
			addSymbolToSymbolTable(&SCOPE_STACK_TOP, statements[i]->nodes[0]->token, statements[i]->nodes[2]->token, NULL, 1, 0, statements[i]->nodes[1]);
		}

		else if (!strcmp(statements[i]->token, "STATIC-FN")) {
			addSymbolToSymbolTable(&SCOPE_STACK_TOP, statements[i]->nodes[0]->token, statements[i]->nodes[2]->token, NULL, 1, 1, statements[i]->nodes[1]);
		}

		else if (!strcmp(statements[i]->token, "FUNC_CALL")){
			checkFunctionCall(statements[i]->nodes[0]->token, statements[i]->nodes[1]);
		}

		else if(!strcmp(statements[i]->token, "<-")){
        	if (isVarDeclared(statements[i]->nodes[0]->token) && strcmp(statements[i]->nodes[0]->token, "PTR")){
                char *left = scopeSearch(statements[i]->nodes[0]->token)->type;
                char *right = checkExpAndReturnItsType(statements[i]->nodes[1]);
				if (!strcmp(left, "STRING"))
					checkStringAssignment(statements[i], right);
				else if (statements[i]->nodes[0]->count > 0){
					printf("Error: Line %d: %s cannot have index.\n", statements[i]->nodes[0]->line, left);
					exit(1);
				}
				else if (!strcmp(right, "NULL") && (strcmp(left, "INT*") && strcmp(left, "CHAR*") && strcmp(left, "DOUBLE*")) && strcmp(left, "FLOAT*")){
					printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", statements[i]->line, right, left);
					exit(1);
				}
            	else if (strcmp(right,left) && strcmp(right,"undefined")){
                    printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", statements[i]->line, right, left);
					exit(1);
				}
			}
			
			else if (!strcmp(statements[i]->nodes[0]->token, "PTR") && isVarDeclared(statements[i]->nodes[0]->nodes[0]->nodes[0]->token))
				isValidPrtAssinment(statements[i]);
			
			else{
				checkExpAndReturnItsType(statements[i]->nodes[1]);
				printf("Error: Line %d: Undeclared variable [%s]\n", statements[i]->line, statements[i]->nodes[0]->token);
				exit(1);
			}
		}
	}
}

void pushSymbolsToSymbolTable(node* var_declaration_nosde){
	int i;
	int j;
	for(i = 0; i<var_declaration_nosde->count; i++){

		int num_of_vars = var_declaration_nosde->nodes[i]->count;
		char* var_type = var_declaration_nosde->nodes[i]->token;
		node** vars_declared = var_declaration_nosde->nodes[i]->nodes;

		for(int j = 0; j < num_of_vars; j++){
			if ((!strcmp(vars_declared[j]->token, "<-") && vars_declared[j]->nodes[1]->node_type != NULL && !strcmp("NULL", vars_declared[j]->nodes[1]->node_type)))
				if (strcmp(var_type, "INT*") && strcmp(var_type, "CHAR*") && strcmp(var_type, "DOUBLE*") && strcmp(var_type, "FLOAT*")){
					printf("Error: Line %d: Assignment type mismatch: can not assign NULL to %s\n", vars_declared[j]->line, var_type);
					exit(1);
				}
				else
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, vars_declared[j]->nodes[0]->token, var_type, vars_declared[j]->nodes[1]->token, 0, 0, NULL);
			
			else if ((!strcmp(vars_declared[j]->token, "<-") && strcmp(var_type, "STRING") == 0)){
				char* exp_type = checkExpAndReturnItsType(vars_declared[j]->nodes[0]->nodes[0]->nodes[0]);
				if(strcmp("INT", exp_type)){
					printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", vars_declared[j]->nodes[0]->line, exp_type);
					exit(1);
				}
				else {
					exp_type = checkExpAndReturnItsType(vars_declared[j]->nodes[1]);
					if (!strcmp(var_type, exp_type))
						addSymbolToSymbolTable(&SCOPE_STACK_TOP, vars_declared[j]->nodes[0]->token, var_type, vars_declared[j]->nodes[1]->token, 0, 0, NULL);
					else{
						printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", vars_declared[j]->nodes[0]->line, exp_type, var_type);
						exit(1);
					}
				}
			}

			else if (strcmp(vars_declared[j]->token, "<-") && strcmp(var_type, "STRING") == 0){
				char* exp_type = checkExpAndReturnItsType(vars_declared[j]->nodes[0]->nodes[0]);
				if(strcmp("INT", exp_type)){
					printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", vars_declared[j]->line, exp_type);
					exit(1);
				}
				else
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, vars_declared[j]->token, var_type, NULL, 0, 0, NULL);
			}
			
			else{
				if (strcmp(vars_declared[j]->token, "<-"))
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, vars_declared[j]->token, var_type, NULL, 0, 0, NULL);
				else{
					char* exp_type = checkExpAndReturnItsType(vars_declared[j]->nodes[1]);
					if (!strcmp(var_type, exp_type))
						addSymbolToSymbolTable(&SCOPE_STACK_TOP, vars_declared[j]->nodes[0]->token, var_type, vars_declared[j]->nodes[1]->token, 0, 0, NULL);
					else {
						printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", vars_declared[j]->nodes[0]->line, exp_type, var_type);
						exit(1);
					}
				}	
			}
		}
	}
}

void addSymbolToSymbolTable(scopeNode** scope_stack_top, char* symbol_id, char* symbol_type, char* data, int is_func, int is_static, node* params) {
	symbolNode* new_node = (symbolNode*) malloc(sizeof(symbolNode));
	new_node->next =(*scope_stack_top)->symbolTable;
	(*scope_stack_top)->symbolTable = new_node;

	new_node->id = (char*)(malloc (sizeof(symbol_id) + 1));
	strncpy(new_node->id, symbol_id, sizeof(symbol_id)+1);

	new_node->type = (char*)(malloc (sizeof(symbol_type) + 1));
	strncpy(new_node->type, symbol_type, sizeof(symbol_type)+1);

	if (data != NULL) {
		new_node->data = (char*)(malloc (sizeof(data) + 1));
		strncpy(new_node->data, data, sizeof(data)+1);
	}
	else
		new_node->data = NULL;

	new_node->is_func = is_func;
	new_node->is_static = is_static;
	
	if (params != NULL) {
		new_node->params = (node*)(malloc(sizeof(node)));
		memcpy(new_node->params, params, sizeof(node));
	}
	else
		new_node->params = NULL;	
}

int isVarDeclared(char* var_name){
	symbolNode* symbol = scopeSearch(var_name);
	return (symbol == NULL) ? 0 : 1;
}

symbolNode* symbolSearch (symbolNode* symbol_table, char* id){
	symbolNode* curr_symbol_table;
	for(curr_symbol_table = symbol_table; curr_symbol_table != NULL; curr_symbol_table = curr_symbol_table->next){
		if (!strcmp(curr_symbol_table->id, id)){
            return curr_symbol_table;
        }
	}
	return NULL;
}

symbolNode* scopeSearch(char* id) {
    scopeNode* curr_scope = SCOPE_STACK_TOP;
	int curr_level;

    while (curr_scope != NULL) {
        symbolNode* found_symbol = symbolSearch(curr_scope->symbolTable, id);
        if (found_symbol != NULL) {
            return found_symbol;  
        }

		curr_level = curr_scope->scopeLevel;
        if (curr_level == 0) {
            break;
        }

        // skip scopes that are not direct parents by comparing scope levels
        curr_scope = curr_scope->next;
        while (curr_scope != NULL && curr_scope->scopeLevel >= curr_level) {
            curr_scope = curr_scope->next;
        }
    }

    return NULL;
}

char* checkExpAndReturnItsType(node* exp){
	if (exp->node_type != NULL && !strcmp(exp->node_type, "NULL"))
		return "NULL";

	else if (exp->node_type != NULL && !strcmp(exp->node_type, "ID")){
		symbolNode* symbol_node = scopeSearch(exp->token);
		if(symbol_node != NULL){
			if(!strcmp(symbol_node->type, "STRING") && exp->count > 0){
				char* indexType = checkExpAndReturnItsType(exp->nodes[0]->nodes[0]);
				if(strcmp("INT", indexType)){
					printf("Error: Line %d: Size of string must be type 'INT' not '%s'.\n", exp->line ,indexType);
					exit(1);
				}
			return "CHAR";
			}
			return symbol_node->type;
		}
        else{
			printf("Error: Line %d: Undeclared variable '%s'.\n", exp->line, exp->token);
			exit(1);
		}
	}

	else if (exp->node_type != NULL)
		return exp->node_type;

	else if(!strcmp(exp->token, "<-")){
        	if (isVarDeclared(exp->nodes[0]->token)){
                char *left = scopeSearch(exp->nodes[0]->token)->type;
                char *right = checkExpAndReturnItsType(exp->nodes[1]);
            	if (strcmp(right, "NULL") && strcmp(right, left)){
                    printf("Error: Line %d: Assignment type mismatch: cannot assign '%s' to '%s'.\n", exp->line, right, left);
					exit(1);
				}
				else
					return left;
			}
		}

	else if (!strcmp(exp->token,"&")){
		char* left;
        left = checkExpAndReturnItsType(exp->nodes[0]);
		if(isArithmeticType(left) || !strcmp(left,"CHAR")){
			char* result = malloc(strlen(left) + 2); // +2 for the '*' and the null terminator
			if (result == NULL) {
				printf("Error: Memory allocation failed.\n");
				exit(1);
			}
			strcpy(result, left);
			strcat(result, "*");
			return result;
		}
		else{
			printf("Error: Line %d: Cannot perform '&' on '%s' - [&%s]\n", exp->nodes[0]->line, left,exp->nodes[0]->token);
			exit(1);
		}
	}

	else if (!strcmp(exp->token,"LEN")){
		char* left;
        left = checkExpAndReturnItsType(exp->nodes[0]);
		if(!strcmp(left,"STRING"))
            return "INT";
		else{
			printf("Error: Line %d: Cannot perform || on '%s' - [|%s|]\n", exp->nodes[0]->line, left,exp->nodes[0]->token);
			exit(1);
		}
	}	
	
	else if(!strcmp(exp->token, "PTR")){
        char* left;
        left = checkExpAndReturnItsType(exp->nodes[0]->nodes[0]);
		char* base_type = getPointerBaseType(left);
        if(!strcmp(left, "NULL")){
			printf("Error: Line %d: '%s' is not pointer\n", exp->nodes[0]->nodes[0]->line ,left);
			exit(1);
		}
		return base_type;
	}

	else if(!strcmp(exp->token, "NOT")){
        char* left;
        left = checkExpAndReturnItsType(exp->nodes[0]);
        if(!strcmp(left,"BOOL"))
            return "BOOL";
		else{
			printf("Error: Line %d: Cannot perform ! on '%s' - [!%s]\n", exp->line ,left,exp->nodes[0]->token);
			exit(1);
		}
	}

	else if (!strcmp(exp->token, "FUNC_CALL")){
		if(checkFunctionCall(exp->nodes[0]->token, exp->nodes[1])){
			symbolNode* func_symbol = scopeSearch(exp->nodes[0]->token);
			return func_symbol->type;
		}
	}

	else if(!strcmp(exp->token, "==") || !strcmp(exp->token, "!=")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->nodes[0]);
        right = checkExpAndReturnItsType(exp->nodes[1]);
        if (isEqualType(left, right)) {
            return "BOOL";
        }
		else{
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line ,exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
			exit(1);
		}
	}

	else if(!strcmp(exp->token, "&&") || !strcmp(exp->token, "||")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->nodes[0]);
        right = checkExpAndReturnItsType(exp->nodes[1]);
        if(!strcmp(left,"BOOL") && !strcmp(right,"BOOL"))
            return "BOOL";
		else {
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line, exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
			exit(1);
		}
	}

	else if(!strcmp(exp->token, ">") || !strcmp(exp->token, "<") || !strcmp(exp->token, ">=") || !strcmp(exp->token, "<=")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->nodes[0]);
        right = checkExpAndReturnItsType(exp->nodes[1]);
        if(isCompatibleForComparison(left, right))
            return "BOOL";
		else{
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line,exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
			exit(1);
		}
	} 
	
	else if (!strcmp(exp->token, "*") || !strcmp(exp->token, "/") || !strcmp(exp->token, "+") || !strcmp(exp->token, "-")){
		char* left_exp, *right_exp;
        left_exp = checkExpAndReturnItsType(exp->nodes[0]);
        right_exp = checkExpAndReturnItsType(exp->nodes[1]);
		if (!strcmp(left_exp, "NULL") || !strcmp(right_exp, "NULL"))
			return "NULL";
        if (isArithmeticType(left_exp) && isArithmeticType(right_exp)) {
            return getArithmeticResultType(left_exp, right_exp, exp);
        }
		else{
			printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line, exp->token, left_exp, right_exp ,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
			exit(1);
		}
	}

	else{
		return "NULL";
	}	 			
}

int isArithmeticType(char* type) {
    return !strcmp(type, "INT") || !strcmp(type, "FLOAT") || !strcmp(type, "DOUBLE");
}

char* getArithmeticResultType(char* left, char* right, node* exp) {
    if (!strcmp(left, "INT") && !strcmp(right, "INT"))
        return "INT";
    else if (!strcmp(left, "FLOAT") && !strcmp(right, "FLOAT"))
        return "FLOAT";
    else if ((!strcmp(left, "DOUBLE") || !strcmp(right, "DOUBLE")))
        return "DOUBLE";
    else if ((!strcmp(left, "FLOAT") || !strcmp(right, "FLOAT")))
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
		printf("Error: Line %d: Cannot perform '%s' operation between '%s' and '%s' - [%s %s %s]\n", exp->line, exp->token, left, right,exp->nodes[0]->token, exp->token, exp->nodes[1]->token);
		exit(1);
	}
}

int isCompatibleForComparison(char* left, char* right) {
    return (!strcmp(left, right) && !strcmp(right, "INT")) ||
           (!strcmp(left, right) && !strcmp(right, "FLOAT")) ||
           (!strcmp(left, right) && !strcmp(right, "DOUBLE"));
}

int isEqualType(char* left, char* right) {
    if (!strcmp(left, right)) {
        if (!strcmp(left, "BOOL") || !strcmp(left, "CHAR") ||
            !strcmp(left, "INT") || !strcmp(left, "DOUBLE") ||
            !strcmp(left, "FLOAT")) {
            return 1;
        }
    }
    return 0;
}

char* getPointerBaseType(char* type) {
    if (!strcmp(type, "INT*"))
        return "INT";
    if (!strcmp(type, "CHAR*"))
        return "CHAR";
    if (!strcmp(type, "DOUBLE*"))
        return "DOUBLE";
    if (!strcmp(type, "FLOAT*"))
        return "FLOAT";
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
        printf ("Error: Line %d: Void function '%s' cannot return value.\n", funcNode->line, funcNode->nodes[0]->token);
        exit(1);
    }
    else if (ans == 0){
        printf ("Error: Line %d: Function '%s' returns an invalid value.\n", funcNode->line ,funcNode->nodes[0]->token);
        exit(1);
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
				char* val = checkExpAndReturnItsType(funcNode->nodes[i]->nodes[0]);
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

void isValidPrtAssinment(node* ptr_node){
	char *left = checkExpAndReturnItsType(ptr_node->nodes[0]);
	char *right = checkExpAndReturnItsType(ptr_node->nodes[1]);
	if (strcmp(right,left)){
		printf("Error: Line %d: Assignment type mismatch: can not assign %s to %s\n", ptr_node->line, right, left);
		exit(1);
	}
}

void checkForSymbolsDuplications(scopeNode* scope){
	scopeNode* curr_scope;
	symbolNode* s1;
	symbolNode* s2;
	for(curr_scope = scope; curr_scope != NULL; curr_scope = curr_scope->next){
		for(s1 = scope->symbolTable; s1 != NULL; s1 = s1->next){
			for(s2 = s1->next; s2 != NULL; s2 = s2->next){
				if (!strcmp(s1->id, s2->id)){
					if (s1->is_func){
						printf ("Re-declaration of function (%s)\n", s1->id);
						exit(1);
					}
					else{
						printf ("Re-declaration of variable (%s)\n", s1->id);
						exit(1);
					}
				}
			}
		}
	}
}

int checkFunctionCall(char* func_name, node* func_args){
    symbolNode *func_symbol = scopeSearch(func_name);
	if (func_symbol != NULL)
		if (checkFunctionArgs(func_symbol->params, func_args))
			return 1;
    printf ("Error: Line %d: Undeclared function '%s' with unmatching arguements\n", func_args->line, func_name);
	exit(1);
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
                    if (callingFuncSymbol->is_static && !calledFuncSymbol->is_static) {
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
            if (calledFuncSymbol != NULL && !calledFuncSymbol->is_static) {
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

int checkFunctionArgs(node* func_params, node* func_args){
	// no args
	if (!strcmp(func_params->token,"ARGS_NONE") && !strcmp(func_args->token,"ARGS_NONE"))
		return 1;

	// num of args == num of params
	int expected_param_count = 0;
	for (int i = 0; i < func_params->count; i++)
		expected_param_count += func_params->nodes[i]->count;
	if(expected_param_count != func_args->count)
		return 0;

	// type matching
    for (int i = 0, k = 0; i < func_params->count; i++) {
        char* expected_type = func_params->nodes[i]->token;
        for (int j = 0; j < func_params->nodes[i]->count; j++, k++) {
            if (strcmp(expected_type, checkExpAndReturnItsType(func_args->nodes[k]))) {
                return 0;
            }
        }
    }
	return 1;
}

void checkStringAssignment(node* str_node, char* assigned_val_type){
	if (str_node->nodes[0]->count == 0){
		if(strcmp("STRING", assigned_val_type)){
			printf("Error: Line %d: Assignment type mismatch: cannot assign '%s' to STRING\n", str_node->nodes[0]->line ,assigned_val_type);
			exit(1);
		}
	}
	if(strcmp(assigned_val_type,"CHAR") && str_node->nodes[0]->count != 0 && !strcmp(str_node->nodes[0]->nodes[0]->token, "INDEX")){
		printf("Error: Line %d: Assignment type mismatch - expected 'CHAR' for string array cell, but found '%s'.\n", str_node->nodes[0]->line ,assigned_val_type);
		exit(1);
	}
	if (str_node->nodes[0]->count != 0 && !strcmp(str_node->nodes[0]->nodes[0]->token, "INDEX")){
		char* indexType = checkExpAndReturnItsType(str_node->nodes[0]->nodes[0]->nodes[0]);
		if(strcmp("INT", indexType)){
			printf("Error: Line %d: Size of string must be type 'INT' not '%s'\n", str_node->nodes[0]->line ,indexType);
			exit(1);
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
 * printSymbolTable - Prints the symbol table for debugging and analysis.
 * This function traverses the symbol table from the scope_stack_top scope and prints information about each symbol,
 * including its identifier, type, value, and the scope in which it is declared. It helps visualize the structure
 * and contents of the symbol table.
 * @param node: The scope_stack_top scope node in the symbol table hierarchy.
 */
void printSymbolTable(scopeNode *node)
{  
   scopeNode *curr_scope = node;
   symbolNode *currentSymbol;
   while(curr_scope != NULL){
		currentSymbol = curr_scope->symbolTable;
		while (currentSymbol != NULL) {
			printf("ID: %s\t|\tVariable Type: %s\t|\tValue: %s\t|\tLocated In Scope: %d", currentSymbol->id, currentSymbol->type, currentSymbol->data,currentSymbol->scopeID);
			printf("\n");
			currentSymbol = currentSymbol->next;
		}
	curr_scope=curr_scope->next;       
	}
}

