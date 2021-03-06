/*! \file          codetable.h
 *  \author        ����� �������
 *  \date          07.12.2006
 *  \brief         ������������ ���� ��������� ������� ����
 *
 *  ���� �������� ���������� �������, �����, ����������, ������������ ���������� ������� ����.
 */

#ifndef __CTABLE_H
#define __CTABLE_H

#include "symbol.h"
#include "code3.h"

//! ������� ������� ����
typedef struct tag_ctable {
	symbol *func;						   //!< ������ �������
	code *cod;							   //!< ��� �������

	struct tag_ctable *next;			   //!< ��������� �������
} ctable_entry;

//! ��������� �� ������� ����
extern ctable_entry *code_table;

//! ��������� � ������� �������� ����� �������
/*!
 *  ������� ������� � ������� ���� ����� ������, ���������� ��� ������� � �� ���.
 *  \param for_func  ������ �������
 *  \param func_code �������������� ��� �������
 *  \return ��������� �� ����������� �������
 */
ctable_entry *add_code_entry(symbol *for_func, code *func_code);
//! ���������� ����� ������� ��������
/*!
 *  ������� ���������� ��������� �� ��������� ������� ������� ����.
 *  \param table ��������� �� ������� ������� ����
 *  \return ��������� �� ��������� ������� ������� ����
 */
ctable_entry *ctable_tail(ctable_entry *table);
//! ������� ��� ������� �� ������� ���� �� �����
/*!
 *  ������� ���������� ����� ���� ������� �� �����. ������������ ��� �������.
 *  \param table ��������� �� ������� ������� ����
 */
void print_ctable(ctable_entry *table);
//! ���������� ������� ����
/*!
 *  ������� ���������� ������������ ���������� ��� ������� ���� ������.
 *  \param table ������� ����
 */
void clear_code_table(ctable_entry *table);
//! ���������, ���������� �� ������� � ������� ����
/*!
 *  ������� ���������� ����� �� ������� ���� ������� � �������� ������. ���� ������� ����������
 *  � ������� ����, ������ ��� ����������. � ������ ������ �� ��� TYPE_DEFINED �� ������ ����� 
 *  ����������, ��� ��� �� ������������ ����� � ������ �����.
 *  \param table ������� ����
 *  \param name  ��� ������� ��� ������
 *  \return 0 - ������� �� ����������, 1 - ������� ����������.
 */
char is_defined(ctable_entry *table,char *name);

#endif