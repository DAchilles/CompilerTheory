/*可以参考文献[2]Flex&Bison中第3章Bison内容58页*/
%error-verbose
%locations
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
void yyerror(const char* fmt, ...);
%}
/*
给出语法分析中非终结符和终结符的类型，供不同文法符号使用；
对无语义子程序的规则，默认使用$$=$1，需要相应的类型匹配；默认类型是整型；
样例中给出了整型、浮点型、字符串数组、抽象语法树节点4种，具体根据自己情况增减
*/
%union {
	int     type_int;
	float   type_float;
    char    type_char;
	char    type_id[32];
	struct node *ptr;
};

/*
用%type来指定非终结符的语义值类型，
用<>选择union中某个类型，
后面列出同类型的非终结符
*/
%type <ptr> Program ExtDefList ExtDef Specifier ExtDecList FuncDec ArrayDec CompSt VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args

/* 用%token来指定终结符的语义值类型，与非终结符类似*/
%token <type_int> INT                   //指定是type_int类型，用于AST树建立
%token <type_id> ID RELOP TYPE          //指定是type_id 类型
%token <type_float> FLOAT               //指定是type_float类型
%token <type_char> CHAR                 //指定是type_char类型

%token LP RP LB RB LC RC SEMI COMMA   
%token PLUS MINUS STAR DIV ASSIGNOP AND OR NOT IF ELSE WHILE RETURN

%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UMINUS NOT
/*%nonassoc的含义是没有结合性,它一般与%prec结合使用表示该操作有同样的优先级*/
%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%%

/*显示语法树*/
Program:
    ExtDefList {
        display($1,0);
        semantic_analysis($1,0,0,' ',0);
        }
    ;

/*ExtDefList：外部定义列表，即是整个语法树*/
ExtDefList:
    //整个语法树为空
    { $$=NULL;}
    //每一个EXTDEFLIST的结点,其第1棵子树对应一个外部变量声明或函数
    | ExtDef ExtDefList {
        $$ = mknode( EXT_DEF_LIST, $1, $2, NULL, yylineno);
        }
    ;

/*外部声明，声明外部变量或者声明函数*/
ExtDef:
    //该结点对应一个外部变量声明
    Specifier ExtDecList SEMI {
        $$ = mknode( EXT_VAR_DEF, $1, $2, NULL, yylineno);
        }
    //该结点对应一个函数定义
    | Specifier FuncDec CompSt {
        $$ = mknode(FUNC_DEF, $1, $2, $3, yylineno);
        }         
    //该结点对应一个数组定义
    | Specifier ArrayDec SEMI {
        $$ = mknode(ARRAY_DEF, $1, $2, NULL, yylineno);
        }
    //报错
    | error SEMI {
        $$ = NULL;
        }
    ;

/*表示一个类型，int、float和char*/
Specifier:
    TYPE {
        $$ = mknode(TYPE, NULL, NULL, NULL, yylineno);
        strcpy($$->type_id,$1);
        $$->type = (!strcmp($1,"int")?INT:(!strcmp($1,"float")?FLOAT:CHAR));
        }
    ;

/*变量名称列表，由一个或多个变量组成，多个变量之间用逗号隔开*/
ExtDecList:
    //每一个EXT_DECLIST的结点，其第一棵子树，对应一个变量名(ID类型的结点)，第二棵子树，对应剩下的外部变量名
    VarDec {
        $$=$1;
        }       
    | VarDec COMMA ExtDecList {
        $$=mknode(EXT_DEC_LIST,$1,$3,NULL,yylineno);
        }
    ;

