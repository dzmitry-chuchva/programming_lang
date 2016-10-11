/*! \file          unistack.c
 *  \author        Чучва Дмитрий
 *  \date          08.12.2006
 *  \brief         исходный текст функций для работы с универсальным стеком
 *
 * Содержит реализацию универсального стека.
 */

#include "unistack.h"
#include "stdlib.h"
#include "string.h"

stack_t *create_stack(void *data, int el_size)
{
	stack_t *ptr = (stack_t *)malloc(sizeof(stack_t));
	ptr -> data = malloc(el_size);
	ptr -> size = el_size;
	memcpy(ptr -> data,data,el_size);
	ptr -> lower = NULL;
	return ptr;
}

stack_t *destroy_stack(stack_t *st)
{
	while (st)
	{
		if (st -> data)
		{
			free(st -> data);
			st -> data = NULL;
		}
		st = st -> lower;
	}
	return st;
}

stack_t *push(stack_t *st,void *data, int el_size)
{
	if (!st)
		st = create_stack(data,el_size);
	else
	{
		stack_t *ptr = (stack_t *)malloc(sizeof(stack_t));
		ptr -> data = malloc(el_size);
		ptr -> size = el_size;
		memcpy(ptr -> data,data,el_size);
		ptr -> lower = st;
		st = ptr;
	}
	return st;
}

void *top(stack_t *st)
{
	if (!st)
		return NULL;
    return st -> data;
}

stack_t *pop(stack_t *st)
{
	stack_t *ptr = st; int el_size;

	if (!st)
		return 0;
	el_size = st -> size;
    if (st -> data)
	{
		free(st -> data);
		st -> data = NULL;
	}
	st = st -> lower;
	free(ptr);
	return st;
}