/*! \file          main.h
 *  \author        ����� �������
 *  \date          08.12.2006
 *  \brief         �������� ������������ ����
 *
 * ������������ ���� �������� ����� �������.
 */

#ifndef __MAIN_H
#define __MAIN_H

//! ��������� bison �� ��, ����� ������������� ����� ��������� �������� ������ ��� ������� yyerror
#define YYERROR_VERBOSE 1
//! ��������� bison �� ��, ��� ���������� �������� ��������� ������� � ������������ ����������
#define YYDEBUG 1

#include "symbol.h"
#include "code3.h"
#include "codetable.h"
#include "asm_code.h"
#include "unistack.h"

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "time.h"

//! ��������� �� ��, ��� ������������ ANSI C (����� ��������� ����������� ������������ ������!).
/*! 
*  ���� ����� �� �������, ���������� �� �������������� ������������ Microsoft C++, �.�. bison
*  ������� ����������� ������� yyparse � ����� C, ����� ���� ���������� ������������ �� �
*  ��������� �������, � ����� ������� �� ���. ��� ��� �� ��������� bison ���������� ����������
*  yyparse ��� ���������� (YYPARSE_PARAM �� �����������), �� ����������� ������� ����� ���������
*  ���:
*        int
*        yyparse()
*            ;
*  ���������� C++ �� ������� Visual Studio �������� ��� ������ ��� �������� �������,
*  � ������ �� ������ � ��������� ������, ��� ����� ��������� ����������� �������� ������:
*        '{' used at file scope (missing function header?)
*  ������ �������� ���������� ���� �� ���������� �������� ���������� � ��������� �������
*  ��������������� ��������� ��������� ������ ����������� /Za, ������� ������������� ����������
*  ����� __STDC__.
*/
#define __STDC__

//! ������, ������������ �������� ��������� � ����������� �� ��� ����
#define VALUE(e)	(e.type & TYPE_INT ? e.val.ival : (e.type & TYPE_FLOAT ? e.val.fval : 0))

//! ��� ���������� �����, ���� ������������ ������� ������������ ���
#define ASMTMPFILE	"tmpfile.asm"
//! ��� ���������� ���������� �����, ������������ ��� ���������� � ����������
#define OBJTMPFILE	"tmpfile.obj"
//! ������ ���������, � �������� ����������� ���������� � ���������� ���������� ���������
/*! 
*  ������ ������ ������������ ��� ������� ������������ � ���������� ���
*  � �������� ���������� ����� ��������� ������ 
*/
#define LIBS "win32.LIB libc.lib crtdll.lib iolayer.obj floating.obj"

//! ���� � ������������ (���������, �����������) �� ���������
#define BINDIR	"bin"
//! ���� � ����������� �� ���������
#define LIBDIR	"lib"

extern int lineno;
extern int curr_type, 
param_count,
param_index;
extern int expr_count;
extern symbol *curr_symbol,
*call_function,
*curr_function,
*temp;
extern char error_flag,
proc_definition,
syn_errflag,
compile_only;

//! �����������, ������������ ��� ���������� ����� symbol* � val_t
typedef union {
	symbol *ptr;						   //!< ��������� �� symbol
	val_t val;							   //!< ����������� "��������"
} valptr_t;

extern valptr_t valptr;

//! ��� ����� ������� ������� � �������� ���������� ������ �������
/*! 
*  ������������ ��� ��������� ��������� ������� ���� f1(f2(f3()),f4(f5(),f6())) ����������
*  ��������� ������������� ����������� ���������� � ����, ��� ���������� param_index, param_count
*  � call_function ������� ���������. �.�. ��� ��������� ���������� ������ �������� ���������� � 
*  ���������� ��������� � ���������� ������, ������� ������������� �� �����.
*/
typedef struct tag_fci_t {
	symbol *call_function;				   //!< ������ �������, ��������� �������� ���� �������� ������ �������
	int param_index;					   //!< ����������� �������� param_index
	int param_count;					   //!< ����������� �������� param_count
} fci_t;

extern stack_t *fci_stack;

//! ��������� ����� ��������������� ��������� �� �������
/*!
*  ��������� ����� ��������� � ������������� �������, ������������ � ������ ��������� � ��������
*  ������������� ����������� ����������.
*  \param tstr   ������, ���������� ��� ������
*  \param format ������-����� � ����� printf, �� � ������������ ������� �������������� ����
*/
void static_error(char *tstr, char *format, ... );

//! ������� ���������� � ���������� ��������� ������
/*!
*  ������� ������� ���������� � ������� ������� �����������.
*/
void usage();

#endif