/*! \file          code3.h
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         менеджмент промежуточного кода :)
 *
 * Файл содержит объявления функций, переменных, типов, используемых в процессе синтаксически
 * управляемой трансляции и генерации промежуточного трехадресного кода.
 */

#ifndef __CODE3_H
#define __CODE3_H

#include "symbol.h"

//@{
//! коды операций
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

//! структура трехадресного оператора в списке операторов
typedef struct tag_code {
	int op;								   //!< код операции
	symbol *arg1,						   //!< символ первого аргумента
		*arg2;				   //!< символ второго аргумента
	symbol *result;						   //!< символ результата
	int type;							   //!< тип операции (и не только :)

	struct tag_code *next,				   //!< следующий оператор
		*prev;		   //!< предыдущий оператор
} code;

//! генерирует оператор
/*!
 *  Функция генерирует оператор в виде структуры code по переданным параметрам.
 *  \param op     код операции
 *  \param arg1   символ первого аргумента
 *  \param arg2   символ второго аргумента
 *  \param result символ результата
 *  \param type   тип оператора
 *  \return указатель на созданную стуктуру code
 */
code *gen_code(int op, symbol *arg1, symbol *arg2, symbol *result, int type);
//! объединяет списки операторов
/*!
 *  Функция добавляет к списку операторов code1 список операторов code2.
 *  \param code1 список операторов
 *  \param code2 список операторов
 *  \return объединенный список операторов
 */
code *merge_code(code *code1,code *code2);
//! получить последний оператор в списке
/*!
 *  Функия возвращает указатель на последний оператор в списке операторов.
 *  \param c список операторов
 *  \return 
 */
code *code_tail(code *c);
//! получает строковое представление оператора
/*!
 *  Функция формирует строку для представления оператора на экране. Используется при отладке.
 *  \param c   оператор
 *  \param str строка, куда формировать представление
 *  \return указатель на сформированную строку
 */
char *codestr(code c,char *str);
//! выводит список операторов на экран
/*!
 *  Функция выводит список оператора на экран в виде набора строк. Используется при отладке.
 *  \param chain список операторов
 */
void print_code_chain(code *chain);
//! уничтожает список операторов
/*!
 *  Функция производит освобождение памяти, занятой под список операторов.
 *  \param c список операторов
 */
void destroy_code(code *c);

#endif