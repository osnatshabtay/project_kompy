#include "semantic.h"

scope* SCOPE_STACK_TOP = NULL;
int NUM_OF_MAIN_FUNCTIONS = 0;
char* CURR_FUNCTION = NULL;
char* CALLED_FUNCTIONS[100];
char* HAS_CALLED_FUNCTIONS[100];
int CALLED_FUNCTIONS_INDEX = 0;
int HAS_CALLED_FUNCTIONS_INDEX = 0;

node* makeNode(char* token) {
	node *newnode = (node*)malloc(sizeof(node));
	char *new_token = (char*)malloc(sizeof(token) + 1);
	strcpy(new_token, token);
	newnode->token = new_token;
	newnode->sons_nodes = NULL;
	newnode->sons_count = 0;
	newnode->node_type = NULL;
	newnode->line_number = 0;
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
	int new_count = first_node->sons_count + second_node->sons_count;
	if (first_node->sons_count == 0) // leaf
		new_count += 1;
	if (second_node->sons_count == 0) // leaf
		new_count += 1;

	combined_node->token = strdup(token);
	combined_node->sons_count = new_count;
	combined_node->sons_nodes = (node**)malloc(sizeof(node*) * combined_node->sons_count);
	if (first_node->sons_count == 0)
		combined_node->sons_nodes[j++] = first_node;
	else {
		for (i = 0; i < first_node->sons_count; i++) {
			combined_node->sons_nodes[j] = first_node->sons_nodes[i];
			j++;
		}
		freeNode(first_node, 0);
	}
	if (second_node->sons_count == 0)
		combined_node->sons_nodes[j++] = second_node;
	else {
		for (i = 0; i < second_node->sons_count; i++) {
			combined_node->sons_nodes[j] = second_node->sons_nodes[i];
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
	if ((*father)->sons_count != 0) {
		(*father)->sons_nodes = (node**)realloc((*father)->sons_nodes, ((*father)->sons_count+1)*sizeof(node*));
		(*father)->sons_nodes[(*father)->sons_count] = son;
		(*father)->sons_count++;
	}
	else {
		(*father)->sons_nodes = (node**)malloc(sizeof(node*));
		(*father)->sons_nodes[(*father)->sons_count] = son;
		(*father)->sons_count++;
	}
}

void addSonsNodesToFatherNode(node* father, node* sons){
	addSonNodeToFatherNode(&father, sons);
	for (int i = 0; i < sons->sons_count; i++){
		addSonNodeToFatherNode(&father, sons->sons_nodes[i]);
	}
	
}

void addNodesList(node* add_to, node* to_add){
	if (to_add->sons_count != 0){
		int i;
		for (i = 0; i < to_add->sons_count; i++){
			addSonNodeToFatherNode(&add_to, to_add->sons_nodes[i]);
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

	if (curr_node->sons_nodes) {
		for (int j = 0; j < curr_node->sons_count; j++) {
			printTree(curr_node->sons_nodes[j], num_of_spaces + 1);
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
	for(i = 0; i < node_to_free->sons_count && free_sons == 1; i++){
		free(node_to_free->sons_nodes[i]);
	}

	free(node_to_free);
}

void pushStatementToStack(node* root, int scope_level){
	if (root == NULL)
		return;
	
	switch (root->token[0]) {
		case 'F': // FUNCTION or FOR
			if (!strcmp(root->token, "FUNCTION")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, root->sons_nodes[1], root->sons_nodes[3]->sons_nodes, scope_level, root->sons_nodes[3]->sons_count);
				isValidReturnType(root);
			}
			else if (!strcmp(root->token, "FOR")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes[3]->sons_nodes, scope_level, root->sons_nodes[3]->sons_count);
				char* initType = checkExpAndReturnItsType(root->sons_nodes[0]);
				if(strcmp("INT" ,initType)){
					printf("Error Line %d: FOR init incorrect.\n", root->sons_nodes[0]->line_number);
					exit(1);
				}
				char* exp_type = checkExpAndReturnItsType(root->sons_nodes[1]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error Line %d FOR condition incorrect'.\n", root->sons_nodes[0]->line_number);
					exit(1);
				}
				char* incType = checkExpAndReturnItsType(root->sons_nodes[2]);
				if(strcmp("INT" ,incType)){
					printf("Error Line %d FOR increment incorrect.\n", root->sons_nodes[0]->line_number);
					exit(1);
				}
			}
			break;

		case 'S': // STATIC
			if (!strcmp(root->token, "STATIC")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, root->sons_nodes[1], root->sons_nodes[3]->sons_nodes, scope_level, root->sons_nodes[3]->sons_count);
				isValidReturnType(root);
			}
			break;

		case 'W': // WHILE
			if (!strcmp(root->token, "WHILE")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes[1]->sons_nodes, scope_level, root->sons_nodes[1]->sons_count);
				char* exp_type = checkExpAndReturnItsType(root->sons_nodes[0]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error Line %d: WHILE condition incorrect'.\n", root->line_number);
					exit(1);
				}
			}
			break;

		case 'D': // DO-WHILE
			if (!strcmp(root->token, "DO-WHILE")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes[0]->sons_nodes, scope_level, root->sons_nodes[0]->sons_count);
				char* exp_type = checkExpAndReturnItsType(root->sons_nodes[1]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error Line %d: DO-WHILE condition incorrect.\n", root->line_number);
					exit(1);
				}
			}
			break;

		case 'C': // CODE
			if (!strcmp(root->token, "CODE")) {
				scope_level++;
        		pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes, scope_level, root->sons_count);
			}
			break;

		case 'M': // MAIN
			if (!strcmp(root->token, "MAIN")) {
				scope_level++;
				NUM_OF_MAIN_FUNCTIONS++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes[0]->sons_nodes, scope_level, root->sons_nodes[0]->sons_count);
			}
			break;

		case 'I': // IF
			if (!strcmp(root->token, "IF")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes[1]->sons_nodes, scope_level, root->sons_nodes[1]->sons_count);
				char* exp_type = checkExpAndReturnItsType(root->sons_nodes[0]);
				if(strcmp("BOOL" ,exp_type)){
					printf("Error Line %d IF condition incorrect'.\n",root->line_number);
					exit(1);
				}
			}
			break;

		case 'B': // BLOCK
			if (!strcmp(root->token, "BLOCK")) {
				scope_level++;
				pushScopeToScopeStack(&SCOPE_STACK_TOP, NULL, root->sons_nodes, scope_level, root->sons_count);
			}
			break;

		default:
			break;
	}

	for (int i = 0; i < root->sons_count; i++){
		pushStatementToStack(root->sons_nodes[i], scope_level);
	}
}

