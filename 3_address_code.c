#include "3_address_code.h"
#define BUFFER_SIZE 5000
int GLOBAL_VAR_COUNT = 0;
int GLOBAL_LABEL_COUNT = 1;


// ----------------------- helpers -----------------------
char* removeLeadingAndTralingSpaces(const char* str) {
    while (isspace((unsigned char)*str)) {
		str++;
	}
    if (*str == '\0'){
		return (char*)str;
	} 

    const char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)){
		end--;
	} 
    size_t len = end - str + 1;
    char* without_sapces = (char*)malloc(len + 1);
    if (without_sapces == NULL) {
        return NULL;
    }
    strncpy(without_sapces, str, len);
    without_sapces[len] = '\0';
    return without_sapces;
}

int isBooleanOperator(node* node){
	if (strcmp(node->token, "==") == 0 || strcmp(node->token, "<") == 0 || strcmp(node->token, ">") == 0 
	|| strcmp(node->token, "<=") == 0 || strcmp(node->token, ">=") == 0 || strcmp(node->token, "!=") == 0
	|| strcmp(node->token, "&&") == 0 || strcmp(node->token, "||") == 0) {
		return 1;
	}
	return 0;

}

void generateNewVar(node* node){
	char new_var[10];
	sprintf(new_var, "t%d", GLOBAL_VAR_COUNT++);
	node->var = strdup(new_var);
}

char* generateNewLabel(){
	char new_label[10] = "";
	sprintf(new_label ,"L%d", GLOBAL_LABEL_COUNT++);
	char* label = strdup(new_label);
	return label;
}

void addVar(node* node, char* var){
	node->var = strdup(var);
}

void addCode(node* node, char* code){
	
	char buffer[BUFFER_SIZE] = "";

	if (!strcmp(node->token, "STATIC")){
		sprintf(buffer, "%s:\n%s", node->sons_nodes[0]->token,"\tBeginFunc\n");
		node->code = strdup(buffer);
	}

	else if (!strcmp(node->token, "MAIN")){
		sprintf(buffer, "%s", "main:\n\tBeginFunc\n");
		node->code = strdup(buffer);
	}

	else if (!strcmp(node->token, "FUNCTION")){
		sprintf(buffer, "%s:\n%s", node->sons_nodes[0]->token,"\tBeginFunc\n");
		node->code = strdup(buffer);
	}

	
	if (node->sons_count > 0){
		for (int i = 0; i< node->sons_count; i++){
			if (strcmp(node->sons_nodes[i]->code,"")){
				sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[i]->code);
			}
		}
	}
	sprintf(buffer + strlen(buffer), "%s", code);
	node->code = strdup(buffer);

	if (!strcmp(node->token, "MAIN") || !strcmp(node->token, "FUNCTION") || !strcmp(node->token, "STATIC")){
		sprintf(buffer + strlen(buffer), "\t%s", "EndFunc\n\n");
		node->code = strdup(buffer);
	}
}

int sizeOfVar(node* args){
	int size = 0;
	for(int i = 0; i < args->sons_count; i++){
		if (!strcmp(args->sons_nodes[i]->node_type, "INT") || !strcmp(args->sons_nodes[i]->node_type, "BOOL"))
			size+=4;
		else if (!strcmp(args->sons_nodes[i]->node_type, "INT*") || !strcmp(args->sons_nodes[i]->node_type, "CHAR*") 
		|| !strcmp(args->sons_nodes[i]->node_type, "DOUBLE*") || !strcmp(args->sons_nodes[i]->node_type, "FLOAT*") ||!strcmp(args->sons_nodes[i]->node_type, "DOUBLE")
		||!strcmp(args->sons_nodes[i]->node_type, "FLOAT"))
			size+=8;
		else if (!strcmp(args->sons_nodes[i]->node_type, "CHAR"))
			size+=1;
	}
	return size;
}

