/*! \file          asm_code.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         исходный текст функций генератора целевого кода
 *
 * Содержит реализацию функций генератора целевого кода.
 */

#include "asm_code.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int generate_asm_code(FILE *where,symbol *data,ctable_entry *cod)
{
	symbol *ptr; ctable_entry *cptr;
	char *bigstr = NULL;
	char *line = NULL;
	char generate_rv = 0;

	// first, define global symbols and declare externs
	fprintf(where,"GLOBAL main\n");
	fprintf(where,"EXTERN %s,%s,%s,%s\n",MY_PRINT,MY_READ,MY_PRINT_STR,MY_PRINTLN_STR);
	fprintf(where,"EXTERN %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
		MY_FADD,MY_FSUB,MY_FMUL,MY_FDIV,MY_FPOW,MY_INT2FLOAT,MY_FLOAT2INT,MY_FGT,MY_FLW,MY_FGE,MY_FLE,MY_FEQ,MY_FNEQ);

	// dump symbol table to data section and check for not defined procs
	fprintf(where,"[section .data]\n");
	ptr = data;
	line = (char *)malloc(MAX_SYMBOL_NAME + 50);
	while (ptr)
	{
		sprintf(line,"");
		if (IS_VAR(ptr) || IS_TEMP(ptr) || IS_STRING(ptr))
		{
			strcat(line,ptr -> name);
			if (IS_DEFINED(ptr))
			{
				union {
					symbol *ptr;
					val_t val;
				} valptr;

				valptr.ptr = ptr -> table;

				if (TYPE_OF(ptr) == TYPE_INT)
					sprintf(line,"%s dd %d",line,valptr.val.ival);
				else
					if (TYPE_OF(ptr) == TYPE_FLOAT)
						sprintf(line,"%s dd %.10f",line,valptr.val.fval);
					else
						return -1;
			}
			else
				if (IS_STRING(ptr))
					sprintf(line,"%s db '%s',0",line,(char *)ptr -> table);
				else
                    strcat(line," dd 0");
		}
		if (IS_PROC(ptr))
		{
			if (!is_defined(cod,ptr -> name))
			{
				generate_rv = -1;
				if (strcmp(ptr -> name,ENTRY_SYMBOL) != 0)
					printf("build: error: `%s' function not defined\n",ptr -> name);
				else
					printf("build: error: `entry' function (entry point of program) not defined\n");
			}
		}
		if (strlen(line))
            fprintf(where,"%s\n",line);
		ptr = ptr -> next;
	}
	free(line); line = NULL;

	fprintf(where,"\n");
	// done dumping symbol table
	// now dump code
	fprintf(where,"[section .text]\n");
    cptr = cod;
	bigstr = (char *)malloc(BIGSTR_LENGTH);
	while (cptr)
	{
		if (strcmp(cptr -> func -> name,ENTRY_SYMBOL) == 0)
		{
			fprintf(where,"main:\n");
			fprintf(where,"finit\n");
		}
		else
			fprintf(where,"%s:\n",cptr -> func -> name);
		sprintf(bigstr,"");
		bigstr = code2asm(data,cptr,bigstr);
		fprintf(where,"%s\n",bigstr);
		cptr = cptr -> next;
	}
	free(bigstr);
	// code dump done
	return generate_rv;
}