void pushScopeToScopeStack(scope** scope_stack_top, node* scope_params, node** statements, int scope_level, int num_of_statements){      
	scope* new_scope = (scope*) malloc(sizeof(scope));
	new_scope->scope_level = scope_level-1;
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
			addSymbolToSymbolTable(&SCOPE_STACK_TOP, statements[i]->sons_nodes[1], statements[i]->sons_nodes[0]->token, statements[i]->sons_nodes[2]->token, 1, 0);
		}

		else if (!strcmp(statements[i]->token, "STATIC")) {
			addSymbolToSymbolTable(&SCOPE_STACK_TOP, statements[i]->sons_nodes[1], statements[i]->sons_nodes[0]->token, statements[i]->sons_nodes[2]->token, 1, 1);
		}

		else if (!strcmp(statements[i]->token, "FUNCTION_CALL")){
			checkFunctionCall(statements[i]->sons_nodes[1], statements[i]->sons_nodes[0]->token);
		}

		else if(!strcmp(statements[i]->token, "<-")){
        	if (isVarDeclared(statements[i]->sons_nodes[0]->token) && strcmp(statements[i]->sons_nodes[0]->token, "POINTER")){
                char *left = scopeSearch(statements[i]->sons_nodes[0]->token)->type;
                char *right = checkExpAndReturnItsType(statements[i]->sons_nodes[1]);
				if (!strcmp(left, "STRING"))
					checkStringAssignment(statements[i], right);
				else if (statements[i]->sons_nodes[0]->sons_count > 0){
					printf("Error Line %d %s index need to be int.\n", statements[i]->sons_nodes[0]->line_number, left);
					exit(1);
				}
				else if (!strcmp(right, "NULL") && (strcmp(left, "INT*") && strcmp(left, "CHAR*") && strcmp(left, "DOUBLE*")) && strcmp(left, "FLOAT*")){
					printf("Error Line %d can not assign %s to %s\n", statements[i]->line_number, right, left);
					exit(1);
				}
			}
			
			else if (!strcmp(statements[i]->sons_nodes[0]->token, "POINTER") && isVarDeclared(statements[i]->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->token))
				isValidPrtAssinment(statements[i]);
			
			else{
				checkExpAndReturnItsType(statements[i]->sons_nodes[1]);
				printf("Error Line %d Undeclared variable [%s]\n", statements[i]->line_number, statements[i]->sons_nodes[0]->token);
				exit(1);
			}
		}
	}
}

