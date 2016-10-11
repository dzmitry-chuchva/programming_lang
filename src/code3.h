/*! \file          code3.h
 *  \author        ����� �������
 *  \date          08.12.2006
 *  \brief         ���������� �������������� ���� :)
 *
 * ���� �������� ���������� �������, ����������, �����, ������������ � �������� �������������
 * ����������� ���������� � ��������� �������������� ������������� ����.
 */

#ifndef __CODE3_H
#define __CODE3_H

#include "symbol.h"

//@{
//! ���� ��������
#define OP_PLUS		0
#define OP_MINUS	1
#define OP_MULT		2
#define OP_DIV		3
#define OP_MOD		4

#define OP_ASSIGN	5
#define OP_GOTO		6
#define OP_IF		7

#define OP_GT		8
#define OP_EQ		9
#define OP_LW		10
#define OP_GE		11
#define OP_LE		12
#define OP_NEQ		13
#define OP_NOT		14
#define OP_AND		15
#define OP_OR		16
#define OP_XOR		17

#define OP_PARAM	18
#define OP_CALL		19

#define OP_POW		20
#define OP_PLUSPLUS	21
#define OP_MINUSMINUS	22

#define OP_RET		23
#define OP_PRINT	24
#define OP_READ		25
#define OP_LABEL	26
#define OP_PRINTLN	27
//@}


////////////////////////////////////////////////////

//! ��������� ������������� ��������� � ������ ����������
typedef struct tag_code {
	int op;								   //!< ��� ��������
	symbol *arg1,						   //!< ������ ������� ���������
		*arg2;				   //!< ������ ������� ���������
	symbol *result;						   //!< ������ ����������
	int type;							   //!< ��� �������� (� �� ������ :)

	struct tag_code *next,				   //!< ��������� ��������
		*prev;		   //!< ���������� ��������
} code;

//! ���������� ��������
/*!
 *  ������� ���������� �������� � ���� ��������� code �� ���������� ����������.
 *  \param op     ��� ��������
 *  \param arg1   ������ ������� ���������
 *  \param arg2   ������ ������� ���������
 *  \param result ������ ����������
 *  \param type   ��� ���������
 *  \return ��������� �� ��������� �������� code
 */
code *gen_code(int op, symbol *arg1, symbol *arg2, symbol *result, int type);
//! ���������� ������ ����������
/*!
 *  ������� ��������� � ������ ���������� code1 ������ ���������� code2.
 *  \param code1 ������ ����������
 *  \param code2 ������ ����������
 *  \return ������������ ������ ����������
 */
code *merge_code(code *code1,code *code2);
//! �������� ��������� �������� � ������
/*!
 *  ������ ���������� ��������� �� ��������� �������� � ������ ����������.
 *  \param c ������ ����������
 *  \return 
 */
code *code_tail(code *c);
//! �������� ��������� ������������� ���������
/*!
 *  ������� ��������� ������ ��� ������������� ��������� �� ������. ������������ ��� �������.
 *  \param c   ��������
 *  \param str ������, ���� ����������� �������������
 *  \return ��������� �� �������������� ������
 */
char *codestr(code c,char *str);
//! ������� ������ ���������� �� �����
/*!
 *  ������� ������� ������ ��������� �� ����� � ���� ������ �����. ������������ ��� �������.
 *  \param chain ������ ����������
 */
void print_code_chain(code *chain);
//! ���������� ������ ����������
/*!
 *  ������� ���������� ������������ ������, ������� ��� ������ ����������.
 *  \param c ������ ����������
 */
void destroy_code(code *c);

#endif