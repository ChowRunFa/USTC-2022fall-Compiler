#ifndef SET_H
#define SET_H

typedef struct snode		//这链表是干嘛的？？？
{
	int elem;
	struct snode* next;
} snode, *symset;

//根据代码的形式，似乎每个链表有一个不包含实际数据的头结点，头结点的next连的才是真正的第一个结点

symset phi, declbegsys, statbegsys, facbegsys, relset;		//这五个链表，究竟是干什么的呢？？？？？

symset createset(int data, .../* SYM_NULL */);
void destroyset(symset s);
symset uniteset(symset s1, symset s2);
int inset(int elem, symset s);

#endif
// EOF set.h

