/*! \file          main.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         главный файл проекта
 *
 * Этот файл содержит фукнцию main, и описвает весь процесс
 * компиляции: от запуска синтаксического анализатора до трансляции и компоновки полученного 
 * целевого кода.
 */

#include "main.h"

int lineno = 1;							//!< содержит номер текущей строки исходного файла
//! тип текущего обрабатываемого символа при синтаксически управляемой трансляции
int curr_type = TYPE_UNKNOWN, 
//! используется для вычисления и хранения количества параметров текущей обрабатываемой функции
param_count,
//! номер текущего обрабатываемого параметра функции
param_index;
//! используется для подсчета количества параметров при обработке вызова print в исходном текстве программы
int expr_count;
symbol *curr_symbol = NULL,				//!< текущий найденный символ
*call_function = NULL,				   //!< указывает на символ функции, вызов которой обнаружен в исходном тексте программы
*curr_function = NULL,				   //!< указывает на символ текущей определяемой функции
*temp = NULL;						   //!< указатель на временный символ
char error_flag = 0,					//!< флаг семантической ошибки в исходном тексте программы
//! флаг "производится определение функции"
proc_definition = 0,
syn_errflag = 0,					   //!< флаг синтаксической ошибки в исходном тексте программы
//! флаг "только проверка синтаксиса"
/*! 
*  Данный флаг устанавливается, если при обработке командной строки компилятора
*  встретился флаг -с, который указывает компилятору сделать только проверку синтаксиса
*  программы.
*/
compile_only = 0;

//! объединение, используемое для приведения типов symbol* и val_t
valptr_t valptr;

//! стек вложенных вызовов
/*! 
*  \sa fci_t
*/
stack_t *fci_stack = NULL;

//! указатель на файловую переменную, используемую функциями yylex и yyparse
extern FILE* yyin;

//! флаг "выодить отладочные сообщения" для bison
extern int yydebug;

