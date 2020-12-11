%{
#include "parser.tab.h"      /*需要与bison联动后加上*/
#include "string.h"
#include "def.h"           /*自定义数据结构、函数头文件*/
int yycolumn=1;          
/*TODO:完成语法单元定位的宏YY_USER_ACTION，在每个动作之前被执行的代码，参考文献[1]31页*/
/*扩充返回类型YYLVAL*/
 
%}
%option yylineno

id   [A-Za-z][A-Za-z0-9]*  
int    [0-9]+
float  ([0-9]*\.[0-9]+)|([0-9]+\.)

%%
{int}        {yylval.type_int=atoi(yytext); return INT;}
{float}      {/*自己补充返回浮点常量的代码，返回类型FLOAT*/}
"int"        {strcpy(yylval.type_id,  yytext);return TYPE;}
"float"      {/*自己补充返回浮点类型的代码，返回类别TYPE*/}
"return"     {return RETURN;}
"if"         {/*Your Own Code*/}
"else"       {/*Your Own Code*/}
"while"      {/*Your Own Code*/}

{id}       {strcpy(yylval.type_id,  yytext); return ID; }
";"			{return SEMI;}
","			{return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext);return RELOP;}
"="			{return ASSIGNOP;}
"+"			{return PLUS;}
"-"			{return MINUS;}
"*"			{return STAR;}
"/"			{return DIV;}
"&&"      {return AND;}
"||"        {return OR;}
"!"			{return NOT;}
"("			{return LP;}
")"			{return RP;}
"{"			{return LC;}
"}"			{return RC;}
[\n]        {/*对位置变量赋值1*/}   
[ \r\t]      {}   
.			{/*输出词法分析错误编号、异常符号串、错误行列号*/}
%%

/* 单独调试词法分析实验时，需要main函数；当与bison联用时，不需要*/
/*
int main()
{
    yylex();
    return 0;
}
*/
int yywrap()
{
    return 1;
}