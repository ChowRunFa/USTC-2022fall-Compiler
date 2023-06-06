// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
//n为错误类型
void error(int n)			//错误处理函数
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
//读进来下一个原始字符
//同时把PL0高级语言代码打印一遍
void getch(void)
{
	//cc==ll意味着本行的所有字符已经读完。
	//此时读入下一整行字符并打印，以及存到line数组里，之后每次调用getch就从line里拿出一个，直到cc==ll，然后开启下一行
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);		//输出本行原始代码生成的对应汇编指令的序号
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);		//输出本行原始代码
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];		//ch作为全局变量，起到了近似于返回值的作用
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
//识别出下一个词法记号
//调用这个函数的时候，ch已经是本次将要得到的词法记号字符串的第一个字符
//因为有时需要多读一个字符才能确定当前的词法记号，因此要多读一个。而读过的字符又退不回去，只能遗留ch给下一个词法记号
//sym就当是返回值，返回了读进来的词法记号的记号名类型
//如果是标识符SYM_IDENTIFIER，id也是返回值
//如果是数字SYM_NUMBER，num也是返回值
//其他的词法记号不需要属性值，返回一个sym足矣
void getsym(void)
{
	int i, k;		//k用来记录当前词法记号看到第几个字符了
	char a[MAXIDLEN + 1];		//记录本次读进来的词法记号的字符串序列

	while (ch == ' '||ch == '\t')	//空格和tab都当成没有，而换行已经交给了getch()处理
		getch();
	//是为了去掉源程序里各个词法单元之间的空格    var i;这种

	if (isalpha(ch))		//当前字符是字母    这一步只有第一个字母会经历，因为以后的字母都直接在内部处理了
	{ 
		//走到这里，就说明这个词法记号只能是标识符或者关键字
		// symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)		//如果长度超了咋整…………
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		//出了这个循环之后，这时的ch是下一个词法记号的第一个字符
		
		a[k] = 0;	//'\0'
		strcpy(id, a);		//id为当前的词法记号字符串
		word[0] = id;		//算是一种骚操作…………
		i = NRW;
		while (strcmp(id, word[i--]));		//将id和各个关键字比对
		if (++i)		//跟某一个关键字对上了
			sym = wsym[i]; // symbol is a reserved word
		else			//跟word[0]对上了，就是一个都没对上，说明是标识符
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}

	else if (isdigit(ch))		//当前字符是数字    这一步只有第一个数字会经历，因为以后的数字都直接在内部处理了
	{
		//走到这里，就说明这个词法记号只能是数字
		// symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));

		if (k > MAXNUMLEN)		//数字过大就知道报错…………
			error(25);     // The number is too great.
	}

	else if (ch == ':')
	{
		getch();	//把下一个字符读进来
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_COLON;       //for循环的 :
		}
	}

	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();		//读入下一个词法记号的第一个字符
		}
		else
		{
			sym = SYM_GTR;     // >
			//当前的ch就是下一个词法记号的第一个字符，就别读下一个了
		}
	}

	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	//= 作为一个单字符的算符，这里就得到了一些超然的地位………………
	else if (ch == '=')
	{
		sym = SYM_EQU;     // =
		getch();
	}

	else		//现在只差11个“自带算符”没处理     11个自带算符都是单字符，因此比较好处理
	{ // other tokens
		i = NSYM;
		csym[0] = ch;		//还是这种骚操作…………
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
//给出了一条汇编指令的三个参数，只需要把这三个参数填到code里就行了
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
//只是用来错误处理的？？？
//只要当前sym不在链表s1里就报错     之后还会尝试修复一下…………
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index    可以理解为当前procedure的AR大小，即为局部变量数目
//只有定义var变量，进而调用enter()的时候，dx才会++

// enter object(constant, variable or procedre) into table.
//把一个标识符加入符号表
//这个标识符就是当前的sym == SYM_IDENTIFIER对应的id
//如果是常数，那么num也需要
void enter(int kind)			//kind只能是常数，变量，过程，数组四者之一
{
	mask* mk;		//只有kind为变量或过程时才会用到

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;

	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)			//num用在了这里   不过为什么是MAXADDRESS？？？
		{
			error(25); // The number is too great.
			num = 0;
		}
		//要求const定义时必须赋值，因此这里的num就是这个常数的值
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;		//定义这个局部变量的过程的绝对嵌套深度
		mk->address = dx++;		//该局部变量在AR中的相对位置
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		//过程的address域不是在这里算出来的
		break;
	case ID_ARRAY:
		mk = (mask*) &table[tx];
		mk->level = level;		//定义这个数组变量的过程的绝对嵌套深度
		mk->address = dx;		//该数组的起始元素在AR中的相对位置    此时dx不动
		//数组的dimension和n[10]不在这里算出
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
//输入的id是试图在符号表里寻找的标识符名的字符串
//如果找到，返回该标识符在符号表中的下标；没找到，则返回0
//table[0]本来是留给main过程的过程名的，不过实际上main过程没有过程名，所以这里就随便用了…………
//在符号表里从后向前遍历查找，没有任何花哨的
//所以实际上不分什么全局变量或者局部变量，只要定义了就是全局变量…………
//上面的话也不完全对，子过程里定义的常量、变量和函数，在子过程编译结束，回到母过程之后，就都会被删掉
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
//处理 const a=5,b=6,c=7; 中的 a=5
//进入此函数时，sym为标识符a
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();		//得到=
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)		//真细节…………
				error(1); // Found ':=' when expecting '='.

			getsym();		//得到5
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);		//入符号表
				getsym();		//词法记号方面，也要提前把下一个词法记号找出来   应该只能是,或;
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);	// There must be an identifier to follow 'const', 'var', or 'procedure'.

} // constdeclaration