char *code2asm(symbol *global_table,ctable_entry *cod,char *str)
{
	code *ptr = cod -> cod;
	symbol *ftable = cod -> func -> table;
	int enter_val = 0, ret_val = 0;
	char *opstr = NULL;

	// count sizes
	enter_val = 4 * count_symbols(ftable,TYPE_VAR | TYPE_TEMP);
	ret_val = 4 * count_symbols(ftable,TYPE_ARG);
	/*
	while (ftable)
	{
		if (ftable -> type & (TYPE_VAR | TYPE_TEMP))
			if (TYPE_OF(ftable) == TYPE_INT)
				enter_val += 4;
			else
				enter_val += 8;
		else
			if (ftable -> type & TYPE_ARG)
				if (TYPE_OF(ftable) == TYPE_INT)
					ret_val += 4;
				else
					ret_val += 8;

		ftable = ftable -> next;
	} */
    
	// alloc mem in stack for local symbols and temps
    sprintf(str,"%senter %d,0\n",str,enter_val);
	/*sprintf(str,"%spush ebp\n",str);
	sprintf(str,"%smov ebp,esp\n",str);
	sprintf(str,"%ssub esp,%d\n",str,enter_val);*/
	// initialize defined local variables
	ftable = cod -> func -> table;
	while (ftable)
	{
		union {
			symbol *ptr;
			val_t val;
		} valptr;

		valptr.ptr = ftable -> table;
		if ((ftable -> type & (TYPE_VAR | TYPE_DEFINED)) == (TYPE_VAR | TYPE_DEFINED))
			sprintf(str,"%smov dword [%s],0%xh\t;`%s' initializing\n",str,local_var(global_table,cod -> func -> table,ftable),
					valptr.val,ftable -> name);
		ftable = ftable -> next;
	}
	// generate code
	opstr = (char *)malloc(MAX_SYMBOL_NAME + 100);
	while (ptr)
	{
		sprintf(opstr,"");
		op2asm(global_table,ptr,cod -> func -> table,opstr);
		if (strlen(opstr))
            strcat(str,opstr);
		ptr = ptr -> next;
	}
	free(opstr); opstr = NULL;
	// put return statement in any case (even if it exist)
	strcat(str,"xor eax,eax\n");	// clear return val
	// destroy stack frame
	strcat(str,"leave\n");
	sprintf(str,"%sret %d\n",str,ret_val);
	return str;
}