void pushSymbolsToSymbolTable(node* var_declaration_nosde){
	int i;
	int j;
	for(i = 0; i<var_declaration_nosde->sons_count; i++){

		int num_of_vars = var_declaration_nosde->sons_nodes[i]->sons_count;
		char* var_type = var_declaration_nosde->sons_nodes[i]->token;
		node** vars_declared = var_declaration_nosde->sons_nodes[i]->sons_nodes;

		for(int j = 0; j < num_of_vars; j++){
			if ((!strcmp(vars_declared[j]->token, "<-") && vars_declared[j]->sons_nodes[1]->node_type != NULL && !strcmp("NULL", vars_declared[j]->sons_nodes[1]->node_type)))
				if (strcmp(var_type, "INT*") && strcmp(var_type, "CHAR*") && strcmp(var_type, "DOUBLE*") && strcmp(var_type, "FLOAT*")){
					printf("Error Line %d can not assign NULL to %s\n", vars_declared[j]->line_number, var_type);
					exit(1);
				}
				else
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, NULL, vars_declared[j]->sons_nodes[0]->token, var_type, 0, 0);
			
			else if ((!strcmp(vars_declared[j]->token, "<-") && strcmp(var_type, "STRING") == 0)){
				char* exp_type = checkExpAndReturnItsType(vars_declared[j]->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]);
				if(strcmp("INT", exp_type)){
					printf("Error Line %d Size need be INT \n", vars_declared[j]->sons_nodes[0]->line_number);
					exit(1);
				}
				else {
					exp_type = checkExpAndReturnItsType(vars_declared[j]->sons_nodes[1]);
					if (!strcmp(var_type, exp_type))
						addSymbolToSymbolTable(&SCOPE_STACK_TOP, NULL, vars_declared[j]->sons_nodes[0]->token, var_type, 0, 0);
					else{
						printf("Error Line %d can not assign %s to %s\n", vars_declared[j]->sons_nodes[0]->line_number, exp_type, var_type);
						exit(1);
					}
				}
			}

			else if (strcmp(vars_declared[j]->token, "<-") && strcmp(var_type, "STRING") == 0){
				char* exp_type = checkExpAndReturnItsType(vars_declared[j]->sons_nodes[0]->sons_nodes[0]);
				if(strcmp("INT", exp_type)){
					printf("Error Line %d Size need be INT'\n", vars_declared[j]->line_number);
					exit(1);
				}
				else
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, NULL, vars_declared[j]->token, var_type, 0, 0);
			}
			
			else{
				if (strcmp(vars_declared[j]->token, "<-"))
					addSymbolToSymbolTable(&SCOPE_STACK_TOP, NULL, vars_declared[j]->token, var_type, 0, 0);
				else{
					char* exp_type = checkExpAndReturnItsType(vars_declared[j]->sons_nodes[1]);
					if (!strcmp(var_type, exp_type))
						addSymbolToSymbolTable(&SCOPE_STACK_TOP, NULL, vars_declared[j]->sons_nodes[0]->token, var_type, 0, 0);
					else {
						printf("Error Line %d can not assign %s to %s\n", vars_declared[j]->sons_nodes[0]->line_number, exp_type, var_type);
						exit(1);
					}
				}	
			}
		}
	}
}