char* extractCondition(const char* str) {
    const char* prefix = "(null) = ";
    size_t prefix_len = strlen(prefix);

    char* cond = removeLeadingAndTralingSpaces(str);
    if (cond == NULL) {
        return NULL;
    }
    if (strncmp(cond, prefix, prefix_len) != 0) {
        free(cond);
        return NULL;
    }
    const char* start = cond + prefix_len;
    char* cond_return = removeLeadingAndTralingSpaces(start);
    free(cond);

    if (cond_return == NULL || *cond_return == '\0') {
        return NULL;
    }

    return cond_return;
}

void removeStringFromCode(char* str, const char* to_remove) {
    size_t nullStrLen = strlen(to_remove);
    char *match;
    while ((match = strstr(str, to_remove)) != NULL) {
        memmove(match, match + nullStrLen, strlen(match + nullStrLen) + 1);
    }
}


// ----------------------- loops related -----------------------
void generateForAs3AC(node* node){
    char buffer[BUFFER_SIZE] = ""; 
    char* label1 = generateNewLabel(); 
    char* label2 = generateNewLabel(); 
	char* label3 = generateNewLabel(); 
	
	if (strcmp(node->sons_nodes[0]->code,"")){
		sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[0]->code);
		sprintf(buffer + strlen(buffer), "%s:", label1);

		char* cond = (!strcmp(node->sons_nodes[0]->code, "truetoremove")) ? "true" : (!strcmp(node->sons_nodes[0]->code, "falsetoremove")) ? "false" : extractCondition(node->sons_nodes[1]->code);
    	sprintf(buffer + strlen(buffer), "\tif %s Goto %s\n", cond, label2);
		sprintf(buffer + strlen(buffer), "\tGoto %s\n",  label3);
	}
	
	sprintf(buffer + strlen(buffer), "%s:", label2);
	sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[2]->code);
	sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[3]->code);
	sprintf(buffer + strlen(buffer), "\tGoto %s\n",  label1);
	sprintf(buffer + strlen(buffer), "%s:", label3);
	node->code = strdup(buffer);
}

void generateWhileAs3AC(node* node) {
    char buffer[BUFFER_SIZE] = ""; 
    char* label1 = generateNewLabel(); 
    char* label2 = generateNewLabel(); 
    char* label3 = generateNewLabel(); 
	char* cond = (!strcmp(node->sons_nodes[0]->code, "truetoremove")) ? "true" : (!strcmp(node->sons_nodes[0]->code, "falsetoremove")) ? "false" : extractCondition(node->sons_nodes[0]->code);

    sprintf(buffer, "%s:", label1);
    sprintf(buffer + strlen(buffer), "\tif %s Goto %s\n", cond, label2);
    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label3);
    sprintf(buffer + strlen(buffer), "%s:", label2);
    
	if (strcmp(node->sons_nodes[1]->code, "")) {
        sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[1]->code);
    }

    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label1);
    sprintf(buffer + strlen(buffer), "%s:", label3);
    node->code = strdup(buffer);
}

void generateDoWhileAs3AC(node* node) {
    char buffer[BUFFER_SIZE] = "";
    char* label1 = generateNewLabel(); 
    char* label2 = generateNewLabel(); 
	char* label3 = generateNewLabel();

    sprintf(buffer, "%s:", label1);
    if (strcmp(node->sons_nodes[0]->code, "")) {
        sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[0]->code);
    }

	char* cond = (!strcmp(node->sons_nodes[0]->code, "truetoremove")) ? "true" : (!strcmp(node->sons_nodes[0]->code, "falsetoremove")) ? "false" : extractCondition(node->sons_nodes[1]->code);
    sprintf(buffer + strlen(buffer), "\tif %s Goto %s\n", cond, label1);
    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label2);
    sprintf(buffer + strlen(buffer), "%s:", label2);
    node->code = strdup(buffer);
}