char *op2asm(symbol *global_table,code *c,symbol *table,char *str)
{
	char tmplbl[9] = "";
	union {
		symbol *ptr;
		val_t val;
	} valptr;

	switch (c -> op)
	{
	case OP_PLUS:
		if (TYPE_OF(c) == TYPE_INT)
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
                sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
			
			if (c -> arg2)	// not unary plus
				if (c -> arg2 -> type & TYPE_CONST)
				{
					valptr.ptr = c -> arg2 -> table;
					sprintf(str,"%sadd eax,%d\n",str,valptr.val.ival);
				}
				else
					sprintf(str,"%sadd eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);

			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		else
		{
			if (c -> arg2)
			{
				floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
				sprintf(str,"%scall %s\n",str,MY_FADD);
			}
			else	// unary plus by floats - just assign with type converting
			{
				if (IS_CONST(c -> arg1))
				{
					valptr.ptr = c -> arg1 -> table;
					sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
				}
				else
					sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
				if (TYPE_OF(c -> result) == TYPE_FLOAT)
					sprintf(str,"%scall %s\n",str,MY_INT2FLOAT);
				else
					sprintf(str,"%scall %s\n",str,MY_FLOAT2INT);
			}

			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_MINUS:
		if (TYPE_OF(c) == TYPE_INT)
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
                sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2)
				if (c -> arg2 -> type & TYPE_CONST)
				{
					valptr.ptr = c -> arg2 -> table;
					sprintf(str,"%ssub eax,%d\n",str,valptr.val.ival);
				}
				else
					sprintf(str,"%ssub eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			else	// unary minus by ints - use neg
                sprintf(str,"%sneg eax\n",str);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		else
		{
			if (c -> arg2)
			{
				floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
				sprintf(str,"%scall %s\n",str,MY_FSUB);
			}
			else	// unary minus by floats - use fchs and assign with type converting
			{
				if (IS_CONST(c -> arg1))
				{
					valptr.ptr = c -> arg1 -> table;
					sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
				}
				else
					sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
				if (TYPE_OF(c -> arg1) == TYPE_INT)
				{
					sprintf(str,"%sfild dword [esp]\n",str);
					sprintf(str,"%sfchs\n",str);
					sprintf(str,"%sfistp dword [esp]\n",str);
				}
				else
				{
					sprintf(str,"%sfld dword [esp]\n",str);
					sprintf(str,"%sfchs\n",str);
					sprintf(str,"%sfstp dword [esp]\n",str);
				}
				sprintf(str,"%spop eax\n",str);
			}

			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_MULT:
		if (TYPE_OF(c) == TYPE_INT)
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%simul dword %d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%simul dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FMUL);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_DIV:
		if (TYPE_OF(c) == TYPE_INT)
		{
			sprintf(str,"%sxor edx,edx\n",str);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);


			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%sidiv dword %d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%sidiv dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FDIV);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_MOD:
		sprintf(str,"%sxor edx,edx\n",str);

		if (c -> arg1 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg1 -> table;
			sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);


		if (c -> arg2 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg2 -> table;
			sprintf(str,"%sidiv dword %d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%sidiv dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
		sprintf(str,"%smov [%s],edx\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_ASSIGN:
		if ((TYPE_OF(c) == TYPE_INT && TYPE_OF(c -> arg1) == TYPE_INT) ||
			(TYPE_OF(c) == TYPE_FLOAT && TYPE_OF(c -> arg1) == TYPE_FLOAT))
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov dword [%s],0%xh\t;%s\n",str,
					local_var(global_table,table,c -> result),
					valptr.val,
					c -> result -> name);
			}
			else
			{
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
                sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			}
		}
		else
		{
			if (TYPE_OF(c -> result) == TYPE_FLOAT)
			{
				if (IS_CONST(c -> arg1))
				{
					valptr.ptr = c -> arg1 -> table;
					sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
				}
				else
					sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
				sprintf(str,"%scall %s\n",str,MY_INT2FLOAT);
			}
			else
			{
				if (IS_CONST(c -> arg1))
				{
					valptr.ptr = c -> arg1 -> table;
					sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
				}
				else
					sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
				sprintf(str,"%scall %s\n",str,MY_FLOAT2INT);
			}
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_GOTO:
		sprintf(str,"%sjmp near %s\n",str,c -> result -> name);
		break;
	case OP_IF:
        sprintf(str,"%scmp dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
		sprintf(str,"%sje near %s\n",str,c -> result -> name);
		break;
	case OP_GT:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sjg %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FGT);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_EQ:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sje %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FEQ);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_LW:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sjl %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FLW);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_GE:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sjge %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FGE);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_LE:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sjle %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FLE);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_NEQ:
		if (TYPE_OF(c) == TYPE_INT)
		{
			randomstr(tmplbl,8);

			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

			if (c -> arg2 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg2 -> table;
				sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
			}
			else
				sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
			sprintf(str,"%scmp eax,ecx\n",str);
			sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%sjne %s\n",str,tmplbl);
			sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
			sprintf(str,"%s%s:\n",str,tmplbl);
		}
		else
		{
			floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
			sprintf(str,"%scall %s\n",str,MY_FNEQ);
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		}
		break;
	case OP_NOT:
		randomstr(tmplbl,8);

		if (c -> arg1 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg1 -> table;
			sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
			sprintf(str,"%scmp eax,0\n",str);
		}
		else
			sprintf(str,"%scmp dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
		sprintf(str,"%smov dword [%s],1\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		sprintf(str,"%sje %s\n",str,tmplbl);
		sprintf(str,"%smov dword [%s],0\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		sprintf(str,"%s%s:\n",str,tmplbl);
		break;
	case OP_AND:
		if (c -> arg1 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg1 -> table;
			sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

		if (c -> arg2 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg2 -> table;
			sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
		sprintf(str,"%sand eax,ecx\n",str);
		sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_OR:
		if (c -> arg1 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg1 -> table;
			sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

		if (c -> arg2 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg2 -> table;
			sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
		sprintf(str,"%sor eax,ecx\n",str);
		sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_XOR:
		if (c -> arg1 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg1 -> table;
			sprintf(str,"%smov eax,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

		if (c -> arg2 -> type & TYPE_CONST)
		{
			valptr.ptr = c -> arg2 -> table;
			sprintf(str,"%smov ecx,%d\n",str,valptr.val.ival);
		}
		else
			sprintf(str,"%smov ecx,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);
		sprintf(str,"%sxor eax,ecx\n",str);
		sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_PARAM:
		if (c -> arg1)
		{
			if (IS_CONST(c -> arg1))
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
				if (TYPE_OF(c) != TYPE_OF(c -> arg1))
				{
					if (TYPE_OF(c) == TYPE_INT) // formal arg is TYPE_INT but fact arg is TYPE_FLOAT
					{
						sprintf(str,"%scall %s\n",str,MY_FLOAT2INT);
						sprintf(str,"%spush eax\n",str);
					}
					else	// formal arg is TYPE_FLOAT but fact arg is TYPE_INT
					{
						sprintf(str,"%scall %s\n",str,MY_INT2FLOAT);
						sprintf(str,"%spush eax\n",str);
					}
				}
				else
					; // nothing to do
			}
			else
				if (IS_STRING(c -> arg1))
					sprintf(str,"%spush dword %s\n",str,c -> arg1 -> name);
				else
					if (IS_STRING(c))	// this is used by calling read function (pushing addrs not a values)
					{
                        char *ptr = local_addr(global_table,table,c -> arg1);
                        strcat(str,ptr);
					}
					else
					{
						sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
						if (TYPE_OF(c) != TYPE_OF(c -> arg1))
						{
							if (TYPE_OF(c) == TYPE_INT) // formal arg is TYPE_INT but fact arg is TYPE_FLOAT
							{
                                sprintf(str,"%scall %s\n",str,MY_FLOAT2INT);
                                sprintf(str,"%spush eax\n",str);
							}
							else	// formal arg is TYPE_FLOAT but fact arg is TYPE_INT
							{
								sprintf(str,"%scall %s\n",str,MY_INT2FLOAT);
								sprintf(str,"%spush eax\n",str);
							}
						}
						else
                            ; // nothing to do
					}
		}
		else
			sprintf(str,"%spush dword %d\n",str,c -> type);	// save type information
		break;
	case OP_CALL:
		sprintf(str,"%scall %s\n",str,c -> arg1 -> name);
		if (c -> result)
			sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_POW:
		floatop_prepare(str,c,global_table,table,TYPE_OF(c -> result));
		sprintf(str,"%scall %s\n",str,MY_FPOW);
		sprintf(str,"%smov [%s],eax\t;%s\n",str,local_var(global_table,table,c -> result),c -> result -> name);
		break;
	case OP_PLUSPLUS:
		sprintf(str,"%sinc dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
		break;
	case OP_MINUSMINUS:
		sprintf(str,"%sdec dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
		break;
	case OP_RET:
		if (TYPE_OF(c -> arg1) == TYPE_OF(c))
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%smov eax,0%xh\n",str,valptr.val);
			}
			else
				sprintf(str,"%smov eax,[%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
		}
		else
		{
			if (c -> arg1 -> type & TYPE_CONST)
			{
				valptr.ptr = c -> arg1 -> table;
				sprintf(str,"%spush dword 0%xh\n",str,valptr.val);
			}
			else
				sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);
			if (TYPE_OF(c) == TYPE_INT)
                sprintf(str,"%scall %s\n",str,MY_FLOAT2INT);
			else
                sprintf(str,"%scall %s\n",str,MY_INT2FLOAT);
		}
		sprintf(str,"%sleave\n",str);
		sprintf(str,"%sret %d\n",str,count_symbols(table,TYPE_ARG) * 4);
		break;
	case OP_PRINT:
		/* c -> type is number of arguments or -1 if it is a string print */
		if (c -> type > 0)
		{
			/* stack is now:
			count_of_args
			typeN
			argN
			typeN-1
			argN-1
			...
			type1
			arg1
			*/
            sprintf(str,"%spush dword %d\t;count of args\n",str,c -> type);
			sprintf(str,"%scall %s\n",str,MY_PRINT);
			sprintf(str,"%sadd esp,%d\n",str,4 + 8 * c -> type);
		}
		else
            if (c -> type == -1)
				sprintf(str,"%scall %s\n",str,MY_PRINT_STR);
		break;
	case OP_PRINTLN:
		sprintf(str,"%scall %s\n",str,MY_PRINTLN_STR);
		break;
	case OP_READ:
		sprintf(str,"%spush dword %d\t;count of args\n",str,c -> type);
		sprintf(str,"%scall %s\n",str,MY_READ);
		sprintf(str,"%sadd esp,%d\n",str,4 + 8 * c -> type);
		break;
	case OP_LABEL:
		sprintf(str,"%s%s:\n",str,c -> arg1 -> name);
		break;
	default:
		printf("internal error: unknown opcode %d\n",c -> op);
	}
	return str;
}

char *local_var(symbol *global_table,symbol *t, symbol *s)
{
    static char offs[MAX_SYMBOL_NAME];

	if (lookup_table(t,s -> name))
	{
		if (s -> type & TYPE_ARG)
            sprintf(offs,"ebp + %d",
					(count_symbols(t,TYPE_ARG) - 
					 symbol_no(t,s -> name,TYPE_ARG)
					 + 1) << 2);
		else
			if (s -> type & (TYPE_VAR | TYPE_TEMP))
	            sprintf(offs,"ebp - %d",(symbol_no(t,s -> name,TYPE_VAR | TYPE_TEMP) + 1) << 2);
	}
	else
		if (lookup_table(global_table,s -> name))
            sprintf(offs,"%s",s -> name);
/*
	while (t && t != s)
	{
		if (t -> type & (TYPE_VAR | TYPE_TEMP))
			if (TYPE_OF(t) == TYPE_INT)
				bytes += 4;
			else
				bytes += 8;
		t = t -> next;
	} */

  
	return offs;
}

//! исходный набор символов, из которых генерируется случайным образом строка
char rset[] = "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
//! выделенная из исходного набора строка с символами (так как генерируемая строка не должна начинаться с цифры)
char *chars = &rset[11];

char *randomstr(char *s,int len)
{
	int setcount = strlen(rset);
	int i;

	if (len <= 0)
		return s;

	s[0] = chars[rand() % strlen(chars)];
    for (i = 1; i < len; i++)
        s[i] = rset[rand() % setcount];
	s[i] = 0;
	return s;
}

char *local_addr(symbol *global_table,symbol *t, symbol *s)
{
	static char offs[MAX_SYMBOL_NAME];
	int offset;

	if (lookup_table(t,s -> name))
	{
		if (s -> type & TYPE_ARG)
		{
			offset = (count_symbols(t,TYPE_ARG) - 
				symbol_no(t,s -> name,TYPE_ARG)
				+ 1) << 2;

            sprintf(offs,"mov eax,ebp\t;eval addr of %s\nadd eax,%d\t;\npush eax\t;\n",s -> name,offset);
		}
		else
            if (s -> type & (TYPE_VAR | TYPE_TEMP))
			{
				offset = (symbol_no(t,s -> name,TYPE_VAR | TYPE_TEMP) + 1) << 2;

				sprintf(offs,"mov eax,ebp\t;eval addr of %s\nsub eax,%d\t;\npush eax\t;\n",s -> name,offset);
			}
	}
	else
		if (lookup_table(global_table,s -> name))
			sprintf(offs,"push %s\n",s -> name);
	return offs;
}

char *floatop_prepare(char *str,code *c,symbol *global_table,symbol *table, int ret_type)
{
	union {
		symbol *ptr;
		val_t val;
	} valptr;

	sprintf(str,"%spush dword %d\n",str,ret_type); 
	if (IS_CONST(c -> arg1))
	{
		valptr.ptr = c -> arg1 -> table;
		sprintf(str,"%spush dword 0%xh\n",str,valptr.val.fval);
	}
	else
		sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg1),c -> arg1 -> name);

	sprintf(str,"%spush dword %d\n",str,TYPE_OF(c -> arg1));

	if (IS_CONST(c -> arg2))
	{
		valptr.ptr = c -> arg2 -> table;
		sprintf(str,"%spush dword 0%xh\n",str,valptr.val.fval);
	}
	else
		sprintf(str,"%spush dword [%s]\t;%s\n",str,local_var(global_table,table,c -> arg2),c -> arg2 -> name);

	sprintf(str,"%spush dword %d\n",str,TYPE_OF(c -> arg2));
	return str;
}
