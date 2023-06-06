

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "set.h"

//将链表s1和s2合并成一个，仍保持从小到大的排序    如果有相同的元素，则都要插入新链表里
symset uniteset(symset s1, symset s2)
{
	symset s;
	snode* p;
	
	s1 = s1->next;
	s2 = s2->next;
	
	s = p = (snode*) malloc(sizeof(snode));
	while (s1 && s2)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		if (s1->elem < s2->elem)
		{
			p->elem = s1->elem;
			s1 = s1->next;
		}
		else
		{
			p->elem = s2->elem;
			s2 = s2->next;
		}
	}

	while (s1)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;
		
	}

	while (s2)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}

	p->next = NULL;

	return s;
} // uniteset

//创建一个新结点，其元素为elem，插入链表s中
void setinsert(symset s, int elem)
{
	snode* p = s;
	snode* q;

	while (p->next && p->next->elem < elem)		//插入的位置也有讲究    看来链表元素是从小到大排序的
	{
		p = p->next;
	}
	
	q = (snode*) malloc(sizeof(snode));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
} // setinsert

//创建一个链表，链表的元素为形参
symset createset(int elem, .../* SYM_NULL */)
{
	va_list list;		//知道是这么回事就行了…………
	symset s;

	s = (snode*) malloc(sizeof(snode));
	s->next = NULL;

	va_start(list, elem);
	while (elem)
	{
		setinsert(s, elem);
		elem = va_arg(list, int);
	}
	va_end(list);
	return s;
} // createset

//完全free掉链表s
void destroyset(symset s)
{
	snode* p;

	while (s)
	{
		p = s;
		p->elem = -1000000;		//不太理解
		s = s->next;
		free(p);
	}
} // destroyset

//判断元素elem是否在链表s中，是返回1，不是返回0
int inset(int elem, symset s)
{
	s = s->next;
	while (s && s->elem < elem)
		s = s->next;

	if (s && s->elem == elem)
		return 1;
	else
		return 0;
} // inset

// EOF set.c

