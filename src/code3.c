/*! \file          code3.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         исходный текст функций генератора промежуточного кода
 *
 * Содержит реализацию функций генератора промежуточного кода.
 */

#include "code3.h"
#include "stdio.h"
#include "stdlib.h"

code *gen_code(int op, symbol *arg1, symbol *arg2, symbol *result, int type)
{
	code *ptr = (code *)malloc(sizeof(code));

	ptr -> op = op;
	ptr -> arg1 = arg1;
	ptr -> arg2 = arg2;
	ptr -> type = type;
	ptr -> result = result;
	ptr -> next = NULL;
	ptr -> prev = NULL;
	return ptr;
}

code *merge_code(code *code1,code *code2)
{
	code *tcode = code_tail(code1);

	if (!code1)
		if (code2)
			return code2;
		else
			return NULL;
	else
	{
		tcode -> next = code2;
		if (code2)
			code2 -> prev = tcode;
		return code1;
	}
}

code *code_tail(code *c)
{
	code *ptr = NULL;

	while (c)
	{
		ptr = c;
		c = c -> next;
	}
	return ptr;
}

void destroy_code(code *c)
{
	code *ptr;

	while (c)
	{
		ptr = c -> next;
		free(c);
		c = ptr;
	}
}

//! строковые представления кодов операторов по их номерам
char *opstrs[] = {
"OP_PLUS",
"OP_MINUS",
"OP_MULT",
"OP_DIV",
"OP_MOD",

"OP_ASSIGN",
"OP_GOTO",
"OP_IF",

"OP_GT",
"OP_EQ",
"OP_LW",
"OP_GE",
"OP_LE",
"OP_NEQ",
"OP_NOT",
"OP_AND",
"OP_OR",
"OP_XOR",

"OP_PARAM",
"OP_CALL",

"OP_POW",
"OP_PLUSPLUS",
"OP_MINUSMINUS",

"OP_RET",
"OP_PRINT",
"OP_READ",
"OP_LABEL",
"OP_PRINTLN"
};

char *codestr(code c,char *str)
{
	char arg1str[MAX_SYMBOL_NAME], arg2str[MAX_SYMBOL_NAME];
	union {
		symbol *ptr;
		val_t val;
	} valptr;

	if (c.arg1)
	{
		if (IS_CONST(c.arg1))
		{
			valptr.ptr = c.arg1 -> table;
			if (TYPE_OF(c.arg1) == TYPE_INT)
				sprintf(arg1str,"%d",valptr.val.ival);
			else
				sprintf(arg1str,"%.2f",valptr.val.fval);

		}
		else
			sprintf(arg1str,"%s",c.arg1 -> name);
	}
	else
		sprintf(arg1str,"NULL");

	if (c.arg2)
	{
		if (IS_CONST(c.arg2))
		{
			valptr.ptr = c.arg2 -> table;
			if (TYPE_OF(c.arg2) == TYPE_INT)
				sprintf(arg2str,"%d",valptr.val.ival);
			else
				sprintf(arg2str,"%.2f",valptr.val.fval);

		}
		else
			sprintf(arg2str,"%s",c.arg2 -> name);
	}
	else
		sprintf(arg2str,"NULL");

	if (c.result)
        sprintf(str,"%-15s %-15s %-15s %-15s",opstrs[c.op],arg1str,arg2str,c.result -> name);
	else
        sprintf(str,"%-15s %-15s %-15s %-15s",opstrs[c.op],arg1str,arg2str,"NULL");
	return str;
}

void print_code_chain(code *chain)
{
	char *str = (char *)malloc(MAX_SYMBOL_NAME * 3);

	while (chain)
	{
        codestr(*chain,str);
		printf(" %s\n",str);
		chain = chain -> next;
	}
	free(str);
}
