#include "def.h"
struct node *mknode(int kind, struct node *first, struct node *second, struct node *third, int position) 
{
    struct node *T=(struct node *)malloc(sizeof(struct node));
    
    T->kind=kind;
    T->ptr[0]=first;
    T->ptr[1]=second;
    T->ptr[2]=third;
    T->position=position;
    
    return T;
}

//对抽象语法树的先根遍历
void display(struct node *T,int indent)
{
    int i=1;
    struct node *T0;
    if (T)
	{
	    switch (T->kind)
        {
	        case EXT_DEF_LIST:
                display(T->ptr[0], indent);     //显示该外部定义列表中的第一个
                display(T->ptr[1], indent);     //显示外部定义列表中的其它外部定义
                break;

	        case EXT_VAR_DEF:
                //显示外部变量类型
                printf("%*c外部变量定义：\n",indent,' ');
                display(T->ptr[0],indent+5);
                //显示变量列表
                printf("%*c变量名：\n",indent+5,' ');
                display(T->ptr[1],indent+10);
                break;

            case FUNC_DEF:
                printf("%*c函数定义：\n",indent,' ');
                display(T->ptr[0],indent+5);      //显示函数返回类型
                display(T->ptr[1],indent+5);      //显示函数名和参数
                display(T->ptr[2],indent+5);      //显示函数体
                break;

	        case FUNC_DEC:
                printf("%*c函数名：%s\n",indent,' ',T->type_id);
                printf("%*c函数形参：\n",indent,' ');
                display(T->ptr[0],indent+5);  //显示函数参数列表
                break;

            case EXT_DEC_LIST:
                display(T->ptr[0],indent);     //依次显示外部变量名，
                display(T->ptr[1],indent);     //后续还有相同的，仅显示语法树此处理代码可以和类似代码合并
                break;
	        
	        case PARAM_LIST:
                display(T->ptr[0],indent);     //依次显示全部参数类型和名称，
                display(T->ptr[1],indent);
                break;

	        case PARAM_DEC:
                if (T->ptr[0]->type == INT)
                    printf("%*c类型：%s,参数名：%s\n", indent, ' ', "int", T->ptr[1]->type_id);
                else if (T->ptr[0]->type == FLOAT)
                    printf("%*c类型：%s,参数名：%s\n", indent, ' ', "float", T->ptr[1]->type_id);
                else if (T->ptr[0]->type == CHAR)
                    printf("%*c类型：%s,参数名：%s\n", indent, ' ', "char", T->ptr[1]->type_id);
                else
                    printf("%*c类型：%s,参数名：%s\n", indent, ' ', "unknown", T->ptr[1]->type_id);
                break;

            case VAR_DEF:
                printf("%*cLOCAL VAR_NAME：\n",indent,' ');
                display(T->ptr[0],indent+5);   //显示变量类型
                display(T->ptr[1],indent+5);   //显示该定义的全部变量名
                break;

            case DEC_LIST:
                printf("%*cVAR_NAME：\n",indent,' ');
                T0=T;
                while (T0)
                {
                    if (T0->ptr[0]->kind==ID)
                        printf("%*c %s\n",indent+5,' ',T0->ptr[0]->type_id);
                    else if (T0->ptr[0]->kind==ASSIGNOP)
                    {
                        printf("%*c %s ASSIGNOP\n ",indent+5,' ',T0->ptr[0]->ptr[0]->type_id);
                        //显示初始化表达式
                        display(T0->ptr[0]->ptr[1],indent+strlen(T0->ptr[0]->ptr[0]->type_id)+4);       
                    }
                    T0=T0->ptr[1];
                }
                break;

            case DEF_LIST:
                display(T->ptr[0],indent);    //显示该局部变量定义列表中的第一个
                display(T->ptr[1],indent);    //显示其它局部变量定义
                break;
            
            case COMP_STM:
                printf("%*c复合语句：\n",indent,' ');
                printf("%*c复合语句的变量定义：\n",indent+5,' ');
                display(T->ptr[0],indent+10);      //显示定义部分
                printf("%*c复合语句的语句部分：\n",indent+5,' ');
                display(T->ptr[1],indent+10);      //显示语句部分
                break;
            
            case STM_LIST:
                display(T->ptr[0],indent);          //显示第一条语句
                display(T->ptr[1],indent);          //显示剩下语句
                break;

	        case EXP_STMT:
                printf("%*c表达式语句：\n",indent,' ');
                display(T->ptr[0],indent+5);
                break;

            case IF_THEN:
                printf("%*c条件语句(IF_THEN)：\n",indent,' ');
                printf("%*c条件：\n",indent+5,' ');
                display(T->ptr[0],indent+10);      //显示条件
                printf("%*cIF子句：\n",indent+5,' ');
                display(T->ptr[1],indent+10);      //显示if子句
                break;

            case IF_THEN_ELSE:
                printf("%*c条件语句(IF_THEN_ELSE)：\n",indent,' ');
                printf("%*c条件：\n",indent+5,' ');
                display(T->ptr[0],indent+10);      //显示条件
                printf("%*cIF子句：\n",indent+5,' ');
                display(T->ptr[1],indent+10);      //显示if子句
                printf("%*cELSE子句：\n",indent+5,' ');
                display(T->ptr[2],indent+10);      //显示else子句
                break;

            case FUNC_CALL:
                printf("%*c函数调用：\n",indent,' ');
                printf("%*c函数名：%s\n",indent+5,' ',T->type_id);
                display(T->ptr[0],indent+5);
                break;

	        case ARGS:
                i=1;
                while (T)   /*ARGS表示实际参数表达式序列结点，其第一棵子树为头部实际参数表达式，第二棵子树为剩下的实际参数。*/
                {  
                    struct node *T0=T->ptr[0];
                    printf("%*c第%d个实际参数表达式：\n",indent,' ',i++);
                    display(T0,indent+5);
                    T=T->ptr[1];
                }
                //printf("%*c第%d个实际参数表达式：\n",indent,' ',i);
                //displayAST(T,indent+5);
                printf("\n");
                break;

	        case RETURN:
                printf("%*c返回语句：\n",indent,' ');
                display(T->ptr[0],indent+5);
                break;

            case WHILE:
                printf("%*cwhile循环语句：\n",indent,' ');
                printf("%*cwhile循环条件：\n",indent+5,' ');
                display(T->ptr[0],indent+10);      //显示循环条件
                printf("%*cwhile循环体：\n",indent+5,' ');
                display(T->ptr[1],indent+10);      //显示循环体
                break;

            case FOR:
                printf("%*cfor循环语句：\n",indent,' ');
                printf("%*cfor循环条件：\n",indent+5,' ');
                display(T->ptr[0],indent+10);      //显示循环条件
                printf("%*cfor循环体：\n",indent+5,' ');
                display(T->ptr[1],indent+10);      //显示循环体
                break;

            case ID:
                printf("%*cID： %s\n",indent,' ',T->type_id);
                break;
           
            case INT:
                printf("%*cINT：%d\n",indent,' ',T->type_int);
                break;

            case FLOAT:
                printf("%*cFLAOT：%f\n",indent,' ',T->type_float);
                break;

            case CHAR:
                printf("%*cCHAR：%c\n",indent,' ',T->type_char);
                break;
            
            //FIXME
            case TYPE:
                if(T->type==INT)
                    printf("%*c类型: int\n",indent,' ');
                else if(T->type==FLOAT)
                    printf("%*c类型: float\n",indent,' ');
                else if(T->type==CHAR)
                    printf("%*c类型: char\n",indent,' ');
                else if(T->type==ARRAY)
                    printf("%*c类型: char[]\n",indent,' ');
                break; 

            case ASSIGNOP:
            case AND:
            case OR:
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
                printf("%*c%s\n",indent,' ',T->type_id);
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                break;
            
            case NOT:
            case UMINUS:    
                printf("%*c%s\n",indent,' ',T->type_id);
                display(T->ptr[0],indent+5);
                break;
        }
    }
}
