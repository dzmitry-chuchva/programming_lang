/*! \file          codetable.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         исходный текст функций менеджера таблицы символов
 *
 * Содержит реализацию функций менеджера таблицы символов.
 */

#include "codetable.h"
#include "stdlib.h"
#include "stdio.h"

ctable_entry *code_table = NULL;

ctable_entry *add_code_entry(symbol *for_func, code *func_code)
{
	ctable_entry *ptr = (ctable_entry *)malloc(sizeof(ctable_entry)), 
		*t = ctable_tail(code_table);

	ptr -> cod = func_code;
	ptr -> func = for_func;
	ptr -> next = NULL;

    if (!t)
		code_table = ptr;
	else
		t -> next = ptr;
	return ptr;
}

ctable_entry *ctable_tail(ctable_entry *table)
{
	ctable_entry *ptr = NULL;

	while (table)
	{
		ptr = table;
		table = table -> next;
	}

	return ptr;
}

void clear_code_table(ctable_entry *table)
{
    ctable_entry *ptr = NULL;

	while (table)
	{
		ptr = table -> next;
		free(table);
		table = ptr;
	}
}

void print_ctable(ctable_entry *table)
{
    code *codeptr = NULL;
	char *cstr = (char *)malloc(80);
	int num;

    while (table)
	{
		printf("Function `%s':\n",table -> func -> name);
		codeptr = table -> cod;
		num = 0;
		while (codeptr)
		{
			num++;
			codestr(*codeptr,cstr);
			printf("%4d: %s\n",num,cstr);
            codeptr = codeptr -> next;
		}
		table = table -> next;
	}
}

char is_defined(ctable_entry *table,char *name)
{
    ctable_entry *ptr = table;

	while (ptr)
	{
		if (strcmp(ptr -> func -> name,name) == 0)
			return 1;
		ptr = ptr -> next;
	}

	return 0;
}