void addSymbolToSymbolTable(scope** scope_stack_top, node* params, char* symbol_id, char* symbol_type, int is_func, int is_static) {
	symbol* new_node = (symbol*) malloc(sizeof(symbol));
	new_node->next =(*scope_stack_top)->symbol_table;
	(*scope_stack_top)->symbol_table = new_node;

	new_node->id = (char*)(malloc (sizeof(symbol_id) + 1));
	strncpy(new_node->id, symbol_id, sizeof(symbol_id)+1);

	new_node->type = (char*)(malloc (sizeof(symbol_type) + 1));
	strncpy(new_node->type, symbol_type, sizeof(symbol_type)+1);

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
	symbol* symbol_node = scopeSearch(var_name);
	return (symbol_node == NULL) ? 0 : 1;
}

symbol* symbolSearch (symbol* symbol_table, char* id){
	symbol* curr_symbol_table;
	for(curr_symbol_table = symbol_table; curr_symbol_table != NULL; curr_symbol_table = curr_symbol_table->next){
		if (!strcmp(curr_symbol_table->id, id)){
            return curr_symbol_table;
        }
	}
	return NULL;
}

symbol* scopeSearch(char* id) {
    scope* curr_scope = SCOPE_STACK_TOP;
	int curr_level;

    while (curr_scope != NULL) {
        symbol* found_symbol = symbolSearch(curr_scope->symbol_table, id);
        if (found_symbol != NULL) {
            return found_symbol;  
        }

		curr_level = curr_scope->scope_level;
        if (curr_level == 0) {
            break;
        }

        // skip scopes that are not direct parents by comparing scope levels
        curr_scope = curr_scope->next;
        while (curr_scope != NULL && curr_scope->scope_level >= curr_level) {
            curr_scope = curr_scope->next;
        }
    }

    return NULL;
}

