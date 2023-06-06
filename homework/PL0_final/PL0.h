#include <stdio.h>

#define NRW 16		 // number of reserved words      16个关键字的编号都在下面		一会关键字++  else print for setjmp longjmp
#define TXMAX 500	 // length of identifier table      标识符的最多数量
#define MAXNUMLEN 14 // maximum number of digits in numbers     数字常数的最大位数
#define NSYM 11		 // maximum number of symbols in array ssym and csym    11种“自带算符”的编号都在下面    数组需要 [ 和 ]，for循环需要 :
#define MAXIDLEN 10	 // length of identifiers      标识符的最大长度

#define MAXADDRESS 32767 // maximum address   这是什么地址  是运行时数据存贮的大小吗…………    应该是数字常数num的最大值
#define MAXLEVEL 32		 // maximum depth of nesting block    函数嵌套定义最多嵌套32层
#define CXMAX 500		 // size of code array      最多只能生成500条汇编

#define MAXSYM 38 // maximum number of symbols   词法记号的记号名类型共有38种		一会词法记号名++

#define STACKSIZE 1000 // maximum storage    运行时数据存贮的大小只有1000

#define JMPMAX 4096		//setjmp()的存档大小

enum symtype //词法记号的记号名类型
{
	SYM_NULL,		//不认识…………
	SYM_IDENTIFIER, //id，在下面这个又细分了三种
	SYM_NUMBER,		//数字常量
	SYM_PLUS,		//“自带算符”1   +
	SYM_MINUS,		//“自带算符”2   -   两种含义：减号、负号
	SYM_TIMES,		//“自带算符”3   *
	SYM_SLASH,		//“自带算符”4   大概是除号/吧…………
	SYM_ODD,		//关键字7   odd
	SYM_EQU,		//=
	SYM_NEQ,		//<>
	SYM_LES,		//<
	SYM_LEQ,		//<=
	SYM_GTR,		//>
	SYM_GEQ,		//>=
	SYM_LPAREN,		//“自带算符”5   left parenthesis 左括号 (
	SYM_RPAREN,		//“自带算符”6   right parenthesis 右括号 )
	SYM_COMMA,		//“自带算符”7   逗号 ,
	SYM_SEMICOLON,	//“自带算符”9  分号 ;
	SYM_PERIOD,		//“自带算符”8   句号 .
	SYM_BECOMES,	//赋值号 :=
	SYM_BEGIN,		//关键字1   begin
	SYM_END,		//关键字5   end
	SYM_IF,			//关键字6   if
	SYM_THEN,		//关键字9   then
	SYM_WHILE,		//关键字11  while
	SYM_DO,			//关键字4   do
	SYM_CALL,		//关键字2   call
	SYM_CONST,		//关键字3   const
	SYM_VAR,		//关键字10  var
	SYM_PROCEDURE,	//关键字8   procedure
	SYM_ELSE,		//关键字12   else
	SYM_PRINT,		//关键字13   print
	SYM_FOR,		//关键字14   for
	SYM_COLON,		//冒号 :     不能算“自带算符”，因为这个跟:=冲突了
	SYM_LBRACKET,	//“自带算符”10   left bracket 左中括号 [
	SYM_RBRACKET,	//“自带算符”11   right bracket 右中括号 ]
	SYM_SETJMP,		//关键字15   setjmp
	SYM_LONGJMP		//关键字16   longjmp
};

enum idtype // id的三种细分			将来还会有数组类型
{
	ID_CONSTANT,		//标识符为常数
	ID_VARIABLE,		//标识符为变量
	ID_PROCEDURE,		//标识符为过程
	ID_ARRAY			//标识符为数组
};

enum opcode //汇编语言的13种指令类型			将来会有用来实现print()的PRT指令
{
	LIT,
	OPR,
	LOD,
	STO,
	CAL,
	INT,
	JMP,
	JPC,
	PRT,	//它来了
	LDR,	//为了实现数组元素的查找而专门打造的
	STR,	//为了实现数组元素的查找而专门打造的
	SJP,	//setjmp()函数的指令
	LJP		//longjmp()函数的指令
};

enum oprcode //汇编指令的OPR的各种操作
{
	OPR_RET,
	OPR_NEG,		//负号的-
	OPR_ADD,
	OPR_MIN,		//减号的-
	OPR_MUL,
	OPR_DIV,
	OPR_ODD,
	OPR_EQU,
	OPR_NEQ,
	OPR_LES,
	OPR_LEQ,
	OPR_GTR,
	OPR_GEQ
};
//上面这些枚举类型，其实根本就没定义这种类型的变量，它们只是为了定义各种操作符的名字
//作用跟#define差不多

