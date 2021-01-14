#include "def.h"

struct SymbolTable symbol_table;
struct SymbolScopeArray symbol_scope;

void display_symbol()
{
    int i;
    printf("\t\tSymbol Table\n");
    printf("---------------------------------------------------\n");
    printf("%s\t%s\t%s\t%s\t%s\t%s\n","Index","Name","Level","Type","Flag","Param_num");
    printf("---------------------------------------------------\n");
    for(i=0; i<symbol_table.index; i++)
    {
        printf("%d\t", i);
        printf("%s\t", symbol_table.symbols[i].name);
        printf("%d\t", symbol_table.symbols[i].level);
        
        if(symbol_table.symbols[i].type==INT)
            printf("int\t");
        else if(symbol_table.symbols[i].type==FLOAT)
            printf("float\t");
        else
            printf("char\t");
        
        printf("%c\t", symbol_table.symbols[i].flag);
        
        if(symbol_table.symbols[i].flag=='F')
            printf("%d\n", symbol_table.symbols[i].paramnum);
        else
            printf("\n");
    }
    printf("---------------------------------------------------\n");
    printf("\n");
}

int j=0, cnt=0, p=0;
int semantic_analysis(struct node* T,int type,int level,char flag,int command)
{
    int type_1=0, type_2=0, i=0;
    if(T)
    {
        switch (T->kind)
        {
            case EXT_DEF_LIST:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

	        case EXT_VAR_DEF:
                type = semantic_analysis(T->ptr[0],type,level,flag,command);
                semantic_analysis(T->ptr[1],type,level,flag,command);
                break;

            case FUNC_DEF:
                type = semantic_analysis(T->ptr[0], type, level+1, flag, command);
                flag='F';
                semantic_analysis(T->ptr[1], type, 1, flag, command);
                flag='T';
                semantic_analysis(T->ptr[2], type, 1, flag, command);
                break;

	        case FUNC_DEC:
                i=0;//从栈底搜起
                while(symbol_table.symbols[i].level!=level && i<symbol_table.index) i++;//定位到表内此时的开作用域   
                while(i<symbol_table.index)
                {
                    if(strcmp(symbol_table.symbols[i].name,T->type_id)==0 && symbol_table.symbols[i].flag==flag)
                    {
                        printf("ERROR! Line %d: Multiple definition of %s\n", T->position, T->type_id);
                        return 0;
                    }
                    i++;
                }
                strcpy(symbol_table.symbols[symbol_table.index].name, T->type_id);
                symbol_table.symbols[symbol_table.index].level = 0;
                symbol_table.symbols[symbol_table.index].type = type;
                symbol_table.symbols[symbol_table.index].flag = 'F';
                symbol_table.index++;
                cnt = 0;
                semantic_analysis(T->ptr[0], type, level, flag, command);
                symbol_table.symbols[symbol_table.index - cnt - 1].paramnum = cnt;
                break;

            case EXT_DEC_LIST:
                flag = 'V';
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;
	        
	        case PARAM_LIST:
                cnt++;
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

	        case PARAM_DEC:
                flag = 'P';
                type = semantic_analysis(T->ptr[0], type, level+1, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            case ARRAY_DEF:
                type = semantic_analysis(T->ptr[0],type,level,flag,command);//TYPE
                semantic_analysis(T->ptr[1],type,level,flag,0);//ARRAY_DEC
                break;
            
            case ARRAY_DEC://搜索是否重名
                i=0;
                while(symbol_table.symbols[i].level!=level && i<symbol_table.index) i++;//定位到表内此时的开作用域   
                flag = 'A';
                if(command==0)
                {
                    while(i<symbol_table.index)
                    {
                        if(strcmp(symbol_table.symbols[i].name,T->type_id)==0 && symbol_table.symbols[i].flag!=flag)
                        {
                            printf("ERROR! Line %d: Multiple definition of %s\n",T->position,T->type_id);
                            return 0;
                        }
                        i++;
                    }
                    strcpy(symbol_table.symbols[symbol_table.index].name,T->type_id);
                    symbol_table.symbols[symbol_table.index].level=level;
                    symbol_table.symbols[symbol_table.index].type=type;//数组类型
                    symbol_table.symbols[symbol_table.index].flag=flag;
                    symbol_table.index++;
                }
                else
                {
                    i=symbol_table.index-1;//从上向下，选择作用域最近的变量调用
                    while(i>=0)
                    {
                        if(strcmp(symbol_table.symbols[i].name,T->type_id)==0)//限定变量
                        {
                            if(symbol_table.symbols[i].flag=='A')
                            {
                                if(T->ptr[0]->kind!=INT)
                                {
                                    printf("ERROR! Line %d: Array subscript is not an integer\n",T->position);
                                    return 0;
                                }
                                else
                                    return symbol_table.symbols[i].type;//返回此名称的类型
                            }
                            else 
                            {
                                printf("Error! Line %d: %s is not a array\n",T->position,T->type_id);
                                break;
                            }
                        }
                        i--;
                    }
                    if(i<0)
                    {
                        printf("ERROR: Line%d: Undefined variable %s\n",T->position,T->type_id);
                    }
                }
                break;

            case VAR_DEF:
                type = semantic_analysis(T->ptr[0], type, level+1, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            //FIXME:
            case DEC_LIST:
                /*
                printf("%*cVAR_NAME：\n",indent,' ');
                T0=T;
                while (T0)
                {
                    if (T0->ptr[0]->kind==ID)
                        printf("%*c %s\n",indent+3,' ',T0->ptr[0]->type_id);
                    else if (T0->ptr[0]->kind==ASSIGNOP)
                    {
                        printf("%*c %s ASSIGNOP\n ",indent+3,' ',T0->ptr[0]->ptr[0]->type_id);
                        //显示初始化表达式
                        display(T0->ptr[0]->ptr[1],indent+strlen(T0->ptr[0]->ptr[0]->type_id)+4);       
                    }
                    T0=T0->ptr[1];
                }
                break;
                */
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;
                
            case DEF_LIST:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;
            
            case COMP_STM:
                flag = 'T';
                command = 0;
                symbol_scope.ScopeArray[symbol_scope.top] = symbol_table.index;
                symbol_scope.top++;
                semantic_analysis(T->ptr[0], type, level, flag, command);
                
                command = 1;
                semantic_analysis(T->ptr[1], type, level+1, flag, command);
                
                if(symbol_scope.ScopeArray[symbol_scope.top-1] != symbol_table.index)
                    display_symbol();

                symbol_table.index = symbol_scope.ScopeArray[symbol_scope.top-1];
                symbol_scope.top--;

                break;
            
            case STM_LIST:
                type_1=semantic_analysis(T->ptr[0],type,level,flag,command);//Stmt
                if(T->ptr[0]->kind==RETURN)
                {
                    p=symbol_table.index;
                    while(p>=0)//从下向上寻找函数ID
                    {
                        if(symbol_table.symbols[p].flag=='F')
                        {
                            type_2=symbol_table.symbols[p].type;
                            break;
                        }
                        p--;
                    }
                    if(type_1!=type_2)
                        printf("ERROR! Line%d: Function return value type mismatch\n",T->position);
                }
                semantic_analysis(T->ptr[1],type,level,flag,command);//STM_LIST
                break;

	        case EXP_STMT:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                break;

            case WHILE:
            case IF_THEN:
                type_1 = semantic_analysis(T->ptr[0], type, level, flag, command);
                if(type_1 != INT)
                    printf("ERROR! Line %d: Only integer variables can be used as conditions for judgment statements\n",T->position);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            case IF_THEN_ELSE:
                type_1 = semantic_analysis(T->ptr[0], type, level, flag, command);
                if(type_1 != INT)
                    printf("ERROR! Line %d: Only integer variables can be used as conditions for judgment statements\n",T->position);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                semantic_analysis(T->ptr[2], type, level, flag, command);
                break;

            //FIXME
            case FUNC_CALL:
                j=0;
                //while(symbol_table.symbols[j].level==0 && j<symbol_table.index)
                while(j<symbol_table.index)
                {
                    if(!strcmp(symbol_table.symbols[j].name, T->type_id))
                    {
                        if(symbol_table.symbols[j].flag != 'F')
                            printf("ERROR! Line %d: Func %s has been define as a variable\n", T->position, T->type_id);
                        break;
                    }
                    j++;
                }
                //if(symbol_table.symbols[j].level==1 || j==symbol_table.index)
                if(j==symbol_table.index)
                {
                    printf("ERROR! Line %d: Func %s undefined\n", T->position, T->type_id);
                    break;
                }
                type = symbol_table.symbols[j+1].type;
                cnt = 0;
                semantic_analysis(T->ptr[0], type, level, flag, command);
                if(symbol_table.symbols[j].paramnum != cnt)
                    printf("ERROR! Line %d: Func %s mismatch in number of args\n", T->position, T->type_id);
                break;
                /*
                printf("%*c函数调用：\n",indent,' ');
                printf("%*c函数名：%s\n",indent+3,' ',T->type_id);
                display(T->ptr[0],indent+3);
                break;
                */

            //FIXME
	        case ARGS:
                cnt++;
                if(type != semantic_analysis(T->ptr[0], type, level, flag, command))
                    printf("ERROR! Line %d: Func mismatch in type of args at No.%d\n", T->position, cnt);
                type = symbol_table.symbols[j + cnt + 1].type;
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

	        case RETURN:
                type_1 =  semantic_analysis(T->ptr[0], type, level, flag, command);
                return type_1;

            case ID:
                //转到相同作用域
                i = 0;
                while(symbol_table.symbols[i].level!=level && i<symbol_table.index)
                    i++;
                if(command == 0)
                {
                    while(i < symbol_table.index)
                    {
                        if(strcmp(symbol_table.symbols[i].name, T->type_id)==0 && symbol_table.symbols[i].flag==flag)
                        {
                            printf("ERROR! Line %d: Multiple definition of %s\n", T->position, T->type_id);
                            return 0;
                        }
                        i++;
                    }
                    strcpy(symbol_table.symbols[symbol_table.index].name, T->type_id);
                    symbol_table.symbols[symbol_table.index].level = level;
                    symbol_table.symbols[symbol_table.index].type = type;
                    symbol_table.symbols[symbol_table.index].flag = flag;
                    symbol_table.index++;
                    return type;
                }
                else
                {
                    i = symbol_table.index-1;
                    while(i>=0)
                    {
                        if(strcmp(symbol_table.symbols[i].name,T->type_id)==0 && symbol_table.symbols[i].flag!='F')
                            return symbol_table.symbols[i].type;
                        i--;
                    }
                    if(i<0)
                        printf("ERROR! Line %d: Undefined variable %s\n", T->position, T->type_id);
                }
                break;
           
            case INT:
                return INT;

            case FLOAT:
                return FLOAT;

            case CHAR:
                return CHAR;
            
            case TYPE:
                return T->type;

            case ASSIGNOP:
                if(T->ptr[0]->kind==INT||T->ptr[0]->kind==FLOAT||T->ptr[0]->kind==CHAR)
                {
                    printf("ERROR! Line %d: lvalue required as left operand of assignment\n",T->position);
                    return 0;
                }

            case AND:
            case OR:
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
                type_1 = semantic_analysis(T->ptr[0], type, level, flag, command);
                type_2 = semantic_analysis(T->ptr[1], type, level, flag, command);
                if(type_1 == type_2)
                {
                    return type_1;
                }
                else
                {
                    if(type_1 && type_2)
                        printf("ERROR! Line %d: Variable types on both sides of the operator are not the same\n", T->position);
                    return 0;
                }
                
                break;
                
            case NOT:
            case UMINUS:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                break;
        }
    }
}