// ----------------------- if else related -----------------------
void generateIfAs3AC(node* node){
    char buffer[BUFFER_SIZE] = ""; 
    char* label1 = generateNewLabel();
    char* label2 = generateNewLabel();
	char* cond = (!strcmp(node->sons_nodes[0]->code, "truetoremove")) ? "true" : (!strcmp(node->sons_nodes[0]->code, "falsetoremove")) ? "false" : extractCondition(node->sons_nodes[0]->code);

    sprintf(buffer + strlen(buffer), "\tif %s Goto %s\n", cond, label1);
    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label2);
	sprintf(buffer + strlen(buffer), "%s:", label1);
	
	if (strcmp(node->sons_nodes[1]->code, "")) {
        sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[1]->code);
    }

	sprintf(buffer + strlen(buffer), "%s:", label2);
    node->code = strdup(buffer);
}

void generateIfElseAs3AC(node* node){
    char buffer[BUFFER_SIZE] = ""; 
    char* label1 = generateNewLabel(); 
    char* label2 = generateNewLabel(); 
    char* label3 = generateNewLabel(); 
	char* cond = (!strcmp(node->sons_nodes[0]->code, "truetoremove")) ? "true" : (!strcmp(node->sons_nodes[0]->code, "falsetoremove")) ? "false" : extractCondition(node->sons_nodes[0]->code);

    sprintf(buffer + strlen(buffer), "\tif %s Goto %s\n", cond, label1);
    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label2);
	sprintf(buffer + strlen(buffer), "%s:", label1);

	if (strcmp(node->sons_nodes[1]->code, "")) {
        sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[1]->code);
    }

    sprintf(buffer + strlen(buffer), "\tGoto %s\n", label3);
	sprintf(buffer + strlen(buffer), "%s:", label2);

    if (strcmp(node->sons_nodes[2]->code, "")) {
        sprintf(buffer + strlen(buffer), "%s", node->sons_nodes[2]->code);
    }

    sprintf(buffer + strlen(buffer), "%s:", label3);
    node->code = strdup(buffer);
}


// ----------------------- functions related -----------------------
void generateFunctionAs3AC(node* node, char* buffer, int flag){
	if (!strcmp(node->node_type, "VOID")){
		sprintf(buffer + strlen(buffer), "\tLCall %s\n", node->token);
	} 
	else{
		generateNewVar(node);
		if (flag)
			sprintf(buffer + strlen(buffer), "\t%s = LCall %s\n", node->var, node->token);
		else
			sprintf(buffer + strlen(buffer), "\tLCall %s\n", node->token);
	}
}

void generateFunctionCallAs3AC(node* node, int flag){
	char buffer[BUFFER_SIZE] = "";
	if (node->sons_nodes[1]->sons_count > 0){
		for (int i = 0;i < node->sons_nodes[1]->sons_count;i++){
			if (strcmp(node->sons_nodes[1]->sons_nodes[i]->node_type, "ID")){
				generateNewVar(node->sons_nodes[1]->sons_nodes[i]);
				sprintf(buffer + strlen(buffer), "\t%s = %s\n", node->sons_nodes[1]->sons_nodes[i]->var, node->sons_nodes[1]->sons_nodes[i]->token);
			}
			sprintf(buffer + strlen(buffer), "\tPushParam %s\n", node->sons_nodes[1]->sons_nodes[i]->var);
		}
		generateFunctionAs3AC(node->sons_nodes[0], buffer, flag);
		node->var = node->sons_nodes[0]->var;
		sprintf(buffer + strlen(buffer), "\tPopParams %d\n", sizeOfVar(node->sons_nodes[1]));
		node->code = strdup(buffer);
	}
	else {
		generateFunctionAs3AC(node->sons_nodes[0], buffer, flag);
		node->var = node->sons_nodes[0]->var;
		node->code = strdup(buffer);
	}
}



