/*! \file          symbol.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         исходный текст функций менеджера таблицы символов
 *
 * Содержит реализацию функций менеджера таблицы символов.
 */

#include "symbol.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

symbol *global_table = NULL;
int next_temp_no = 0, next_label_no = 0;
int next_string_no = 0;

//! глобальный статический контекст
context_stack_member start_context = {
		&global_table,
		NULL
};

context_stack_member *context = &start_context;
symbol *symtable_end = NULL;

symbol *lookup(char *name)
{
	symbol *local_symbol = NULL, *global_symbol = NULL;

	local_symbol = search_context(name);

	if (context -> next)
		global_symbol = lookup_table(*(context -> next -> table),name);

	if (local_symbol)
		return local_symbol;
	else
		return global_symbol;
}

int issymbol(symbol *symb)
{
	symbol *ptr;

	for (ptr = *(context -> table); ptr != NULL; ptr = ptr -> next)
		if (ptr == symb)
			return 1;
	return 0;
}

symbol *install(char *name, int type)
{
	return install_in_table(context -> table,name,type);
}

void destroy_symtable(symbol** from)
{
	symbol *ptr = *from, *tmp = NULL;
	
	while (ptr)
	{
		tmp = ptr -> next;
		delete_symbol(ptr);
		ptr = tmp;
	}
	*from = NULL;
}

void print_symtable(symbol *table)
{
	static int level = 0;
	int i = 0;
	symbol *ptr = table;
	char *typestr = (char *)malloc(100);
	union {
		symbol *ptr;
		val_t val;
	} valptr;

	level++;
	while (ptr)
	{
		int count_of_args = 0;
		if (IS_PROC(ptr))
            count_of_args = count_args(ptr);
		get_typestr(ptr,typestr);
		if (IS_PROC(ptr))
			printf("%3d: %-10s %-55s %d %s %-6s\n",i,ptr -> name,typestr,count_of_args,"args",
				level == 1 ? "global" : "local");
		else
		{
			if ((IS_VAR(ptr) && IS_DEFINED(ptr)) || IS_CONST(ptr))
			{
				valptr.ptr = ptr -> table;
				if (TYPE_OF(ptr) == TYPE_INT)
					printf("%3d: %-10s %-55s %-6d %-6s\n",i,ptr -> name,typestr,valptr.val.ival,
						level == 1 ? "global" : "local");
				else
					printf("%3d: %-10s %-55s %-6.1f %-6s\n",i,ptr -> name,typestr,valptr.val.fval,
						level == 1 ? "global" : "local");
			}
			else
				printf("%3d: %-10s %-55s %-6s %-6s\n",i,ptr -> name,typestr," ",
					level == 1 ? "global" : "local");
		}

		if (IS_PROC(ptr) && ptr -> table)
            print_symtable(ptr -> table);

		ptr = ptr -> next;
		i++;
	}
	free(typestr);
	level--;
}

int count_args(symbol *symb)
{
	symbol *ptr = symb -> table;
	int res = 0;

	while (ptr && IS_ARG(ptr))
	{
		res++;
		ptr = ptr -> next;
	}

	return res;
}

int count_symbols(symbol *symb,int type_mask)
{
	symbol *ptr = symb;
	int res = 0;

	while (ptr)
	{
		if (ptr -> type & type_mask)
			res++;
		ptr = ptr -> next;
	}

	return res;
}

symbol *lookup_arg(symbol *symb, int no)
{
    symbol *ptr = symb -> table;
	int count = count_args(symb), curr_no = 1;

	if (no > count)
		return NULL;
    while (ptr && IS_ARG(ptr))
	{
		if (curr_no == no)
			return ptr;
		curr_no++;
		ptr = ptr -> next;
	}
	return ptr;
}

symbol *lookup_arg_by_name(symbol *symb, char *name)
{
	symbol *ptr = symb -> table;

	while (ptr && IS_ARG(ptr))
	{
		if (strcmp(name,ptr -> name) == 0)
			return ptr;
		ptr = ptr -> next;
	}
	return ptr;
}

symbol *tail(symbol *table)
{
	symbol *ptr = NULL, *tail = NULL;

	while (table)
	{
		ptr = table;
        table = table -> next;
	}

	return ptr;
}

symbol *install_temp(int type)
{
	char temp[100];
	symbol *s;

	sprintf(temp,"%s%.6d",TEMPPREFIX,next_temp_no++);
	s = install(temp,type);
	return s;
}