//////////////////////////////////////////////////////////////////////
//处理 var x,y,z; 中的 x
//变量不允许赋初值
//进入此函数时，sym为标识符x
//现在，这个函数也要处理数组变量的定义，就麻烦了很多
// var x,y,z[10][10]; 中的 x 或者 z[10][10]
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);			//先默认当成普通变量来算，要是错了再改回来     反正知道这个标识符在符号表里的下标是tx
		getsym();
		if(sym==SYM_LBRACKET)		//说明定义的是数组变量，要多搞一些东西     普通变量就直接过了
		{
			tx--;dx--;	//补救措施
			enter(ID_ARRAY);		//原位置重填一次    由于读进来的是[，所以id未变，重填一次完全可行
			//接下来要处理[10][10]了
			mask* mk;
			mk = (mask*) &table[tx];	//指向对应本数组名的符号表单元
			mk->dimension=0;		//维数赋初值为0
			int space=1;		//记录这个数组变量占用的总空间
			do
			{
				getsym();	//得到 10
				if(sym==SYM_NUMBER)
				{
					mk->n[mk->dimension]=num;		//记录本维数的大小    n[k]表示第k+1维的大小
					mk->dimension=mk->dimension+1;	//维数++
					space=space*num;	//空间增长的方式
					getsym();	//得到 ]
				}
				else
				{
					error(28);		// A number is expected here.
				}

				if(sym==SYM_RBRACKET)
				{
					getsym();	//得到 [ 或者其他的东西     其他的东西标志着这个数组变量定义完毕

				}
				else
				{
					error(29);		// Missing ']'.
				}
			} while (sym==SYM_LBRACKET);	//每轮处理一个[10]
			
			dx+=space;		//总空间记上
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
//把已经生成好的PL0汇编语言打印一遍
//打印区间：[from,to)
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode


//辅助函数定义完毕，下面是诸多非终结符的处理方案
//////////////////////////////////////////////////////////////////////
//处理非终结符之————因子
//得到的汇编代码段，执行之后，应该只有一个运算结果留在数据栈的栈顶
void factor(symset fsys)
{
	void expression(symset fsys);		//大概是局部声明吧
	int i;
	symset set,set1;
	int j=0;	//对于是数组的情况，记录已经处理的数组维数
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))		//判断四者之一       加了setjmp()之后，现在是五者了
	{
		if (sym == SYM_IDENTIFIER)		//ident    或者 ident := 表达式
		{
			if ((i = position(id)) == 0)		//i为目标标识符的符号表位置
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);		//常数放到栈顶
					getsym();
					break;

				case ID_VARIABLE:			//我们的赋值表达式的魔改，将发生在这里
					mk = (mask*) &table[i];
					getsym();		//需要再看一个词法记号才能做出判断    看看下一个词法记号是不是:=
					if(sym==SYM_BECOMES)		//赋值表达式发挥作用    id := 表达式
					{
						//下面不需要区分:=右面是数组还是什么，一个expression()足以解决所有问题

						getsym();	//得到后面表达式的第一个词法记号
						set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
						//表达式的后继可以有)     此时这个表达式的后继不应该有]
						expression(set);		//处理表达式
						destroyset(set);

						gen(STO, level - mk->level, mk->address);		//生成汇编指令，将表达式的结果存入赋值变量的存储单元
						gen(LOD, level - mk->level, mk->address);		//同时这个计算结果需要出现在栈顶
					}
					else		//没有赋值表达式，就像以往一样运行就行
					{
						gen(LOD, level - mk->level, mk->address);		//变量放到栈顶
					}
					break;

				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					getsym();
					break;

				case ID_ARRAY:			//我们的赋值表达式的魔改，将发生在这里   这里的操作可能极其复杂…………
					mk = (mask*) &table[i];
					getsym();		//得到 [

					//显然在看是否有:=之前，应当先把数组的各维处理了
					if (sym == SYM_LBRACKET)
					{
						getsym();		//得到表达式的第一个词法记号
					}
					else
					{
						error(30); // Missing '['.
					}

					set1 = createset(SYM_RBRACKET, SYM_NULL);
					set = uniteset(set1, fsys);
					//此处的表达式后继可以有 ]
					expression(set);	//处理第一维的表达式
					if (sym == SYM_RBRACKET)
					{
						getsym();		//得到 [      一维数组的话应该就是 :=
					}
					else
					{
						error(29); // Missing ']'.
					}

					j=1;
					while(j < mk->dimension)
					{
						if (sym == SYM_LBRACKET)
						{
							getsym();		//得到表达式的第一个词法记号
						}
						else
						{
							error(30); // Missing '['.
						}
						gen(LIT,0,mk->n[j]);
						gen(OPR,0,OPR_MUL);
						expression(set);	//处理表达式
						gen(OPR,0,OPR_ADD);
						if (sym == SYM_RBRACKET)
						{
							getsym();		//得到 [      数组维数用尽的话应该就是 :=
						}
						else
						{
							error(29); // Missing ']'.
						}
						j++;
					}
					destroyset(set1);
					destroyset(set);
					//此时运行时栈顶的唯一数据就是数组中目标元素的offset

					if(sym==SYM_BECOMES)		//赋值表达式发挥作用    id := 表达式
					{
						getsym();	//得到后面表达式的第一个词法记号
						set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
						//表达式的后继可以有)
						expression(set);		//处理表达式
						destroyset(set);

						gen(STR, level - mk->level, mk->address);		//生成汇编指令，将表达式的结果存入赋值变量的存储单元
						gen(LDR, level - mk->level, mk->address);		//同时这个计算结果需要出现在栈顶
					}
					else		//没有赋值表达式，就像以往一样运行就行
					{
						gen(LDR, level - mk->level, mk->address);		//变量放到栈顶
					}

					break;
				} // switch
			}
			//getsym();
		}

		else if (sym == SYM_NUMBER)		//number
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);		//常数放到栈顶
			getsym();
		}

		else if (sym == SYM_LPAREN)		//( 表达式 )
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_COMMA, SYM_NULL), fsys);
			//表达式的后继可以有)和,了
			expression(set);		//处理表达式
			destroyset(set);

			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}

		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr        - 因子
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);		//栈顶取反指令
		}

		else if(sym==SYM_SETJMP)
		{
			getsym();		//得到 (
			//鉴定并处理 (
			if (sym == SYM_LPAREN)
			{
				getsym();		//得到表达式的第一个词法记号
			}
			else
			{
				error(26); // Missing '('.
			}

			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			//表达式的后继可以有)了
			expression(set);		//处理表达式
			destroyset(set);

			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}

			gen(SJP,0,0);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
