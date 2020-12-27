%{
#include "parser.tab.h"     /*需要与bison联动后加上*/
#include "string.h"
#include "def.h"            /*自定义数据结构、函数头文件*/
int yycolumn=1;          
/*TODO:完成语法单元定位的宏YY_USER_ACTION，在每个动作之前被执行的代码，参考文献[1]31页*/
/*扩充返回类型YYLVAL*/
#define YY_USER_ACTION yylloc.first_line=yylloc.last_line=yylineno; yylloc.first_column=yycolumn; yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;
typedef union{
    int type_int;
    float type_float;
    char type_char;
    char type_id[32];
    struct node *ptr;
}YYLVAL;
#define YYSTYPE YYLVAL
%}

%option yylineno
id      [A-Za-z][A-Za-z0-9]*  
int     [0-9]+
float   ([0-9]*\.[0-9]+)|([0-9]+\.)

%%
\/\/[^\n]*  {;}
\/\*(\s|.)*?\*\/    {;}
{int}       {yylval.type_int=atoi(yytext); return INT;}
{float}     {yylval.type_float=atof(yytext); return FLOAT;}
"int"       {strcpy(yylval.type_id,  yytext);return TYPE;}
"float"     {strcpy(yylval.type_id,  yytext);return TYPE;}
"return"    {return RETURN;}
"if"        {strcpy(yylval.type_id, yytext);return IF;}
"else"      {strcpy(yylval.type_id, yytext);return ELSE;}
"while"     {strcpy(yylval.type_id, yytext);return WHILE;}
{id}        {strcpy(yylval.type_id,  yytext); return ID; }
";"			{return SEMI;}
","			{return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext);return RELOP;}
"="			{return ASSIGNOP;}
"+"			{return PLUS;}
"-"			{return MINUS;}
"*"			{return STAR;}
"/"			{return DIV;}
"&&"        {return AND;}
"||"        {return OR;}
"!"			{return NOT;}
"("			{return LP;}
")"			{return RP;}
"{"			{return LC;}
"}"			{return RC;}
[\n]        {yylineno=1;}   
[ \r\t]     {;}   
.			{printf("ERROR at line%d: %s\n", yylineno, yytext);}
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