typedef struct //一条指令的三个组成部分
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
//上面是数据结构，下面开始定义编译器需要的变量名
//神奇的错误处理
char *err_msg[] =
	{
		/*  0 */ "",
		/*  1 */ "Found ':=' when expecting '='.",
		/*  2 */ "There must be a number to follow '='.",
		/*  3 */ "There must be an '=' to follow the identifier.",
		/*  4 */ "There must be an identifier to follow 'const', 'var', or 'procedure'.",
		/*  5 */ "Missing ',' or ';'.",
		/*  6 */ "Incorrect procedure name.",
		/*  7 */ "Statement expected.",
		/*  8 */ "Follow the statement is an incorrect symbol.",
		/*  9 */ "'.' expected.",
		/* 10 */ "';' expected.",
		/* 11 */ "Undeclared identifier.",
		/* 12 */ "Illegal assignment.",
		/* 13 */ "':=' expected.",
		/* 14 */ "There must be an identifier to follow the 'call'.",
		/* 15 */ "A constant or variable can not be called.",
		/* 16 */ "'then' expected.",
		/* 17 */ "';' or 'end' expected.",
		/* 18 */ "'do' expected.",
		/* 19 */ "Incorrect symbol.",
		/* 20 */ "Relative operators expected.",
		/* 21 */ "Procedure identifier can not be in an expression.",
		/* 22 */ "Missing ')'.",
		/* 23 */ "The symbol can not be followed by a factor.",
		/* 24 */ "The symbol can not be as the beginning of an expression.",
		/* 25 */ "The number is too great.",
		//下面就是我自己新加的了
		/* 26 */ "Missing '('.",
		/* 27 */ "Incorrect for.",
		/* 28 */ "A number is expected here.",
		/* 29 */ "Missing ']'.",
		/* 30 */ "Missing '['.",
		/* 31 */ "",
		/* 32 */ "There are too many levels."};

//////////////////////////////////////////////////////////////////////
char ch;			   // last character read		每次读入一个字符
int sym;			   // last symbol read		若干字符拼成一个词法记号，其记号名对应的序号
char id[MAXIDLEN + 1]; // last identifier read
int num;			   // last number read
int cc;				   // character count    本行当前读到的字符序号
int ll;				   // line length     本行所有的字符数量
int kk;				   //这个变量似乎没用
int err;		//发现的错误数量
int cx;		   // index of current instruction to be generated.	下一条生成的指令的序号，指的是code数组中最后一条已生成指令的下一位空位
int level = 0; //生成符号表和汇编指令时才会用到     记录当前所在procedure的绝对层次   main过程是0
int tx = 0;	   //主要也是用于符号表    当前符号表中最后一个标识符的序号（不是最后一个的下一个空位）
//只有gen()能使cx++，只有enter()能使tx++

char line[80]; //记录当前正在读的行的字符串

instruction code[CXMAX]; //记录生成的汇编指令的数组

char *word[NRW + 1] = //关键字数组，记的是关键字的字符串
	{
		"", /* place holder */
		"begin", "call", "const", "do", "end", "if",
		"odd", "procedure", "then", "var", "while",
		"else", "print", "for", "setjmp", "longjmp"};

//原来枚举类型不需要真的用enum类型的变量来引用…………
int wsym[NRW + 1] = //关键字数组，记的是关键字的序号
	{
		SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
		SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
		SYM_ELSE, SYM_PRINT, SYM_FOR, SYM_SETJMP, SYM_LONGJMP};

int ssym[NSYM + 1] = //“自带算符”数组，记的是“自带算符”的序号
	{
		SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
		SYM_LPAREN, SYM_RPAREN, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
		SYM_LBRACKET, SYM_RBRACKET};

char csym[NSYM + 1] = //“自带算符”数组，记的是“自带算符”的字符
	{
		' ', '+', '-', '*', '/', '(', ')', ',', '.', ';', '[', ']'};

#define MAXINS 13 	//指令的种类一共13种		将来会增加 PRT、SETJMP、LONGJMP
char *mnemonic[MAXINS] =		//指令序号对应的字符串
	{
		"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "PRT", "LDR", "STR", "SJP", "LJP"};


//下面是符号表的炫酷狂拽D炸天的骚操作
typedef struct
{
	char name[MAXIDLEN + 1];
	int kind;		//常数，变量，过程，数组   四选一
	int value;		//对于kind为常数的标识符，value记录这个常数
	int dimension;	//数组维数，只对kind为数组的标识符有效
	int n[10];		//数组每一维的大小   数组最多为10维
} comtab;		//符号表中的常数项

comtab table[TXMAX];		//符号表

//对于kind为变量或者过程的标识符，value就变成了level和address（空间大小相同，应该可以变）
//此时符号表的项由comtab类型转换成mask类型，value域就变成了level域和address域
typedef struct
{
	char name[MAXIDLEN + 1];
	int kind;
	short level;		//对于kind为变量或者过程或者数组的标识符，level记录该标识符的绝对嵌套深度
	short address;		//对于kind为变量的标识符，address记录该变量在所在过程的AR中的相对位置（相对基址的偏移）
						//对于kind为过程的标识符，address记录该过程第一行汇编代码（INT）在code数组中的绝对下标。
						//对于kind为数组的标识符，address记录这个整个数组变量在所在过程的AR中的基址
	int dimension;		//数组维数，只对kind为数组的标识符有效
	int n[10];			//数组每一维的大小   数组最多为10维
} mask;		//符号表中的变量项和过程项以及数组项

//严格来说，应该开三个不一样的表的，这种炫酷狂拽D炸天的骚操作也许不太规范…………

FILE *infile;		//写了PL0高级语言的程序文件

// EOF PL0.h