// ----------------------- assignments related -----------------------
void generateAssignmentAs3AC(node* node){
	char buffer[BUFFER_SIZE] ="";
	if (node->sons_nodes[1]->sons_count > 0) {
		if (!strcmp(node->sons_nodes[1]->sons_nodes[0]->token, "INDEX")){
			generateNewVar(node->sons_nodes[1]->sons_nodes[0]);
			sprintf(buffer,"\t%s = &%s\n", node->sons_nodes[1]->sons_nodes[0]->var, node->sons_nodes[1]->token);
			generateNewVar(node);
			sprintf(buffer + strlen(buffer),"\t%s = %s + %s\n", node->var, node->sons_nodes[1]->sons_nodes[0]->var, node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->var);
			sprintf(buffer + strlen(buffer),"\t%s = *%s\n", node->sons_nodes[0]->var, node->var);
		}

		else if (!strcmp(node->sons_nodes[1]->token, "LEN")){
			generateNewVar(node->sons_nodes[1]);
			sprintf(buffer,"\t%s = Sizeof(%s)\n", node->sons_nodes[1]->var, node->sons_nodes[1]->sons_nodes[0]->token);
			sprintf(buffer + strlen(buffer),"\t%s = %s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		}

		else if (isBooleanOperator(node->sons_nodes[1])){
			sprintf(buffer + strlen(buffer),"\t%s = %s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		}


		else if (!strcmp(node->sons_nodes[0]->token, "POINTER") || !strcmp(node->sons_nodes[1]->token, "POINTER") || !strcmp(node->sons_nodes[1]->token, "&")){
			generatePointerAs3AC(node);
		}

		else{
			sprintf(buffer,"\t%s = %s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		}
	}
	else {
		if (!strcmp(node->sons_nodes[1]->node_type, "ID")){
			sprintf(buffer,"\t%s = %s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		}

		else {
			generateNewVar(node->sons_nodes[1]);
			sprintf(buffer,"\t%s = %s\n", node->sons_nodes[1]->var, node->sons_nodes[1]->token);
			sprintf(buffer + strlen(buffer),"\t%s = %s\n", node->sons_nodes[0]->token, node->sons_nodes[1]->var);
		}
	}
	addCode(node, buffer);
}

void generateStringAssign3AC(node* node){
	char buffer[BUFFER_SIZE] ="";
	if (strcmp(node->sons_nodes[1]->sons_nodes[0]->token, "INDEX")){
		if (node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->sons_count > 0)
			sprintf(buffer,"%s", node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->code);
		generateNewVar(node->sons_nodes[0]->sons_nodes[0]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[0]->sons_nodes[0]->var, node->sons_nodes[0]->var);
		generateNewVar(node);
		sprintf(buffer + strlen(buffer),"\t%s = %s + %s\n", node->var, node->sons_nodes[0]->sons_nodes[0]->var, node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->var);
		sprintf(buffer + strlen(buffer),"\t*%s = %s\n", node->var, node->sons_nodes[1]->var);
		addCode(node, buffer);
	}
	else
		StringAssignStringAs3AC(node);
}

void StringAssignStringAs3AC(node* ass){

    char buffer[BUFFER_SIZE] =""; 
    node* temp = makeNode("");

    if (ass->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->sons_count > 0)
        sprintf(buffer + strlen(buffer),"%s", ass->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->code);

    generateNewVar(ass->sons_nodes[1]->sons_nodes[0]);
    sprintf(buffer + strlen(buffer),"\t%s = &%s\n", ass->sons_nodes[1]->sons_nodes[0]->var, ass->sons_nodes[1]->var);

    generateNewVar(ass);
    sprintf(buffer + strlen(buffer),"\t%s = %s + %s\n", ass->var, ass->sons_nodes[1]->sons_nodes[0]->var, ass->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->var);

    if (ass->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->sons_count > 0)
        sprintf(buffer + strlen(buffer),"%s", ass->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->code);

    generateNewVar(ass->sons_nodes[0]->sons_nodes[0]);
    sprintf(buffer + strlen(buffer),"\t%s = &%s\n", ass->sons_nodes[0]->sons_nodes[0]->var, ass->sons_nodes[0]->var);

    generateNewVar(temp);
    sprintf(buffer + strlen(buffer),"\t%s = %s + %s\n", temp->var, ass->sons_nodes[0]->sons_nodes[0]->var, ass->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->var);

    sprintf(buffer + strlen(buffer),"\t*%s = *%s\n", temp->var ,ass->var);    

    addCode(ass, buffer);
}


// ----------------------- general -----------------------
void generatePointerAs3AC(node* node){
	char buffer[1000] = "";
	if (node->sons_nodes[0]->node_type!=NULL && !strcmp(node->sons_nodes[0]->node_type, "ID") && node->sons_nodes[1]->sons_count > 0 && !strcmp(node->sons_nodes[1]->token, "POINTER")){
		generateNewVar(node->sons_nodes[1]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[1]->var, node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->var);
		sprintf(buffer + strlen(buffer),"\t%s = *%s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		addCode(node->sons_nodes[0], buffer);
	}

	else if (node->sons_nodes[1]->node_type!=NULL &&!strcmp(node->sons_nodes[1]->node_type, "ID") && node->sons_nodes[0]->sons_count > 0 && !strcmp(node->sons_nodes[0]->token, "POINTER")){
		generateNewVar(node->sons_nodes[0]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[0]->var, node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->var);
		sprintf(buffer + strlen(buffer),"\t*%s = %s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		addCode(node, buffer);
	}

	else if (node->sons_nodes[0]->node_type!=NULL && !strcmp(node->sons_nodes[0]->node_type, "ID") && node->sons_nodes[1]->sons_count > 0 && !strcmp(node->sons_nodes[1]->token, "&")){
		generateNewVar(node->sons_nodes[1]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[1]->var, node->sons_nodes[1]->sons_nodes[0]->var);
		if (!strcmp(node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->token, "INDEX")){
			if (node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->sons_count > 0)
				sprintf(buffer + strlen(buffer),"%s", node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->code);
			generateNewVar(node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]);
			sprintf(buffer + strlen(buffer),"\t%s = %s + %s\n", node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->var, node->sons_nodes[1]->var, node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->var);
		}
		generateNewVar(node);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->var, node->sons_nodes[0]->var);
		sprintf(buffer + strlen(buffer),"\t%s = %s\n", node->sons_nodes[1]->var, node->var);
		addCode(node->sons_nodes[0], buffer);
	}

	else if (!strcmp(node->sons_nodes[0]->token, "POINTER") && !strcmp(node->sons_nodes[1]->token, "POINTER")){
		generateNewVar(node->sons_nodes[1]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[1]->var, node->sons_nodes[1]->sons_nodes[0]->sons_nodes[0]->var);
		generateNewVar(node->sons_nodes[0]);
		sprintf(buffer + strlen(buffer),"\t%s = &%s\n", node->sons_nodes[0]->var, node->sons_nodes[0]->sons_nodes[0]->sons_nodes[0]->var);
		sprintf(buffer + strlen(buffer),"\t*%s = *%s\n", node->sons_nodes[0]->var, node->sons_nodes[1]->var);
		addCode(node, buffer);
	}
}

void generateExpressionAs3AC(node* node, int from_assignment){
	char buffer[BUFFER_SIZE] ="";
	if (isBooleanOperator(node) && !from_assignment) {
		sprintf(buffer,"\t%s = %s %s %s\n", node->var, node->sons_nodes[0]->var, node->token, node->sons_nodes[1]->var);
		addCode(node, buffer);
	}
	else {
		generateNewVar(node);
		sprintf(buffer,"\t%s = %s %s %s\n", node->var, node->sons_nodes[0]->var, node->token, node->sons_nodes[1]->var);
		addCode(node, buffer);
	}
}

void print3AC(node* node){
	const char *to_remove = "falsetoremove";
	removeStringFromCode(node->code, to_remove);

	const char *to_remove4 = "truetoremove";
	removeStringFromCode(node->code, to_remove4);

	const char *to_remove2 = "(null)";
	removeStringFromCode(node->code, to_remove2);

	const char *to_remove3 = "\n\t =";
	removeStringFromCode(node->code, to_remove3);

	printf("%s", node->code);
}