char *get_typestr(symbol *symb,char *str)
{
	sprintf(str,"");
	if (TYPE_OF(symb) == TYPE_INT)
        strcat(str,"TYPE_INT   | ");
	if (TYPE_OF(symb) == TYPE_FLOAT)
		strcat(str,"TYPE_FLOAT | ");
    if (IS_PROC(symb))
		strcat(str,"TYPE_PROC | ");
	if (IS_VAR(symb))
		strcat(str,"TYPE_VAR  | ");
	if (IS_ARG(symb))
		strcat(str,"TYPE_ARG  | ");
	if (IS_CONST(symb))
		strcat(str,"TYPE_CONST  | ");
	if (IS_LABEL(symb))
		strcat(str,"TYPE_LABEL  | ");
	if (IS_TEMP(symb))
		strcat(str,"TYPE_TEMP  | ");
    if (IS_DECLARED(symb))
		strcat(str,"TYPE_DECLARED | ");
	if (IS_DEFINED(symb))
		strcat(str,"TYPE_DEFINED  | ");
	if (IS_STRING(symb))
		strcat(str,"TYPE_STRING | ");
	if (strlen(str) == 0)
		strcat(str,"TYPE_UNKNOWN");
	else
		memset(str + strlen(str) - 3,0,3);	// clear last ` | `
	return str;
}

void delete_symbol(symbol *symb)
{
	if (symb -> name)
	{
		free(symb -> name);
		symb -> name = NULL;
	}
	if (IS_PROC(symb) && symb -> table)
		destroy_symtable(&(symb -> table));
	else
		if (IS_STRING(symb) && symb -> table)
		{
			free(symb -> table);
			symb -> table = NULL;
		}
		else
        	symb -> table = NULL;
	symb -> next = NULL;
	symb -> prev = NULL;
	symb -> type = 0;
	free(symb);
	symb = NULL;
}

void delete_local(symbol *symb)
{
	destroy_symtable(&(symb -> table));
}

void change_context(symbol **table)
{
	context_stack_member *ptr = (context_stack_member *)malloc(sizeof(context_stack_member));

	ptr -> table = table;
	if (context)
	{
		ptr -> next = context;
		context = ptr;
	}
	else
	{
        context = ptr;
		ptr -> next = NULL;
	}
}

void restore_context()
{
	context_stack_member *ptr = context;

	context = context -> next;
	free(ptr);
}

context_stack_member *get_global_context()
{
	return &start_context;
}

void destroy_context_stack()
{
	context_stack_member *ptr;

	while (context && context != &start_context)
	{
		ptr = context -> next;
		free(context);
		context = ptr;
	}
}

symbol *search_context(char *name)
{
	return lookup_table(*(context -> table),name);
}

symbol *install_const(int type,val_t val)
{
	symbol *ptr = install(CONSTANTSNAME,type);
	union {
		symbol *ptr;
		val_t val;
	} valptr;

	valptr.val = val;
	ptr -> table = valptr.ptr;
	return ptr;
}

symbol *install_label()
{
	char *name = (char *)malloc(20);
	symbol *ptr = NULL;
	sprintf(name,"%s%.4d",LABELPREFIX,next_label_no++);
	ptr = install(name,TYPE_LABEL);
	free(name);
	return ptr;
}

int symbol_no(symbol *symb,char *name,int type_mask)
{
	int res = 0;

	while (symb)
	{
		if (symb -> type & type_mask)
            if (strcmp(symb -> name,name) == 0)
				return res;
			else
				res++;
		symb = symb -> next;
	}

	return res;
}

symbol *lookup_table(symbol *table,char *name)
{
	return lookup_table_type(table,name,0xFFFFFFFF);
}

symbol *lookup_table_type(symbol *table,char *name,int type_mask)
{
	symbol *ptr = NULL;

	for (ptr = table; ptr != NULL; ptr = ptr -> next)
		if (strcmp(name,ptr -> name) == 0 && (ptr -> type & type_mask))
			return ptr;
	return NULL;
}

symbol *install_string(char *str)
{
	char name[30] = "";
	symbol *res = NULL;
	
    sprintf(name,"%s%.3d",STRINGRESOURCE,next_string_no++);
	res = install_in_table(get_global_context() -> table,name,TYPE_STRING);
	res -> table = (symbol *)malloc(strlen(str) + 1);
	strcpy((char *)res -> table,str + 1);	// skip first "
    ((char *)res -> table)[strlen((char *)res -> table) - 1] = 0;	// and last "
	return res;
}

symbol *get_current_context_table()
{
    return *(context -> table);
}

symbol *install_in_table(symbol **table,char *name,int type)
{
	symbol *ptr;

	ptr = (symbol *)malloc(sizeof(symbol));
	ptr -> name = (char *)malloc(strlen(name) + 1);
	strcpy(ptr -> name,name);//,MAX_SYMBOL_NAME);
	ptr -> type = type;
	ptr -> next = NULL;
	ptr -> prev = NULL;
	ptr -> table = NULL;

	symtable_end = tail(*table);
	if (!symtable_end)
	{
		symtable_end = ptr;
		*table = ptr;
	}
	else
	{
		ptr -> prev = symtable_end;
		symtable_end -> next = ptr;
		symtable_end = ptr;
	}
	return ptr;    
}