//处理非终结符之————项
//得到的汇编代码段，执行之后，应该只有一个运算结果留在数据栈的栈顶
void term(symset fsys)
{
	int mulop;		//*或者/
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);		//处理因子
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);		//处理因子
		if (mulop == SYM_TIMES)		//相当于得到一个后缀表达式
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
//处理非终结符之————表达式
//得到的汇编代码段，执行之后，应该只有一个运算结果留在数据栈的栈顶
void expression(symset fsys)
{
	int addop;		//+或者-
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);		//处理项
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);		//处理项
		if (addop == SYM_PLUS)		//相当于得到一个后缀表达式
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
//处理非终结符之————条件
//得到的汇编代码段，执行之后，应该只有一个运算结果留在数据栈的栈顶
//condition里的表达式就不是print()里的表达式，所以表达式的后继扩展是有限制的
void condition(symset fsys)
{
	int relop;		//如果是条件表达式的话，用这个记录条件符号    = <> < <= > >= 之一
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);		//处理表达式
		gen(OPR, 0, 6);		//6为OPR_ODD
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);		//处理表达式
		destroyset(set);

		if (! inset(sym, relset))		//判断读进来的必须是 = <> < <= > >= 之一
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);		//处理表达式

			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);		//目前尚不知道OPR和JPC的运行规则…………
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
//处理非终结符之————语句    不是语句序列，但是这里通过多个statement()的循环，可以直接构成语句序列，因此语句序列就没有一个单独的非终结符
//得到的汇编代码段，执行之后，数据栈应该恰好被清空
//修正了语句结尾分号的判定问题
void statement(symset fsys)
{
	int i, cx1, cx2;		//cx1和cx2用来记录JPC和JMP指令序号，以便将来回填
	symset set1, set;		//用来后继扩展的

	if (sym == SYM_IDENTIFIER)		//ident := 表达式 ;     现在ident应当拓展成变量和数组
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))		//得到标识符对应的符号表下标
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind == ID_CONSTANT||table[i].kind == ID_PROCEDURE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}

		//在正式处理这个数组元素赋值之前，应该先处理好LDR和STR指令
		if(table[i].kind==ID_VARIABLE)		//普通变量，就跟以前一样处理
		{
			getsym();
			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}

			expression(fsys);	//处理表达式

			if (sym == SYM_SEMICOLON)		//处理结尾分号
			{
				getsym();
			}
			else
			{
				error(10);		//';' expected.
			}

			mk = (mask*) &table[i];		//指向ident对应的符号表单元
			if (i)
			{
				gen(STO, level - mk->level, mk->address);		//生成汇编指令，将赋值结果存入AR中相应的存储单元
			}
		}

		else if(table[i].kind==ID_ARRAY)		//数组变量
		{
			int j=0;	//记录已经处理的数组维数
			mk = (mask*) &table[i];		//指向数组名对应的符号表单元
			getsym();		//得到 [
			if (sym == SYM_LBRACKET)
			{
				getsym();		//得到表达式的第一个词法记号
			}
			else
			{
				error(30); // Missing '['.
			}

			set1 = createset(SYM_RBRACKET, SYM_NULL);
			set = uniteset(set1, fsys);
			//此处的表达式后继可以有 ]
			expression(set);	//处理第一维的表达式
			if (sym == SYM_RBRACKET)
			{
				getsym();		//得到 [      一维数组的话应该就是 :=
			}
			else
			{
				error(29); // Missing ']'.
			}

			j=1;
			while(j < mk->dimension)
			{
				if (sym == SYM_LBRACKET)
				{
					getsym();		//得到表达式的第一个词法记号
				}
				else
				{
					error(30); // Missing '['.
				}
				gen(LIT,0,mk->n[j]);
				gen(OPR,0,OPR_MUL);
				expression(set);	//处理表达式
				gen(OPR,0,OPR_ADD);
				if (sym == SYM_RBRACKET)
				{
					getsym();		//得到 [      数组维数用尽的话应该就是 :=
				}
				else
				{
					error(29); // Missing ']'.
				}
				j++;
			}
			destroyset(set1);
			destroyset(set);
			//此时运行时栈顶的唯一数据就是数组中目标元素的offset

			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}

			expression(fsys);	//处理表达式
			if (sym == SYM_SEMICOLON)		//处理结尾分号
			{
				getsym();
			}
			else
			{
				error(10);		//';' expected.
			}
			gen(STR, level - mk->level, mk->address);		//生成汇编指令，将赋值结果存入AR中相应的存储单元
		}
	}

	else if (sym == SYM_CALL)      //call ident ;
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];		//指向ident对应的符号表单元
				gen(CAL, level - mk->level, mk->address);		//生成汇编指令
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();		//得到分号
		}

		if (sym == SYM_SEMICOLON)		//处理结尾分号
		{
			getsym();
		}
		else
		{
			error(10);		//';' expected.
		}
	}

	else if (sym == SYM_IF)		//if 条件 then 语句1 (else 语句2)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);

		condition(set);	//处理条件

		destroyset(set1);
		destroyset(set);

		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}

		cx1 = cx;	//记录下面JPC指令的code下标，以便回填
		gen(JPC, 0, 0);		//根据下面是否有else，存在两种回填的可能

		statement(fsys);	//处理语句1

		//code[cx1].a = cx;
		if(sym==SYM_ELSE)	//存在else语句
		{
			getsym();
			cx2 = cx;	//记录下面JMP指令的code下标，以便回填
			gen(JMP, 0, 0);

			code[cx1].a=cx;		//有else时的JPC回填

			statement(fsys);	//处理语句2

			code[cx2].a=cx;		//JMP回填
		}
		else		//不存在else语句
		{
			code[cx1].a = cx;
		}

	}

	else if (sym == SYM_BEGIN)		//begin 语句序列 end
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);		//语句序列里至少有一条语句

		//我们来仔细推演一下，begin ... end中，...的最后一个分号的处理方案
		//sym为分号，再次getsym()，得到sym为end
		//end在statement()里没有任何匹配，相当于白执行一个statement()
		//然后回来，跳出下面这个while
		//一切结束………………
		//?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!?!     这TM是人能写出的代码？！？！？！？！？！？！？！？！？！？！
		//也就是说，无论最后一个分号是有还是没有，语法都是正确的，离开while循环的时候sym都是end
		//?!?!?!?!?!?!?!?!?!?!?!?!?!?!

		//一切都已经得到解决
		while (inset(sym, statbegsys))	//处理完一条语句之后，发现又有一条语句
		{
			//如果sys为ident而且不是var类型，下面的statement()会处理
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);

		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}

	else if (sym == SYM_WHILE)		//while 条件 do 语句
	{ // while statement
		cx1 = cx;		//记录条件的第一条代码的位置，以供JMP回来
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);	//处理条件
		destroyset(set1);
		destroyset(set);

		cx2 = cx;	//记录下面JPC指令的code下标，以便回填
		gen(JPC, 0, 0);		//目标是while循环后面的第一条代码

		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}

		statement(fsys);	//处理语句

		gen(JMP, 0, cx1);	//调回while开头的地方
		code[cx2].a = cx;	//回填上面的JPC
	}

	else if(sym == SYM_PRINT)		//print ( 表达式 , ... ) ;
	{
		getsym();		//得到 (
		if (sym == SYM_LPAREN)
		{
			getsym();		//得到表达式的第一项 或者 )
		}
		else
		{
			error(26); // Missing '('.
		}

		if(sym==SYM_RPAREN)		//print();
		{
			gen(PRT, 0, 0);		//这条指令输出换行
			getsym();
		}
		else
		{
			set1 = createset(SYM_RPAREN, SYM_COMMA, SYM_NULL);
			set = uniteset(set1, fsys);
			//此处的表达式后继可以有 ) 和 ,      print()函数里面的表达式大概不会有 ] 作为后继

			expression(set);	//处理表达式
			gen(PRT, 0, 1);		//这条指令输出表达式的值
			while(sym==SYM_COMMA)
			{
				getsym();
				expression(set);	//处理表达式
				gen(PRT, 0, 1);		//这条指令输出表达式的值
			}
			destroyset(set1);
			destroyset(set);

			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}

		if (sym == SYM_SEMICOLON)		//处理结尾分号
		{
			getsym();
		}
		else
		{
			error(10);		//';' expected.
		}
	}

	else if(sym == SYM_FOR)		//for ( var ident : ( low , up , step ) ) 语句
	{
		//low,up,step为负数的情况需要增加判定      毕竟这里没有expression()可以调用，都得靠自己手工处理
		int low,up,step;
		mask* mk;			//循环变量的符号表指针

		getsym();		//得到 (
		//鉴定并处理 (
		if (sym == SYM_LPAREN)
		{
			getsym();		//得到var
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 var
		if (sym == SYM_VAR)
		{
			getsym();		//得到ident
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 ident
		if (sym == SYM_IDENTIFIER)
		{
			enter(ID_VARIABLE);			//在这里dx++
			i = position(id);			//此时的id应该还是那个循环变量
			mk = (mask*) &table[i];		//指向循环变量对应的符号表单元
			getsym();		//得到:
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 :
		if (sym == SYM_COLON)
		{
			getsym();		//得到(
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 (
		if (sym == SYM_LPAREN)
		{
			getsym();		//得到low
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 low
		if (sym == SYM_NUMBER)
		{
			low=num;
			getsym();		//得到:
		}
		else if(sym == SYM_MINUS)		//读入的low为负数
		{
			getsym();		//得到low的绝对值
			if (sym == SYM_NUMBER)
			{
				low=-num;
				getsym();		//得到:
			}
			else
			{
				error(27); // Incorrect for.
			}
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 ,
		if (sym == SYM_COMMA)
		{
			getsym();		//得到up
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 up
		if (sym == SYM_NUMBER)
		{
			up=num;
			getsym();		//得到:或者)
		}
		else if(sym == SYM_MINUS)		//读入的up为负数
		{
			getsym();		//得到up的绝对值
			if (sym == SYM_NUMBER)
			{
				up=-num;
				getsym();		//得到:或者)
			}
			else
			{
				error(27); // Incorrect for.
			}
		}
		else
		{
			error(27); // Incorrect for.
		}

		//下面有两种可能：step是否缺省
		if(sym==SYM_RPAREN)		//step缺省，默认为1
		{
			step=1;
			getsym();		//得到)
		}
		else if(sym==SYM_COMMA)		//准备迎接step
		{
			getsym();		//得到step
			//鉴定并处理 step
			if (sym == SYM_NUMBER)
			{
				step=num;
				getsym();		//得到)
			}
			else if(sym == SYM_MINUS)		//读入的step为负数
			{
				getsym();		//得到step的绝对值
				if (sym == SYM_NUMBER)
				{
					step=-num;
					getsym();		//得到)
				}
				else
				{
				error(27); // Incorrect for.
				}
			}
			else
			{
				error(27); // Incorrect for.
			}

			//鉴定并处理 )
			if (sym == SYM_RPAREN)
			{
				getsym();		//得到)
			}
			else
			{
				error(27); // Incorrect for.
			}
		}
		else
		{
			error(27); // Incorrect for.
		}

		//鉴定并处理 )
		if (sym == SYM_RPAREN)
		{
			getsym();		//得到语句的第一个词法记号
		}
		else
		{
			error(27); // Incorrect for.
		}

		//for语句的首部都读完了，现在准备生成相应的汇编指令
		gen(LIT, 0, low);
		gen(STO, level - mk->level, mk->address);		//id=low
		cx1=cx;		//记下指令下标，将来JMP要用
		gen(LOD, level - mk->level, mk->address);		//循环变量放到栈顶
		gen(LIT, 0, up);
		if(step>0)   gen(OPR,0,OPR_LEQ);	//如果step>0，则在id<=up的时候继续循环
		else   gen(OPR,0,OPR_GEQ);			//如果step<0，则在id>=up的时候继续循环
		cx2=cx;		//记下JPC下标，将来回填
		gen(JPC, 0, 0);	//将来回填

		statement(fsys);	//处理语句

		gen(LOD, level - mk->level, mk->address);		//循环变量放到栈顶
		gen(LIT, 0, step);
		gen(OPR,0,OPR_ADD);
		gen(STO, level - mk->level, mk->address);		//id=id+step
		gen(JMP, 0, cx1);	//回去判断循环条件是否成立
		code[cx2].a = cx;	//回填上面的JPC

		//这个STO指令有点复杂，明天再说吧
		//语句处理
	}

	else if(sym==SYM_LONGJMP)	//longjmp ( 表达式 ， 表达式 ) ;
	{
		getsym();		//得到 (
		//鉴定并处理 (
		if (sym == SYM_LPAREN)
		{
			getsym();		//得到第一个表达式的第一个词法记号
		}
		else
		{
			error(26); // Missing '('.
		}

		set = uniteset(createset(SYM_RPAREN, SYM_COMMA, SYM_NULL), fsys);
		//表达式的后继可以有)和,了
		expression(set);		//处理第一个表达式

		if (sym == SYM_COMMA)
		{
			getsym();		//得到第二个表达式的第一个词法记号
		}
		else
		{
			error(5); // Missing ',' or ';'.
		}
		expression(set);		//处理第二个表达式
		destroyset(set);

		if (sym == SYM_RPAREN)
		{
			getsym();		//得到;
		}
		else
		{
			error(22); // Missing ')'.
		}

		if (sym == SYM_SEMICOLON)		//处理结尾分号
		{
			getsym();
		}
		else
		{
			error(10);		//';' expected.
		}

		gen(LJP,0,0);
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
//fsys作为一个常值参数链表，不知道有什么用      不是常值链表，主要用来错误处理，偶尔还能判断语法是否正确   似乎是非终结符的后继
//处理一个procedure      这里的procedure似乎就是程序块block
//应该叫，程序体     处理非终结符之————程序体
void block(symset fsys)
{
	int cx0; // initial code index     这里的编译器为每一个procedure生成一段汇编，cx0指明当前procedure的第一条汇编的code下标
	mask* mk;			//指向对应本procedure名的符号表单元
	//开局的main过程没有过程名，但是符号表的第一个位置也算是给它留的
	int block_dx;		//当前程序块的AR对应的数据存贮的大小
	int savedTx;		//用了这个变量，为了节约一点符号表的空间
	symset set1, set;	//不知道是什么

	dx = 3;		//程序块初始的数据存贮大小为3
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;		//暂时记录下面的JMP指令在code数组中的下标，以便将来回填
	//其实这个mk->address根本就不是干这个用的，这里只是当临时变量来用（代码风格极差）

	gen(JMP, 0, 0);		//每个procedure的开头都有一个JMP
	//因为这个procedure里可能还会嵌套procedure，这个JMP是为了跳过嵌套的procedure，以抵达本procedure的代码部分
	//JMP到哪里，当前应该是不知道的，将来肯定要回填

	if (level > MAXLEVEL)		//当前procedure嵌套太深
	{
		error(32); // There are too many levels.
	}

	do
	{
		if (sym == SYM_CONST)	//const a=5,b=6,c=7;
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();		//干掉了a=5
				while (sym == SYM_COMMA)	//干掉了b=6,c=7
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)	//干掉了;
				{
					getsym();		//取下一个词法记号
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);	//看来 const a=5;b=6;c=7; 也是合法的
		} // if

		if (sym == SYM_VAR)		//var x,y,z;    看来变量不允许赋初值
		{ // variable declarations       dx只有在这里才会增加
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);		//看来 var x;y;z; 也是合法的
		} // if

		block_dx = dx; //save dx before handling procedure call!
		//记下本procedure里定义的局部变量数目
		//作为全局变量，dx马上就要给嵌套的procedure用了，所以这里必须记下来

		//可能嵌套定义了多个函数，因此用while
		while (sym == SYM_PROCEDURE)		//procedure multiply;
		{ // procedure declarations
			getsym();		//得到过程名multiply
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);		//此时的level还是调用函数的level，即为母函数的level
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}

			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
			//函数首部处理完成

			level++;		//嵌套深度+1
			savedTx = tx;	//这个嵌套procedure的过程名在符号表中的位置
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);		//嵌套函数的主体在这里
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			//注意！！这里的tx变回来，是要把定义的子过程的符号表删掉，因为子过程的局部变量在子过程结束之后就不能用了
			level--;

			if (sym == SYM_SEMICOLON)		//一个procedure的末尾
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while

		dx = block_dx; //restore dx after handling procedure call!   这有什么用呢………………
		//可能var有若干行（while循环若干次），这里接续上一行来
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);		//下一个只能是id、begin、call、if、while    下面以为的循环没了…………
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));		//看来未必需要严格按照const->var->procedure的顺序来，只要保证这三者都在函数实际运行代码前面就行
	//别TM循环了…………就只能照着const->var->procedure的顺序来…………

	code[mk->address].a = cx;		//JMP跳到的地方回填
	mk->address = cx;		//过程的符号表单元里address填的是本procedure的第一条汇编指令的绝对地址
	cx0 = cx;		//当前procedure的第一条汇编(是下面的INT，而不是上面的JMP)的code下标    可以用来回填
	gen(INT, 0, 0);			//为本procedure的AR分配空间           上面的JMP跳到的就是这条指令
	//为了给for循环的循环变量留出空间，这个INT的值必须回填
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_ELSE, SYM_NULL);
	//加了个else
	set = uniteset(set1, fsys);

	statement(set);			//这个procedure里的指令都是在这生成的
	//在statement()里，如果看到了for，则还是需要enter()，同时dx也要++

	destroyset(set1);
	destroyset(set);
	code[cx0].a=dx;			//for语句都看到了，此时的dx为真正的dx，可以回填了
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block


//////////////////////////////////////////////////////////////////////
//调用这个函数的时候，currentLevel是当前执行过程的SL字段在stack中的下标，levelDiff是当前执行过程与目标变量所在过程的静态层次差。
//返回目标变量所在过程的SL字段的下标（就是AR基地址的指针）
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
//执行PL0汇编语言     跟编译器就没什么关系了
//code数组早就准备好了      那code里到底有多少汇编代码，它是怎么知道的呢…………
//每个过程开局三件套的顺序是：SL、DL、RA
//各个过程的AR与运行时计算栈共用一个stack数组………………
//逻辑上，每个AR的计算都是独立的，所以每个AR结束的时候，该AR的计算栈应该恰好清空，此时top指向该AR的末尾
//计算栈应该是每执行完一个语句就会清空，因此调用子过程的时候应该也不会有未清空计算栈
void interpret()
{
	int pc;        // program counter    指向下一条执行的汇编代码
	int stack[STACKSIZE];	//从stack[1]开始用
	int top;       // top of stack    当前栈顶的下标（不是最后一个的下一个空位），指的可能是AR里的局部变量，也可能是计算栈的栈顶
	int b;         // program, base, and top-stack register     指向栈顶AR的基地址，即为SL字段
	instruction i; // instruction register		存储当前执行的汇编指令

	int jmp_buf[JMPMAX];		//setjmp()的存档
	int j;		//循环变量
	int value;	//恢复存档之前，用来临时存储longjmp()的参数

	printf("Begin executing PL/0 program.\n");

	pc = 0;		//第一条执行的，自然是code[0]
	b = 1;		//指向main过程的SL
	top = 3;	//初始时栈顶指向stack[3]      为什么不是0呢？？？？？留出冗余空间吗？？？？？？只有main过程有冗余？？？？？
	stack[1] = stack[2] = stack[3] = 0;		//main过程的SL、DL、RA皆为0

	do
	{
		i = code[pc++];		//得到当前执行的汇编指令

		switch (i.f)
		{
		case LIT:					//栈顶存一个常数
			stack[++top] = i.a;
			break;

		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:			//当前过程运行结束，收回AR     特别是main函数RET的时候，pc变成0，进而结束整个interpret()
				top = b - 1;	//b为当前AR的开头，b-1就是前一个AR的结尾
				pc = stack[top + 3];	//动用RA
				b = stack[top + 2];		//动用DL
				//上面三条代码的逻辑是对的，但是这么用栈是不是不太好…………
				break;

			case OPR_NEG:			//栈顶元素取相反数
				stack[top] = -stack[top];
				break;
			case OPR_ADD:			//stack[top-1] + stack[top]，结果入栈顶
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:			//stack[top-1] - stack[top]，结果入栈顶
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:			//stack[top-1] * stack[top]，结果入栈顶
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:			//stack[top-1] / stack[top]，结果入栈顶
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:			//栈顶元素为奇数则为1，进而执行条件语句；栈顶元素为偶数则为0，进而跳过条件语句
				stack[top] %= 2;
				break;

			case OPR_EQU:			//stack[top-1] == stack[top]成立，则栈顶为1,执行；反之则跳过
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:			//同上
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:			//同上
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:			//同上
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:			//同上
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:			//同上
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;

		case LOD:					//栈顶存一个变量    首先需要找到这个变量存在哪里
			stack[++top] = stack[base(stack, b, i.l) + i.a];	//基址 + 偏移
			break;

		case STO:					//把栈顶值赋给某个变量    首先需要找到这个变量存在哪里
			stack[base(stack, b, i.l) + i.a] = stack[top];
			top--;
			break;

		case CAL:					//调用子过程，这里先把新AR的开局三件套整好，新AR的基地址填上，并完成PC跳转
			stack[top + 1] = base(stack, b, i.l);	//生成SL
			// generate new block mark
			stack[top + 2] = b;		//生成DL
			stack[top + 3] = pc;	//生成RA
			//其实这很不符合用栈的规范，但是马上就会INT，就将就将就吧…………
			b = top + 1;
			pc = i.a;
			break;

		case INT:					//为新的AR分配空间
			top += i.a;
			break;

		case JMP:					//无条件改变PC
			pc = i.a;
			break;

		case JPC:					//根据栈顶单元，==0则改变PC，!=0则PC不变     栈顶元素用完后就扔掉了
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;

		case PRT:					//根据a的值来决定print什么
			switch (i.a) // operator
			{
			case 0:			//输出'\n'
				printf("\n");
				break;

			case 1:			//输出栈顶元素
				printf("%d ",stack[top]);
				top--;
				break;
			}
			break;

		case LDR:					//栈顶存一个变量    首先需要找到这个变量存在哪里
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];	//基址 + 偏移
			break;

		case STR:					//把栈顶值赋给某个变量    首先需要找到这个变量存在哪里
			stack[base(stack, b, i.l) + i.a + stack[top-1]] = stack[top];
			top=top-2;
			break;

		case SJP:					//存档指令
			stack[top]=0;			//直接调用setjmp()返回0   setjmp()的参数P用没有，直接抹掉…………
			for(j=0;j<=top;j++)
			{
				jmp_buf[j]=stack[j];
			}
			jmp_buf[JMPMAX-3]=pc;
			jmp_buf[JMPMAX-2]=top;
			jmp_buf[JMPMAX-1]=b;
			break;

		case LJP:					//恢复setjmp()的存档
			value=stack[top];		//保存longjmp()的后一个参数     前一个参数P用没有
			//下面恢复存档
			pc=jmp_buf[JMPMAX-3];
			top=jmp_buf[JMPMAX-2];
			b=jmp_buf[JMPMAX-1];
			for(j=0;j<=top;j++)
			{
				stack[j]=jmp_buf[j];
			}
			//最后，这里间接调用的setjmp()返回值就不能是0
			stack[top]=value;
			break;
		} // switch
	}
	while (pc);

	printf("\nEnd executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;		//一个生成的中间文件，暂时不知道有什么用   P用没有
	char s[80];		//输入的PL0高级语言的程序文件名
	int i;
	symset set, set1, set2;		//开了3个未知链表

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets   即为非终结符的FIRST集合?    这些链表吧，其用处难以言说…………
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_IDENTIFIER, SYM_PRINT, SYM_FOR, SYM_LONGJMP, SYM_NULL);
	//加了一个ident，这里非常关键，是修改statement()里分号判定的基础     再加一个print和一个for以及一个longjmp
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_SETJMP, SYM_NULL);
	//现在加了一个setjmp

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();		//开局先整进来一个记号名，以供下面block

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	//set1和set2都没用，只有set作为参数有用

	block(set);		//这个函数大概是重头戏    所有的汇编指令在这里生成

	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)		//最后剩下的，只有最后的.了
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);		//这有个P用…………
		fclose(hbin);
	}
	if (err == 0)
		interpret();		//对于没有错误的PL0高级语言程序，这里还要执行一遍
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);		//把已经生成好的PL0汇编语言打印一遍
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c