char* checkExpAndReturnItsType(node* exp){
	if (exp->node_type != NULL && !strcmp(exp->node_type, "NULL"))
		return "NULL";

	else if (exp->node_type != NULL && !strcmp(exp->node_type, "ID")){
		symbol* symbol_node = scopeSearch(exp->token);
		if(symbol_node != NULL){
			if(!strcmp(symbol_node->type, "STRING") && exp->sons_count > 0){
				char* indexType = checkExpAndReturnItsType(exp->sons_nodes[0]->sons_nodes[0]);
				if(strcmp("INT", indexType)){
					printf("Error Line %d Size of string need to be INT.\n", exp->line_number);
					exit(1);
				}
			return "CHAR";
			}
			return symbol_node->type;
		}
        else{
			printf("Error Line %d Undeclared variable '%s'.\n", exp->line_number, exp->token);
			exit(1);
		}
	}

	else if (exp->node_type != NULL)
		return exp->node_type;

	else if(!strcmp(exp->token, "<-")){
        	if (isVarDeclared(exp->sons_nodes[0]->token)){
                char *left = scopeSearch(exp->sons_nodes[0]->token)->type;
                char *right = checkExpAndReturnItsType(exp->sons_nodes[1]);
            	if (strcmp(right, "NULL") && strcmp(right, left)){
                    printf("Error Line %d cannot assign '%s' to '%s'.\n", exp->line_number, right, left);
					exit(1);
				}
				else
					return left;
			}
		}

	else if (!strcmp(exp->token,"&")){
		char* left;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
		if(isArithmeticType(left) || !strcmp(left,"CHAR")){
			char* result = malloc(strlen(left) + 2); // +2 for the '*' and the null terminator
			if (result == NULL) {
				printf("Memory allocation failed.\n");
				exit(1);
			}
			strcpy(result, left);
			strcat(result, "*");
			return result;
		}
		else{
			printf("Error Line %d Cannot use '&' on '%s'\n", exp->sons_nodes[0]->line_number, left);
			exit(1);
		}
	}

	else if (!strcmp(exp->token,"LEN")){
		char* left;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
		if(!strcmp(left,"STRING"))
            return "INT";
		else{
			printf("Error Line %d Cannot use len opertaor on '%s'\n", exp->sons_nodes[0]->line_number, left);
			exit(1);
		}
	}	
	
	else if(!strcmp(exp->token, "POINTER")){
        char* left;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]->sons_nodes[0]);
		char* base_type = getPointerBaseType(left);
        if(!strcmp(left, "NULL")){
			printf("Error Line %d '%s' is not pointer\n", exp->sons_nodes[0]->sons_nodes[0]->line_number ,left);
			exit(1);
		}
		return base_type;
	}

	else if(!strcmp(exp->token, "NOT")){
        char* left;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
        if(!strcmp(left,"BOOL"))
            return "BOOL";
		else{
			printf("Error Line %d: Cannot use ! on '%s' \n", exp->line_number ,left);
			exit(1);
		}
	}

	else if (!strcmp(exp->token, "FUNCTION_CALL")){
		if(checkFunctionCall(exp->sons_nodes[1], exp->sons_nodes[0]->token)){
			symbol* func_symbol = scopeSearch(exp->sons_nodes[0]->token);
			return func_symbol->type;
		}
	}

	else if(!strcmp(exp->token, "==") || !strcmp(exp->token, "!=")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
        right = checkExpAndReturnItsType(exp->sons_nodes[1]);
        if (isEqualType(left, right)) {
            return "BOOL";
        }
		else{
			printf("Error Line %d Cannot use '%s' between '%s' and '%s' \n", exp->line_number ,exp->token, left, right);
			exit(1);
		}
	}

	else if(!strcmp(exp->token, "&&") || !strcmp(exp->token, "||")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
        right = checkExpAndReturnItsType(exp->sons_nodes[1]);
        if(!strcmp(left,"BOOL") && !strcmp(right,"BOOL"))
            return "BOOL";
		else {
			printf("Error Line %d Cannot use '%s' between '%s' and '%s' \n", exp->line_number, exp->token, left, right);
			exit(1);
		}
	}

	else if(!strcmp(exp->token, ">") || !strcmp(exp->token, "<") || !strcmp(exp->token, ">=") || !strcmp(exp->token, "<=")){
        char* left, *right;
        left = checkExpAndReturnItsType(exp->sons_nodes[0]);
        right = checkExpAndReturnItsType(exp->sons_nodes[1]);
        if(isCompatibleForComparison(left, right))
            return "BOOL";
		else{
			printf("Error Line %d Cannot use '%s' between '%s' and '%s'\n", exp->line_number,exp->token, left, right);
			exit(1);
		}
	} 
	
	else if (!strcmp(exp->token, "*") || !strcmp(exp->token, "/") || !strcmp(exp->token, "+") || !strcmp(exp->token, "-")){
		char* left_exp, *right_exp;
        left_exp = checkExpAndReturnItsType(exp->sons_nodes[0]);
        right_exp = checkExpAndReturnItsType(exp->sons_nodes[1]);
		if (!strcmp(left_exp, "NULL") || !strcmp(right_exp, "NULL"))
			return "NULL";
        if (isArithmeticType(left_exp) && isArithmeticType(right_exp)) {
            return getArithmeticResultType(exp, left_exp, right_exp);
        }
		else{
			printf("Error Line %d Cannot use '%s' between '%s' and '%s'\n", exp->line_number, exp->token, left_exp, right_exp);
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

char* getArithmeticResultType(node* exp, char* left, char* right) {
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
		printf("Error Line %d Cannot use '%s' operation between '%s' and '%s'\n", exp->line_number, exp->token, left, right);
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

int isValidReturnType(node* func_node){
    char* func_type = func_node->sons_nodes[2]->token;
    int is_valit_ret_statement = isValidReturnStatement(func_node->sons_nodes[3], func_type);
	if (is_valit_ret_statement == 0){
		if (!strcmp(func_type,"VOID")){
        	printf ("Error Line %d Void function can not return value.\n", func_node->line_number);
		}
		else{
        	printf ("Error Line %d Function %s returns an invalid value.\n", func_node->line_number ,func_node->sons_nodes[0]->token);
		}
		exit(1);
	}	
    return 1;
}

int isValidReturnStatement(node* func_node, char* expected_ret_type){
	for (int i = 0; i < func_node->sons_count; i++){
		if(!strcmp(func_node->sons_nodes[i]->token, "RETURN")){
			if (func_node->sons_nodes[i]->sons_count > 0){
				char* actual_ret_type = checkExpAndReturnItsType(func_node->sons_nodes[i]->sons_nodes[0]);
				if (strcmp(actual_ret_type, expected_ret_type))
					return 0;
			}
			else if(func_node->sons_nodes[i]->sons_count == 0 && strcmp(expected_ret_type, "VOID")){
				return 0;
			}
		}
		if(strcmp(func_node->sons_nodes[i]->token, "FUNCTION"))
			if(isValidReturnStatement(func_node->sons_nodes[i], expected_ret_type) == 0)
				return 0;
	}
	return 1;
}

void isValidPrtAssinment(node* ptr_node){
	char *left = checkExpAndReturnItsType(ptr_node->sons_nodes[0]);
	char *right = checkExpAndReturnItsType(ptr_node->sons_nodes[1]);
	if (strcmp(right,left)){
		printf("Error Line %d can not assign %s to %s\n", ptr_node->line_number, right, left);
		exit(1);
	}
}

void checkForSymbolsDuplications(scope* scope_node){
	scope* curr_scope;
	symbol* s1;
	symbol* s2;
	for(curr_scope = scope_node; curr_scope != NULL; curr_scope = curr_scope->next){
		for(s1 = scope_node->symbol_table; s1 != NULL; s1 = s1->next){
			for(s2 = s1->next; s2 != NULL; s2 = s2->next){
				if (!strcmp(s1->id, s2->id)){
					if (s1->is_func){
						printf ("function name is not valid -%s\n", s1->id);
						exit(1);
					}
					else{
						printf ("variable name is not valid - %s\n", s1->id);
						exit(1);
					}
				}
			}
		}
	}
}

int checkFunctionCall(node* func_args, char* func_name){
    symbol *func_symbol = scopeSearch(func_name);
	if (func_symbol != NULL)
		if (checkFunctionArgs(func_symbol->params, func_args))
			return 1;
    printf ("Error Line %d call to this function '%s' without match arguements\n", func_args->line_number, func_name);
	exit(1);
}

void checkStaticNonStaticCallsViolation() {
    int i = 0, j = 0;
    for (j = 0; j < HAS_CALLED_FUNCTIONS_INDEX; j++) {
        symbol* called_func_symbol = scopeSearch(HAS_CALLED_FUNCTIONS[j]);
        if (called_func_symbol != NULL) {
            for (i = 0; i < CALLED_FUNCTIONS_INDEX; i++) {
                symbol* symbol_of_calling_func = scopeSearch(CALLED_FUNCTIONS[i]);

                if (symbol_of_calling_func != NULL) {
                    if (symbol_of_calling_func->is_static && !called_func_symbol->is_static) {
                        printf("Error Static function cannot call non-static function .\n");
                        exit(1);
                    }
				}
			}
		}
    }
}

void checkMainNonStaticCalls(node* tree) {
    if (tree == NULL){
		return;
	}

    char* token = tree->token;
    static int inMain = 0;

    // detect entering the 'main' function
    if (strcmp(token, "MAIN") == 0) {
        inMain = 1;
    }

    if (strcmp(token, "FUNCTION_CALL") == 0) {
        if (inMain) {
            symbol* called_func_symbol = scopeSearch(tree->sons_nodes[0]->token);
            if (called_func_symbol != NULL && !called_func_symbol->is_static) {
                printf("Error Static function cannot call non-static function .\n");
                exit(1);
            }
        }
    }

    for (int j = 0; j < tree->sons_count; j++) {
        checkMainNonStaticCalls(tree->sons_nodes[j]);
    }

    // detect exiting the 'main' function
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
	for (int i = 0; i < func_params->sons_count; i++)
		expected_param_count += func_params->sons_nodes[i]->sons_count;
	if(expected_param_count != func_args->sons_count)
		return 0;

	// type matching
    for (int i = 0, k = 0; i < func_params->sons_count; i++) {
        char* expected_type = func_params->sons_nodes[i]->token;
        for (int j = 0; j < func_params->sons_nodes[i]->sons_count; j++, k++) {
            if (strcmp(expected_type, checkExpAndReturnItsType(func_args->sons_nodes[k]))) {
                return 0;
            }
        }
    }
	return 1;
}

void checkStringAssignment(node* str_node, char* assigned_val_type){
	if (str_node->sons_nodes[0]->sons_count == 0){
		if(strcmp("STRING", assigned_val_type)){
			printf("Error Line %d can not assign '%s' to be STRING\n", str_node->sons_nodes[0]->line_number ,assigned_val_type);
			exit(1);
		}
	}
	if(strcmp(assigned_val_type,"CHAR") && str_node->sons_nodes[0]->sons_count != 0 && !strcmp(str_node->sons_nodes[0]->sons_nodes[0]->token, "INDEX")){
		printf("Error Line %d can not do this action.\n", str_node->sons_nodes[0]->line_number);
		exit(1);
	}
	if (str_node->sons_nodes[0]->sons_count != 0 && !strcmp(str_node->sons_nodes[0]->sons_nodes[0]->token, "INDEX")){
		char* indexType = checkExpAndReturnItsType(str_node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]);
		if(strcmp("INT", indexType)){
			printf("Error Line %d  Size of string neet be INT\n", str_node->sons_nodes[0]->line_number);
			exit(1);
		}
	}
	
}

void findCalledFunctions(node* tree) {
    if (tree == NULL) 
		return;

    char* token = tree->token;
	if (!strcmp(token, "FUNCTION_CALL")) {
        if (CURR_FUNCTION) {
			HAS_CALLED_FUNCTIONS[HAS_CALLED_FUNCTIONS_INDEX++] = tree->sons_nodes[0]->token;
            CALLED_FUNCTIONS[CALLED_FUNCTIONS_INDEX++] = CURR_FUNCTION; 
        }
    }
    else if (!strcmp(token, "FUNCTION") || !strcmp(token, "STATIC")) {
        if (tree->sons_nodes[0] && tree->sons_nodes[0]->token) {
            CURR_FUNCTION = tree->sons_nodes[0]->token;
		}
    }

    for (int j = 0; j < tree->sons_count; j++) {
        findCalledFunctions(tree->sons_nodes[j]);
    }
}