/*变量名称，由一个ID组成*/
VarDec:
    //ID结点，标识符符号串存放结点的type_id
    ID {
        $$=mknode(ID,NULL,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    ;

/*函数名+参数定义*/
FuncDec:
    //函数名存放在$$->type_id
    ID LP VarList RP {
        $$=mknode(FUNC_DEC,$3,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
	| ID LP RP {
        $$=mknode(FUNC_DEC,NULL,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | error RP {
        $$=NULL;
        printf("---函数左括号右括号不匹配---\n");
        }
    ;

/*数组声明*/
ArrayDec:
    ID LB Exp RB {
        $$=mknode(ARRAY_DEC,$3,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | ID LB RB {
        $$=mknode(ARRAY_DEC,NULL,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | error RB {
        $$=NULL;
        printf("---数组定义错误---\n");
    }
    ;

/*参数定义列表，有一个到多个参数定义组成，用逗号隔开*/
VarList:
    ParamDec {
        $$=mknode(PARAM_LIST,$1,NULL,NULL,yylineno);
        }
    | ParamDec COMMA VarList {
        $$=mknode(PARAM_LIST,$1,$3,NULL,yylineno);
        }
    ;

/*参数定义，一个类型和一个变量组成*/
ParamDec:
    Specifier VarDec {
        $$=mknode(PARAM_DEC,$1,$2,NULL,yylineno);
        }
    ;
/*复合语句，左右分别用大括号括起来，中间有定义列表和语句列表*/
CompSt:
    LC DefList StmList RC {
        $$=mknode(COMP_STM,$2,$3,NULL,yylineno);
        }
    | error RC {
        $$=NULL;
        printf("---复合语句错误---\n");
        }
    ;

/*语句列表，由0个或多个语句stmt组成*/
StmList:
    { $$=NULL;}  
    | Stmt StmList {
        $$=mknode(STM_LIST,$1,$2,NULL,yylineno);
        }
    ;

/*语句，可能为表达式，复合语句，return语句，if语句，if-else语句，while语句*/
Stmt:
    Exp SEMI {
        $$=mknode(EXP_STMT,$1,NULL,NULL,yylineno);
        }
    //复合语句结点直接最为语句结点，不再生成新的结点
    | CompSt {
        $$=$1;
        }
    | RETURN Exp SEMI {
        $$=mknode(RETURN,$2,NULL,NULL,yylineno);
        }
    | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE {
        $$=mknode(IF_THEN,$3,$5,NULL,yylineno);
        }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$=mknode(IF_THEN_ELSE,$3,$5,$7,yylineno);
        }
    | WHILE LP Exp RP Stmt {
        $$=mknode(WHILE,$3,$5,NULL,yylineno);
        }
    ;

/*定义列表，由0个或多个定义语句组成*/
DefList:
    { $$=NULL;}
    | Def DefList {
        $$=mknode(DEF_LIST,$1,$2,NULL,yylineno);
        }
    ;

/*定义一个或多个语句语句，由分号隔开*/
Def:
    Specifier DecList SEMI {
        $$=mknode(VAR_DEF,$1,$2,NULL,yylineno);
        }
    | Specifier ArrayDec SEMI {
        $$=mknode(ARRAY_DEF,$1,$2,NULL,yylineno);
        }
    ;

/*语句列表，由一个或多个语句组成，由逗号隔开，最终都成一个表达式*/
DecList:
    Dec {
        $$=mknode(DEC_LIST,$1,NULL,NULL,yylineno);
        }
    | Dec COMMA DecList {
        $$=mknode(DEC_LIST,$1,$3,NULL,yylineno);
        }
	;

/*语句，一个变量名称或者一个赋值语句（变量名称等于一个表达式）*/
Dec:
    VarDec {
        $$=$1;
        }
    | VarDec ASSIGNOP Exp {
        $$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"ASSIGNOP");
        }
    ;

/*表达式*/
Exp:
    //$$结点type_id空置未用，正好存放运算符
    // "="
    Exp ASSIGNOP Exp {
        $$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"ASSIGNOP");
        }
    // "&&"
    | Exp AND Exp {
        $$=mknode(AND,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"AND");
        }
    // "||"
    | Exp OR Exp {
        $$=mknode(OR,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"OR");
        }
    //词法分析关系运算符号自身值保存在$2中
    | Exp RELOP Exp {
        $$=mknode(RELOP,$1,$3,NULL,yylineno);
        strcpy($$->type_id,$2);
        }
    // "+"
    | Exp PLUS Exp {
        $$=mknode(PLUS,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"PLUS");
        }
    // "-"
    | Exp MINUS Exp {
        $$=mknode(MINUS,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"MINUS");
        }
    // "*"
    | Exp STAR Exp {
        $$=mknode(STAR,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"STAR");
        }
    // "/"
    | Exp DIV Exp {
        $$=mknode(DIV,$1,$3,NULL,yylineno);
        strcpy($$->type_id,"DIV");
        }
    //遇到左右括号，可直接忽略括号，Exp的值就为括号里面的Exp
    | LP Exp RP {
        $$=$2;
        }
    | MINUS Exp %prec UMINUS {
        $$=mknode(UMINUS,$2,NULL,NULL,yylineno);
        strcpy($$->type_id,"UMINUS");
        }
    | NOT Exp {
        $$=mknode(NOT,$2,NULL,NULL,yylineno);
        strcpy($$->type_id,"NOT");
        }
    | ID LP Args RP {
        $$=mknode(FUNC_CALL,$3,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | ID LP RP {
        $$=mknode(FUNC_CALL,NULL,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | ID LB Exp RB {
        $$=mknode(ARRAY_DEC,$3,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | ID {
        $$=mknode(ID,NULL,NULL,NULL,yylineno);
        strcpy($$->type_id,$1);
        }
    | INT {
        $$=mknode(INT,NULL,NULL,NULL,yylineno);
        $$->type_int=$1;$$->type=INT;
        }
    | FLOAT {
        $$=mknode(FLOAT,NULL,NULL,NULL,yylineno);
        $$->type_float=$1;
        $$->type=FLOAT;
        }
    | CHAR {
        $$=mknode(CHAR,NULL,NULL,NULL,yylineno);
        $$->type_char=$1;
        $$->type=CHAR;
        }
    ;

/*用逗号隔开的参数*/
Args:
    Exp COMMA Args {
        $$=mknode(ARGS,$1,$3,NULL,yylineno);
        }
    | Exp {
        $$=mknode(ARGS,$1,NULL,NULL,yylineno);
        }
    ; 

%%
int main(int argc, char *argv[])
{
	yyin=fopen(argv[1],"r");
	if (!yyin)
        return -1;
	yylineno=1;
	yyparse();
	return 0;
}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error Found at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}	
