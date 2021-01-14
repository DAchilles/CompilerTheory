#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "parser.tab.h"

enum node_kind{
    EXT_DEF_LIST, EXT_VAR_DEF, FUNC_DEF, FUNC_DEC, EXT_DEC_LIST,
    PARAM_LIST, PARAM_DEC, VAR_DEF, DEC_LIST, DEF_LIST, COMP_STM,
    STM_LIST, EXP_STMT, IF_THEN, IF_THEN_ELSE, FUNC_CALL, ARGS, FUNCTION,
    PARAM, ARG, CALL, LABEL, GOTO, JLT, JLE, JGT, JGE, EQ, NEQ,
    //AUTOADD_L, AUTOADD_R, AUTOSUB_L, AUTOSUB_R, 
    ARRAY_DEF, ARRAY_DEC, ARRAY
};
#define SYMBOLTABLESIZE   1000    //定义符号表的大小
#define DX 3*sizeof(int)          //活动记录控制信息需要的单元数

/*以下包括语法树、中间代码存储需要的类型等定义，仅供参考，请根据自己的需要修改定义*/
struct operandStruct{   /*中间代码操作数信息*/
    int kind;                   //操作的类型标识
    int type;                   //操作数的类型标识
    union {
        int     const_int;      //整常数值，立即数
        float   const_float;    //浮点常数值，立即数
        char    const_char;     //字符常数值，立即数
        char    id[33];         //变量、临时变量的别名、标号字符串
        };
    int level;                  //变量的层号，0表示是全局变量，数据保存在静态数据区
    int offset;                 //变量单元偏移量，或函数在符号表的位置
};

struct codenode {   //中间代码TAC结点,采用双向循环链表存放
    int  op;                          //TAC代码的运算符种类
    struct operandStruct opn1, opn2, result;          //2个操作数和运算结果
    struct codenode  *next, *prior;
};

struct node {   //以下对结点属性定义没有考虑存储效率，只是简单地列出要用到的一些属性
	enum node_kind kind;        //结点类型
	union{
		char    type_id[33];    //由标识符生成的叶结点
		int     type_int;       //由整常数生成的叶结点
		float   type_float;     //由浮点常数生成的叶结点
        char    type_char;      //由字符型生成的叶节点
	};
    struct node *ptr[3];        //子树指针，由kind确定有多少棵子树
    int level;                  //层号
    int place;                  //表示结点对应的变量或运算结果临时变量在符号表的位置序号
    char Etrue[15],Efalse[15];  //对布尔表达式的翻译时，真假转移目标的标号
    char Snext[15];             //该结点对应语句执行后的下一条语句位置标号
    struct codenode *code;      //该结点中间代码链表头指针
    char op[10];
    int  type;                  //结点对应值的类型
    int position;               //语法单位所在位置行号
    int offset;                 //偏移量
    int width;                  //各种数据占用的字节数
    int num;                    //定义变量的数量
};

/*符号表中元素结构*/
struct symbol{  //这里只列出了一个符号表项的部分属性，没考虑属性间的互斥
    char name[33];              //变量或函数名
    int level;                  //层号，外部变量名或函数名，层号为0；形参为1；进入复合语句加1，退出减1
    int type;                   //变量类型或函数返回值类型
    int  paramnum;              //形式参数个数
    char alias[33];             //别名，为解决嵌套层次使用，可以使每个数据名称唯一
    char flag;                  //符号标记缩写，函数：'F'  变量：'V'   参数：'P'  临时变量：'T'
    char offset;                //外部变量和局部变量，在其静态数据区或活动记录中的偏移量 或 函数活动记录大小，目标代码生成时使用
    //其它你需要补充的信息可以增加保存...
};

//符号表，是一个顺序栈，index初值为0
struct SymbolTable{
    struct symbol symbols[SYMBOLTABLESIZE];
    int index;
};

/*
保存当前作用域的符号在符号表中起始位置的序号；
栈结构，每到达一个复合语句时，将符号表的index值进栈；
离开复合语句时，取其退栈值修改符号表的index值，
可以实现删除该复合语句中的所有局部变量和临时变量
*/
struct SymbolScopeArray{
    int ScopeArray [30];
    int top;
};

struct node *mknode(int kind,struct node *first,struct node *second, struct node *third,int position );
void display(struct node *,int);

void semantic_analysis_init(struct node *T);
void semantic_analysis(struct node* T);
void Exp(struct node *T);
void ext_var_list(struct node *T);
void semantic_error(int line, char *msg1, char *msg2);
int search_symbol_table(char *name);
int fill_symbol_table(char *name, char *alias, int level, int type, char flag, int offset);
int temp_add(char *name, int level, int type, char flag, int offset);
int match_param(int i, struct node *T);
void bool_exp(struct node *T);
char *str_catch(char *s1, char *s2);
char *new_alias();
char *new_label();
char *new_temp();
struct codenode *genIR(int op, struct operandStruct opn1, struct operandStruct opn2, struct operandStruct result);
struct codenode *genLabel(char *label);
struct codenode *genGoto(char *label);
struct codenode *merge(int num, ...);
void print_IR(struct codenode *head);