//! точка входа в приложение
/*!
*  Функция main вызывается при старте приложения в языке C/C++
*  \param argc   количество переданных аргументов командной строки
*  \param argv[] массив аргументов командной строки
*  \return в случае ошибки возвращается 1, иначе 0.
*/
int main(int argc, char *argv[])
{
	char cmd[1000] = "";				   // строка, в которой будут формироваться командные строки вызова транслятора и компоновщика
	char fname[100] = "",				   // имя входного файла с исходным текстом программы
		ofname[100] = "";					  // имя выходного запускаемого файла, являющегося результатом компиляции
	char libdir[500] = "",				// строка с путем к библиотекам для компоновки
		bindir[500] = "";					  // строка с путем к инструментам (транслятор, компоновщик)
	FILE *tmp = NULL;					// файловая переменная выходного файла
	int rv;								   // переменная для хранения возвращаемого значения функций

	srand((int)time(NULL));
	printf("COMPILER for MyMathLanguage ver.0.1a\nWritten by Dmitry Chuchva\n");
	if (argc > 1)
	{
		int i;
		for (i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				switch (argv[i][1])
				{
				case 'd':
					yydebug = 1;
					break;
				case 'o':
					if (strlen(ofname) > 0)
						printf("another output filename ignored...\n");
					else
						if (argv[i][2] == 0)
						{
							if (argc > i + 1)
							{
								strcpy(ofname,argv[i + 1]);
								i++;
							}
							else
							{
								printf("not enough parameters\n");
								exit(EXIT_FAILURE);
							}
						}
						else
							strcpy(ofname,&argv[i][2]);
					break;
				case 'L':
					if (strlen(libdir) > 0)
						printf("all libs must be in one directory, sorry...\n");
					else
						if (argv[i][2] == 0)
						{
							if (argc > i + 1)
							{
								strcpy(libdir,argv[i + 1]);
								i++;
							}
							else
							{
								printf("not enough parameters\n");
								exit(EXIT_FAILURE);
							}
						}
						else
							strcpy(libdir,&argv[i][2]);
					break;					
				case 'B':
					if (strlen(bindir) > 0)
						printf("all tools must be in one directory, sorry...\n");
					else
						if (argv[i][2] == 0)
						{
							if (argc > i + 1)
							{
								strcpy(bindir,argv[i + 1]);
								i++;
							}
							else
							{
								printf("not enough parameters\n");
								exit(EXIT_FAILURE);
							}
						}
						else
							strcpy(bindir,&argv[i][2]);
					break;
				case 'c':
					compile_only = 1;
					break;
				default:
					printf("parameter %s unknown, ingnoring...\n",argv[i]);
				}
			}
			else
			{
				if (strlen(fname) == 0)
					strcpy(fname,argv[i]);
				else
					printf("only one input file supported. ignoring %s\n",argv[i]);
			}
		}
	}
	else
	{
		usage();
		return 0;
	}

	if (strlen(fname) > 0)
	{
		if (yydebug) printf("input is `%s'...\n",fname);
		yyin = fopen(fname,"rt");
		if (!yyin)
		{
			printf("unable to open file %s. using stdin...\n",fname);
			yyin = stdin;
			sprintf(fname,"stdin");
		}
	}
	else
	{
		printf("input file not specified, using stdin...\n");
		sprintf(fname,"stdin");
		yyin = stdin;
	}

	if (strlen(ofname) == 0)
	{
		if (fname)
		{
			char *c;

			strcpy(ofname,fname);
			c = strstr(ofname,".");
			if (c)
				*c = 0;
			strcat(ofname,".exe");
		}
		else
			strcpy(ofname,"a.exe");
	}
	else
	{
		if (!strstr(ofname,"."))
			strcat(ofname,".exe");
	}
	if (yydebug) printf("output is `%s'...\n",ofname);

	if (strlen(bindir) == 0)
		strcat(bindir,BINDIR);
	else
	{
		while (bindir[strlen(bindir) - 1] == '\\')
			bindir[strlen(bindir) - 1] = '\0';
	}
	if (yydebug) printf("bindir is `%s'...\n",bindir);

	if (strlen(libdir) == 0)
		strcat(libdir,LIBDIR);
	else
	{
		while (libdir[strlen(libdir) - 1] == '\\')
			libdir[strlen(libdir) - 1] = '\0';
	}
	if (yydebug) printf("libdir is `%s'...\n",libdir);

	printf("compiling `%s':\n",fname);

	install(INT_STR,TYPE_INT);
	install(FLOAT_STR,TYPE_FLOAT);

	while (!feof(yyin))
		if (!yyparse())
			break;

	if (yyin != stdin)
		fclose(yyin);

	if (yydebug)
	{        
		print_symtable(*(get_global_context() -> table));
		printf("\n");
		print_ctable(code_table);
	}

	if (!error_flag && !syn_errflag)
	{
		if  (compile_only)
			printf("\ncompile ok\n");
		else
		{
			printf("building `%s':\n",ofname);
			tmp = fopen(ASMTMPFILE,"wt");
			rv = generate_asm_code(tmp,*(get_global_context() -> table),code_table);
			fclose(tmp);

			if (rv < 0)
				printf("build: fatal error: couldnot generate code\n");
			else
			{
				printf("assembling...\n");
				_flushall();
				sprintf(cmd,"%s\\nasmw -f win32 %s -o %s",bindir,ASMTMPFILE,OBJTMPFILE);
				rv = system(cmd);
				if (rv != 0)
					printf("fatal error: assembler returns error code %d\n",rv);
				else
				{
					printf("linking...\n");
					sprintf(cmd,"%s\\alink -oPE -o %s -subsys con -entry mainCRTStartup -L %s %s %s",bindir,ofname,libdir,OBJTMPFILE,LIBS);
					_flushall();
					rv = system(cmd);
					if (rv != 0)
						printf("fatal error: linker returns an error code %d\n",rv);
					else
						printf("\nbuild done\n");
				}
				if (remove(OBJTMPFILE))
					printf("strange error: couldnot delete temp objfile\n");
			}
			if (!yydebug)
				if (remove(ASMTMPFILE))
					printf("strange error: couldnot delete temp asmfile\n");
		}
	}
	else
		printf("compile fails\n");

	clear_code_table(code_table);
	destroy_context_stack();
	destroy_symtable(get_global_context() -> table);
	return 0;
}

void static_error(char *tstr, char *format, ... )
{
	int i;
	va_list list;
	char *output = (char *)malloc(500);
	int output_i = 0;

	char *s = NULL; int d = 0; float f = 0.0;

	memset(output,0,500);
	i = 0;
	va_start(list,format);
	while (i < strlen(format))
	{
		if (format[i] == '%')
		{
			switch (format[i + 1])
			{
			case 's':
				s = va_arg(list,char*);
				strcpy(&output[output_i],s);
				output_i += strlen(s);
				break;
			case 'd':
				d = va_arg(list,int);
				output_i += sprintf(&output[output_i],"%d",d);
				break;
			case 'f':
				f = va_arg(list,float);
				output_i += sprintf(&output[output_i],"%.3f",f);
				break;
			default:
				{
					output[output_i++] = format[i];
					output[output_i++] = format[i + 1];
				}
			}
			i++;
		}
		else
			output[output_i++] = format[i];			
		i++;
	}
	printf("%d: %s: %s\n",lineno,tstr,output);
	free(output);
	if (strcmp(tstr,ERROR) == 0)
		error_flag = 1;
}

void usage()
{
	printf("\
		   usage: compiler [options] [filename]\n\
		   \n\
		   where 'options' are:\n\
		   -d\tswitch debug mode on (output in stderr)\n\
		   -c\tcompile only, do not assemble or link\n\
		   -o\tspecify output executable name\n\
		   -B\tspecify directory with tools (assembler, linker)\n\
		   -L\tspecify directory with linker libs\n\n\
		   and 'filename' is input program code\n\
		   if nothing specified - this help\n");
}