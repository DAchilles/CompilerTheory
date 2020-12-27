#include "def.h"

struct SymbolTable symbol_table;
struct SymbolScopeArray symbol_scope;

int i=0, j=0, cnt=0;

void display_symbol()
{
    int it;
    printf("\t\tSymbol Table\n");
    printf("---------------------------------------------------\n");
    printf("%s\t%s\t%s\t%s\t%s\t%s\n","Index","Name","Level","Type","Flag","Param_num");
    printf("---------------------------------------------------\n");
    for(it=0; it<symbol_table.index; it++)
    {
        printf("%d\t", it);
        printf("%s\t", symbol_table.symbols[it].name);
        printf("%d\t", symbol_table.symbols[it].level);
        
        if(symbol_table.symbols[it].type==INT)
            printf("int\t");
        else if(symbol_table.symbols[it].type==FLOAT)
            printf("float\t");
        else
            printf("char\t");
        
        printf("%c\t", symbol_table.symbols[it].flag);
        
        if(symbol_table.symbols[it].flag=='F')
            printf("%d\n", symbol_table.symbols[it].paramnum);
        else
            printf("\n");
    }
    printf("---------------------------------------------------\n");
    printf("\n");
}

int semantic_analysis(struct node* T,int type,int level,char flag,int command)
{
    int type_1, type_2;
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
                semantic_analysis(T->ptr[1], type, 1, flag, command);
                semantic_analysis(T->ptr[2], type, 1, flag, command);
                break;

	        case FUNC_DEC:
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
                symbol_table.index = symbol_scope.ScopeArray[symbol_scope.top-1];
                symbol_scope.top--;
                if(symbol_scope.top == 0)
                    display_symbol();
                break;
            
            case STM_LIST:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

	        case EXP_STMT:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                break;

            case IF_THEN:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            case IF_THEN_ELSE:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                semantic_analysis(T->ptr[2], type, level, flag, command);
                break;

            //FIXME
            case FUNC_CALL:
                j=0;
                while(symbol_table.symbols[j].level==0 && j<symbol_table.index)
                {
                    if(!strcmp(symbol_table.symbols[j].name, T->type_id))
                    {
                        if(symbol_table.symbols[j].flag != 'F')
                            printf("ERROR! Line %d: Func %s has been define as a variable\n", T->position, T->type_id);
                        break;
                    }
                    j++;
                }
                if(symbol_table.symbols[j].level==1 || j==symbol_table.index)
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
                semantic_analysis(T->ptr[0], type, level, flag, command);
                break;

            case WHILE:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            case FOR:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                semantic_analysis(T->ptr[1], type, level, flag, command);
                break;

            //FIXME
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
                }
                else
                {
                    i = symbol_table.index-1;
                    while(i>=0)
                    {
                        if(strcmp(symbol_table.symbols[i].name,T->type_id)==0 && (symbol_table.symbols[i].flag=='V' || symbol_table.symbols[i].flag=='T'))
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
            case AND:
            case OR:
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
            case COMADD:
            case COMSUB:
            case COMSTAR:
            case COMDIV:
                type_1 = semantic_analysis(T->ptr[0], type, level, flag, command);
                type_2 = semantic_analysis(T->ptr[1], type, level, flag, command);
                if(type_1 == type_2)
                    return type_1;
                break;
            case AUTOADD_L:
            case AUTOSUB_L:
            case AUTOADD_R:
            case AUTOSUB_R:
            case NOT:
            case UMINUS:
                semantic_analysis(T->ptr[0], type, level, flag, command);
                break;
        }
    }
}