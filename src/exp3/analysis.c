#include "def.h"

int LEV = 0;   //层号
int func_size; //函数的活动记录大小
struct SymbolTable symbol_table;
struct SymbolScopeArray symbol_scope;

void semantic_analysis_init(struct node *T)
{
    symbol_table.index = 0;
    /*
    fill_symbol_table("read", "", 0, INT, 'F', 4);
    symbol_table.symbols[0].paramnum = 0; //read的形参个数
    fill_symbol_table("x", "", 1, INT, 'P', 12);
    fill_symbol_table("write", "", 0, INT, 'F', 4);
    symbol_table.symbols[2].paramnum = 1;
    */
    //初始化数据
    symbol_scope.ScopeArray[0] = 0; //外部变量在符号表中的起始序号为0
    symbol_scope.top = 1;
    T->offset = 0; // 外部变量在数据区的偏移量
    semantic_analysis(T);
	
    print_IR(T->code);//打印这棵树
}

void semantic_analysis(struct node* T)
{
    int rtn, num, width;
    struct operandStruct opn1, opn2, result;
    struct node *T0;
    if(T)
    {
        switch (T->kind)
        {
            case EXT_DEF_LIST:
                if (!T->ptr[0])
                    return;
                // 语义分析之前先设置偏移地址
                T->ptr[0]->offset = T->offset;
                //访问外部定义列表中的第一个
                semantic_analysis(T->ptr[0]);
                // 之后合并code
                T->code = T->ptr[0]->code;
                // 可为空
                if (T->ptr[1])
                {
                    T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
                    semantic_analysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
                    T->code = merge(2, T->code, T->ptr[1]->code);
                }
                break;

	        case EXT_VAR_DEF:
                if (!strcmp(T->ptr[0]->type_id, "int"))
                {
                    T->type = T->ptr[1]->type = INT;
                    T->ptr[1]->width = 4;
                }
                if (!strcmp(T->ptr[0]->type_id, "float"))
                {
                    T->type = T->ptr[1]->type = FLOAT;
                    T->ptr[1]->width = 8;
                }
                if (!strcmp(T->ptr[0]->type_id, "char"))
                {
                    T->type = T->ptr[1]->type = CHAR;
                    T->ptr[1]->width = 1;
                }
                //这个外部变量的偏移量向下传递
                T->ptr[1]->offset = T->offset; 
                //处理外部变量说明中的标识符序列
                ext_var_list(T->ptr[1]);
                //计算这个外部变量说明的宽度
                T->width = (T->ptr[1]->width) * T->ptr[1]->num;
                T->code = NULL;   
                break;

            case FUNC_DEF:
                if (!strcmp(T->ptr[0]->type_id, "int"))
                    T->ptr[1]->type = INT;
                else if (!strcmp(T->ptr[0]->type_id, "float"))
                    T->ptr[1]->type = FLOAT;
                else if (!strcmp(T->ptr[0]->type_id, "char"))
                    T->ptr[1]->type = CHAR;
                
                //函数的宽度设置为0，不会对外部变量的地址分配产生影响
                T->width = 0;
                //设置局部变量在活动记录中的偏移量初值
                T->offset = DX;
                //处理函数名和参数结点部分
                semantic_analysis(T->ptr[1]);
                //用形参单元宽度修改函数局部变量的起始偏移量
                T->offset += T->ptr[1]->width;
                T->ptr[2]->offset = T->offset;
                //函数体语句执行结束后的位置属性
                strcpy(T->ptr[2]->Snext, new_label());
                //处理函数体结点
                semantic_analysis(T->ptr[2]);
                //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
                symbol_table.symbols[T->ptr[1]->place].offset = T->offset + T->ptr[2]->width;
                T->code = merge(3, T->ptr[1]->code, T->ptr[2]->code, genLabel(T->ptr[2]->Snext)); //函数体的代码作为函数的代码
                break;

	        case FUNC_DEC:
                rtn = fill_symbol_table(T->type_id, new_alias(), LEV, T->type, 'F', 0); //函数不在数据区中分配单元，偏移量为0
                if (rtn == -1)
                {
                    semantic_error(T->position, T->type_id, "函数名重复使用，可能是函数重复定义，语义错误");
                    return;
                }
                else
                    T->place = rtn;
                
                result.kind = ID;
                strcpy(result.id, T->type_id);
                result.offset = rtn;
                
                T->code = genIR(FUNCTION, opn1, opn2, result); //生成中间代码：FUNCTION 函数名
                T->offset = DX;                                //设置形式参数在活动记录中的偏移量初值
                if (T->ptr[0])
                { //判断是否有参数
                    T->ptr[0]->offset = T->offset;
                    semantic_analysis(T->ptr[0]); //处理函数参数列表
                    T->width = T->ptr[0]->width;
                    symbol_table.symbols[rtn].paramnum = T->ptr[0]->num;
                    T->code = merge(2, T->code, T->ptr[0]->code); //连接函数名和参数代码序列
                }
                else
                    symbol_table.symbols[rtn].paramnum = 0, T->width = 0;
                break;

            //FIXME:delete
            case EXT_DEC_LIST:
                break;
	        
	        case PARAM_LIST:
                T->ptr[0]->offset = T->offset;
                semantic_analysis(T->ptr[0]);
                if (T->ptr[1])
                {
                    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                    semantic_analysis(T->ptr[1]);
                    T->num = T->ptr[0]->num + T->ptr[1]->num;             //统计参数个数
                    T->width = T->ptr[0]->width + T->ptr[1]->width;       //累加参数单元宽度
                    T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code); //连接参数代码
                }
                else
                {
                    T->num = T->ptr[0]->num;
                    T->width = T->ptr[0]->width;
                    T->code = T->ptr[0]->code;
                }
                break;

	        case PARAM_DEC:
                rtn = fill_symbol_table(T->ptr[1]->type_id, new_alias(), 1, T->ptr[0]->type, 'P', T->offset);
                if (rtn == -1)
                    semantic_error(T->ptr[1]->position, T->ptr[1]->type_id, "参数名重复定义,语义错误");
                else
                    T->ptr[1]->place = rtn;
                T->num = 1;                                //参数个数计算的初始值
                T->width = T->ptr[0]->type == INT ? 4 : 8; //参数宽度
                result.kind = ID;
                strcpy(result.id, symbol_table.symbols[rtn].alias);
                result.offset = T->offset;
                T->code = genIR(PARAM, opn1, opn2, result); //生成：FUNCTION 函数名
                break;

            case VAR_DEF:
                T->code = NULL;
                if (!strcmp(T->ptr[0]->type_id, "int"))
                {
                    T->ptr[1]->type = INT;
                    width = 4;
                }
                if (!strcmp(T->ptr[0]->type_id, "float"))
                {
                    T->ptr[1]->type = FLOAT;
                    width = 8;
                }
                if (!strcmp(T->ptr[0]->type_id, "char"))
                {
                    T->ptr[1]->type = CHAR;
                    width = 1;
                }
                
                //T0为变量名列表子树根指针，对ID、ASSIGNOP类结点在登记到符号表，作为局部变量
                T0 = T->ptr[1]; 
                num = 0;
                T0->offset = T->offset;
                T->width = 0;

                //处理所有DEC_LIST结点
                while (T0)
                {
                    num++;
                    T0->ptr[0]->type = T0->type; //类型属性向下传递
                    if (T0->ptr[1])
                        T0->ptr[1]->type = T0->type;

                    T0->ptr[0]->offset = T0->offset; //类型属性向下传递
                    if (T0->ptr[1])
                        T0->ptr[1]->offset = T0->offset + width;
                    if (T0->ptr[0]->kind == ID)
                    {
                        rtn = fill_symbol_table(T0->ptr[0]->type_id, new_alias(), LEV, T0->ptr[0]->type, 'V', T->offset + T->width); //此处偏移量未计算，暂时为0
                        if (rtn == -1)
                            semantic_error(T0->ptr[0]->position, T0->ptr[0]->type_id, "变量重复定义");
                        else
                            T0->ptr[0]->place = rtn;
                        T->width += width;
                    }
                    else if (T0->ptr[0]->kind == ASSIGNOP)
                    {
                        rtn = fill_symbol_table(T0->ptr[0]->ptr[0]->type_id, new_alias(), LEV, T0->ptr[0]->type, 'V', T->offset + T->width); //此处偏移量未计算，暂时为0
                        if (rtn == -1)
                            semantic_error(T0->ptr[0]->ptr[0]->position, T0->ptr[0]->ptr[0]->type_id, "变量重复定义");
                        else
                        {
                            T0->ptr[0]->place = rtn;
                            T0->ptr[0]->ptr[1]->offset = T->offset + T->width + width;
                            Exp(T0->ptr[0]->ptr[1]);
                            opn1.kind = ID;
                            strcpy(opn1.id, symbol_table.symbols[T0->ptr[0]->ptr[1]->place].alias);
                            result.kind = ID;
                            strcpy(result.id, symbol_table.symbols[T0->ptr[0]->place].alias);
                            T->code = merge(3, T->code, T0->ptr[0]->ptr[1]->code, genIR(ASSIGNOP, opn1, opn2, result));
                        }
                        T->width += width + T0->ptr[0]->ptr[1]->width;
                    }
                    T0 = T0->ptr[1];
                }
                break;

            //FIXME:delete
            case DEC_LIST:
                break;
                
            case DEF_LIST:
                T->code = NULL;
                if (T->ptr[0])
                {
                    T->ptr[0]->offset = T->offset;
                    semantic_analysis(T->ptr[0]); //处理一个局部变量定义
                    T->code = T->ptr[0]->code;
                    T->width = T->ptr[0]->width;
                }
                if (T->ptr[1])
                {
                    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                    semantic_analysis(T->ptr[1]); //处理剩下的局部变量定义
                    T->code = merge(2, T->code, T->ptr[1]->code);
                    T->width += T->ptr[1]->width;
                }
                break;
            
            case COMP_STM:
                LEV++;
                //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbol_scope_Stack
                //新作用域
                symbol_scope.ScopeArray[symbol_scope.top++] = symbol_table.index;
                T->width = 0;
                T->code = NULL;
                if (T->ptr[0])//DefList
                {
                    T->ptr[0]->offset = T->offset;
                    semantic_analysis(T->ptr[0]); //处理该层的局部变量DEF_LIST
                    T->width += T->ptr[0]->width;
                    T->code = T->ptr[0]->code;
                }
                if (T->ptr[1])//StmList
                {
                    T->ptr[1]->offset = T->offset + T->width;
                    strcpy(T->ptr[1]->Snext, T->Snext); //S.next属性向下传递
                    semantic_analysis(T->ptr[1]);       //处理复合语句的语句序列
                    T->width += T->ptr[1]->width;
                    T->code = merge(2, T->code, T->ptr[1]->code);
                }

                LEV--;     //出复合语句，层号减1
                symbol_table.index = symbol_scope.ScopeArray[--symbol_scope.top]; //删除该作用域中的符号
                break;
            
            case STM_LIST:
                if (!T->ptr[0])
                {
                    T->code = NULL;
                    T->width = 0;
                    return;
                }
                //2条以上语句连接，生成新标号作为第一条语句结束后到达的位置
                if (T->ptr[1])
                    strcpy(T->ptr[0]->Snext, new_label());
                //语句序列仅有一条语句，S.next属性向下传递
                else
                    strcpy(T->ptr[0]->Snext, T->Snext);
                T->ptr[0]->offset = T->offset;
                semantic_analysis(T->ptr[0]);
                T->code = T->ptr[0]->code;
                T->width = T->ptr[0]->width;
                //2条以上语句连接,S.next属性向下传递
                if (T->ptr[1])
                {
                    strcpy(T->ptr[1]->Snext, T->Snext);
                    T->ptr[1]->offset = T->offset; //顺序结构共享单元方式
                    semantic_analysis(T->ptr[1]);
                    
                    //序列中第1条为表达式语句，返回语句，复合语句时，第2条前不需要标号
                    if (T->ptr[0]->kind == RETURN || T->ptr[0]->kind == EXP_STMT || T->ptr[0]->kind == COMP_STM)
                        T->code = merge(2, T->code, T->ptr[1]->code);
                    else
                        T->code = merge(3, T->code, genLabel(T->ptr[0]->Snext), T->ptr[1]->code);
                    
                    if (T->ptr[1]->width > T->width)
                        T->width = T->ptr[1]->width; //顺序结构共享单元方式
                }
                break;

	        case EXP_STMT:
                T->ptr[0]->offset = T->offset;
                semantic_analysis(T->ptr[0]);
                T->code = T->ptr[0]->code;
                T->width = T->ptr[0]->width;
                break;

            case IF_THEN:
                strcpy(T->ptr[0]->Etrue, new_label()); //设置条件语句真假转移位置
                strcpy(T->ptr[0]->Efalse, T->Snext);
                T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
                bool_exp(T->ptr[0]);
                T->width = T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext, T->Snext);
                semantic_analysis(T->ptr[1]); //if子句
                if (T->width < T->ptr[1]->width)
                    T->width = T->ptr[1]->width;
                T->code = merge(3, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue), T->ptr[1]->code);
                break;

            case IF_THEN_ELSE:
                //设置条件语句真假转移位置
                strcpy(T->ptr[0]->Etrue, new_label());
                strcpy(T->ptr[0]->Efalse, new_label());
                T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;
                
                //条件，要单独按短路代码处理
                bool_exp(T->ptr[0]);
                T->width = T->ptr[0]->width;
                
                strcpy(T->ptr[1]->Snext, T->Snext);
                semantic_analysis(T->ptr[1]); //if子句
                if (T->width < T->ptr[1]->width)
                    T->width = T->ptr[1]->width;
                
                strcpy(T->ptr[2]->Snext, T->Snext);
                semantic_analysis(T->ptr[2]); //else子句
                if (T->width < T->ptr[2]->width)
                    T->width = T->ptr[2]->width;
                
                T->code = merge(6, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue), T->ptr[1]->code, genGoto(T->Snext), genLabel(T->ptr[0]->Efalse), T->ptr[2]->code);
                break;
            
            //TODO
	        case RETURN:
                if (T->ptr[0])
                {
                    T->ptr[0]->offset = T->offset;
                    Exp(T->ptr[0]);
                    num = symbol_table.index;
                    do
                        num--;
                    while (symbol_table.symbols[num].flag != 'F');
                    if (T->ptr[0]->type != symbol_table.symbols[num].type)
                    {
                        semantic_error(T->position, "返回值类型错误。", "");
                        T->width = 0;
                        T->code = NULL;
                        return;
                    }
                    T->width = T->ptr[0]->width;
                    result.kind = ID;
                    strcpy(result.id, symbol_table.symbols[T->ptr[0]->place].alias);
                    result.offset = symbol_table.symbols[T->ptr[0]->place].offset;
                    T->code = merge(2, T->ptr[0]->code, genIR(RETURN, opn1, opn2, result));
                }
                else
                {
                    T->width = 0;
                    result.kind = 0;
                    T->code = genIR(RETURN, opn1, opn2, result);
                }
                break;

            //TODO
            case WHILE:
                break;

            //FIXME
            case ID:
            case INT:
            case FLOAT:
            case CHAR:
            case ASSIGNOP:
            case AND:
            case OR:
            case RELOP:
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
            case NOT:
            case UMINUS:
            case FUNC_CALL:
                Exp(T); //处理基本表达式
                break;
        }
    }
}

void Exp(struct node *T)
{
	struct operandStruct opn1, opn2, result;
	int op;
	int rtn, width;
	struct node *T0;

	if (T)
	{
		switch (T->kind)
		{
		case ID:
			rtn = search_symbol_table(T->type_id);//先查找
			if (rtn == -1)
				semantic_error(T->position, T->type_id, "变量未定义。");
			if (symbol_table.symbols[rtn].flag == 'F')
				semantic_error(T->position, T->type_id, "是函数，非普通变量。");
			else//查找成功则为此结点赋值
			{
				T->place = rtn; //结点保存变量在符号表中的位置
				T->code = NULL; //标识符不需要生成TAC
				T->type = symbol_table.symbols[rtn].type;
				T->offset = symbol_table.symbols[rtn].offset;
				T->width = 0; //未再使用新单元
			}
			break;

		case INT:
			T->place = temp_add(new_temp(), LEV, T->type, 'T', T->offset); //为整常量生成一个临时变量
			T->type = INT;

			opn1.kind = INT;
			opn1.const_int = T->type_int;

			result.kind = ID;
			strcpy(result.id, symbol_table.symbols[T->place].alias);
			result.offset = symbol_table.symbols[T->place].offset;

			T->code = genIR(ASSIGNOP, opn1, opn2, result);//生成临时变量储存立即数
			T->width = 4;
			break;

		case ASSIGNOP:
			if (T->ptr[0]->kind != ID)
			{
				semantic_error(T->position, "", "赋值号左边必须为变量。");
			}
			else
			{
				Exp(T->ptr[0]); //处理左值，例中仅为变量
				T->ptr[1]->offset = T->offset;
				Exp(T->ptr[1]);
				T->type = T->ptr[0]->type;
				T->width = T->ptr[1]->width;
				T->code = merge(2, T->ptr[0]->code, T->ptr[1]->code);

				opn1.kind = ID;
				strcpy(opn1.id, symbol_table.symbols[T->ptr[1]->place].alias); //右值一定是个变量或临时变量（立即数
				opn1.offset = symbol_table.symbols[T->ptr[1]->place].offset;

				result.kind = ID;
				strcpy(result.id, symbol_table.symbols[T->ptr[0]->place].alias);
				result.offset = symbol_table.symbols[T->ptr[0]->place].offset;

				T->code = merge(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
			}
			break;

		case AND:
		case OR:
		case RELOP:
			T->type = INT;//关系运算结果为整形
			T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
			Exp(T->ptr[0]);
			Exp(T->ptr[1]);
			break;

		case PLUS:
		case MINUS:
		case STAR:
		case DIV:
			T->ptr[0]->offset = T->offset;
			Exp(T->ptr[0]);
			T->ptr[1]->offset = T->offset + T->ptr[0]->width;
			Exp(T->ptr[1]);
			//判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
			//下面的类型属性计算，没有考虑错误处理情况
			T->type = INT, T->width = T->ptr[0]->width + T->ptr[1]->width + 2;
			T->place = temp_add(new_temp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);

			opn1.kind = ID;
			strcpy(opn1.id, symbol_table.symbols[T->ptr[0]->place].alias);
			opn1.type = T->ptr[0]->type;
			opn1.offset = symbol_table.symbols[T->ptr[0]->place].offset;

			opn2.kind = ID;
			strcpy(opn2.id, symbol_table.symbols[T->ptr[1]->place].alias);
			opn2.type = T->ptr[1]->type;
			opn2.offset = symbol_table.symbols[T->ptr[1]->place].offset;

			result.kind = ID;
			strcpy(result.id, symbol_table.symbols[T->place].alias);
			result.type = T->type;
			result.offset = symbol_table.symbols[T->place].offset;

			T->code = merge(3, T->ptr[0]->code, T->ptr[1]->code, genIR(T->kind, opn1, opn2, result));
			T->width = T->ptr[0]->width + T->ptr[1]->width + 4;//INT
			break;

		case NOT:
		case UMINUS:
			T->type = INT;
			T->ptr[0]->offset = T->offset;
			Exp(T->ptr[0]);
			break;

		case FUNC_CALL: //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
			rtn = search_symbol_table(T->type_id);
			if (rtn == -1)
			{
				semantic_error(T->position, T->type_id, "函数未定义。");
				return;
			}
			if (symbol_table.symbols[rtn].flag != 'F')
			{
				semantic_error(T->position, T->type_id, "是函数，非普通变量。");
				return;
			}
			T->type = symbol_table.symbols[rtn].type;
			width = T->type == INT ? 4 : 8; //存放函数返回值的单数字节数

			if (T->ptr[0])//有实参
			{
				T->ptr[0]->offset = T->offset;
				Exp(T->ptr[0]);                      //处理所有实参表达式求值，及类型
				T->width = T->ptr[0]->width + width; //累加上计算实参使用临时变量的单元数
				T->code = T->ptr[0]->code;
			}
			else
			{
				T->width = width;
				T->code = NULL;
			}
			match_param(rtn, T->ptr[0]); //处理所以参数的匹配
				//处理参数列表的中间代码

			T0 = T->ptr[0];
			while (T0)
			{
				result.kind = ID;
				strcpy(result.id, symbol_table.symbols[T0->ptr[0]->place].alias);
				result.offset = symbol_table.symbols[T0->ptr[0]->place].offset;
				T->code = merge(2, T->code, genIR(ARG, opn1, opn2, result));
				T0 = T0->ptr[1];
			}
			T->place = temp_add(new_temp(), LEV, T->type, 'T', T->offset + T->width - width);

			opn1.kind = ID;
			strcpy(opn1.id, T->type_id); //保存函数名
			opn1.offset = rtn;           //这里offset用以保存函数定义入口,在目标代码生成时，能获取相应信息

			result.kind = ID;
			strcpy(result.id, symbol_table.symbols[T->place].alias);
			result.offset = symbol_table.symbols[T->place].offset;

			T->code = merge(2, T->code, genIR(CALL, opn1, opn2, result)); //生成函数调用中间代码
			break;

		case ARGS: //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
			T->ptr[0]->offset = T->offset;
			Exp(T->ptr[0]);
			T->width = T->ptr[0]->width;
			T->code = T->ptr[0]->code;
			if (T->ptr[1])
			{
				T->ptr[1]->offset = T->offset + T->ptr[0]->width;
				Exp(T->ptr[1]);
				T->width += T->ptr[1]->width;
				T->code = merge(2, T->code, T->ptr[1]->code);
			}
			break;
		}
	}
}

void ext_var_list(struct node *T)
{
    int rtn, num = 1;
    switch (T->kind)
    {
    case EXT_DEC_LIST:
        T->ptr[0]->type = T->type;                //将类型属性向下传递变量结点
        T->ptr[0]->offset = T->offset;            //外部变量的偏移量向下传递
        T->ptr[1]->type = T->type;                //将类型属性向下传递变量结点
        T->ptr[1]->offset = T->offset + T->width; //外部变量的偏移量向下传递
        T->ptr[1]->width = T->width;
        ext_var_list(T->ptr[0]);
        ext_var_list(T->ptr[1]);
        T->num = T->ptr[1]->num + 1;
        break;
    case ID:
        rtn = fill_symbol_table(T->type_id, new_alias(), LEV, T->type, 'V', T->offset); //最后一个变量名
        if (rtn == -1)
            semantic_error(T->position, T->type_id, "变量重复定义，语义错误");
        else
            T->place = rtn;
        T->num = 1;
        break;
    default:
        break;
    }
}

void semantic_error(int line, char *msg1, char *msg2)
{
    printf("[Error] Line %d: %s %s\n", line, msg1, msg2);
}

int search_symbol_table(char *name)
{
    int i;
    for (i = symbol_table.index - 1; i >= 0; i--)
        if (!strcmp(symbol_table.symbols[i].name, name))
            return i;
    return -1;
}

int fill_symbol_table(char *name, char *alias, int level, int type, char flag, int offset)
{
    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for (i = symbol_table.index - 1; symbol_table.symbols[i].level == level || (level == 0 && i >= 0); i--)
    {
        if (level == 0 && symbol_table.symbols[i].level == 1)
            continue; //外部变量和形参不必比较重名
        if (!strcmp(symbol_table.symbols[i].name, name))
            return -1;
    }
    //填写符号表内容
    strcpy(symbol_table.symbols[symbol_table.index].name, name);
    strcpy(symbol_table.symbols[symbol_table.index].alias, alias);
    symbol_table.symbols[symbol_table.index].level = level;
    symbol_table.symbols[symbol_table.index].type = type;
    symbol_table.symbols[symbol_table.index].flag = flag;
    symbol_table.symbols[symbol_table.index].offset = offset;
    return symbol_table.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

int temp_add(char *name, int level, int type, char flag, int offset)
{
    strcpy(symbol_table.symbols[symbol_table.index].name, "");
    strcpy(symbol_table.symbols[symbol_table.index].alias, name);
    symbol_table.symbols[symbol_table.index].level = level;
    symbol_table.symbols[symbol_table.index].type = type;
    symbol_table.symbols[symbol_table.index].flag = flag;
    symbol_table.symbols[symbol_table.index].offset = offset;
    return symbol_table.index++; //返回的是临时变量在符号表中的位置序号
}

int match_param(int i, struct node *T)
{
    int j, num=symbol_table.symbols[i].paramnum;
    int type1, type2;
    if(T == NULL)
    {
        if(num == 0) return 1;
        else semantic_error(T->position, T->type_id,"函数调用参数个数过少。");
    }
    for (j = 1; j < num; j++)
    {
        type1 = symbol_table.symbols[i + j].type; //形参类型
        type2 = T->ptr[0]->type;
        if (type1 != type2)
        {
            semantic_error(T->position, T->type_id, "函数调用参数类型与定义不匹配。");
            return 0;
        }
        T = T->ptr[1];
    }
    if (T->ptr[1])
    { //num个参数已经匹配完，还有实参表达式
        semantic_error(T->position, T->type_id,"函数调用参数个数过多。");
        return 0;
    }
    return 1;
}

void bool_exp(struct node *T)
{
    struct operandStruct opn1, opn2, result;
    int op;
    int rtn;
    if (T)
    {
        switch (T->kind)
        {
            case INT:
                if (T->type_int != 0)
                    T->code = genGoto(T->Etrue);
                else
                    T->code = genGoto(T->Efalse);
                T->width = 0;
                break;
            case FLOAT:
                if (T->type_float != 0.0)
                    T->code = genGoto(T->Etrue);
                else
                    T->code = genGoto(T->Efalse);
                T->width = 0;
                break;
            case ID: //查符号表，获得符号表中的位置，类型送type
                rtn = search_symbol_table(T->type_id);
                if (rtn == -1)
                    semantic_error(T->position, T->type_id, "函数未定义，语义错误");
                if (symbol_table.symbols[rtn].flag == 'F')
                    semantic_error(T->position, T->type_id, "不是函数名，不能进行函数调用，语义错误");
                else
                {
                    opn1.kind = ID;
                    strcpy(opn1.id, symbol_table.symbols[rtn].alias);
                    opn1.offset = symbol_table.symbols[rtn].offset;
                    opn2.kind = INT;
                    opn2.const_int = 0;
                    result.kind = ID;
                    strcpy(result.id, T->Etrue);
                    T->code = genIR(NEQ, opn1, opn2, result);
                    T->code = merge(2, T->code, genGoto(T->Efalse));
                }
                T->width = 0;
                break;
            case RELOP: //处理关系运算表达式,2个操作数都按基本表达式处理
                T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
                Exp(T->ptr[0]);
                T->width = T->ptr[0]->width;
                Exp(T->ptr[1]);
                if (T->width < T->ptr[1]->width)
                    T->width = T->ptr[1]->width;
                opn1.kind = ID;
                strcpy(opn1.id, symbol_table.symbols[T->ptr[0]->place].alias);
                opn1.offset = symbol_table.symbols[T->ptr[0]->place].offset;
                opn2.kind = ID;
                strcpy(opn2.id, symbol_table.symbols[T->ptr[1]->place].alias);
                opn2.offset = symbol_table.symbols[T->ptr[1]->place].offset;
                result.kind = ID;
                strcpy(result.id, T->Etrue);
                if (strcmp(T->type_id, "<") == 0)
                    op = JLT;
                else if (strcmp(T->type_id, "<=") == 0)
                    op = JLE;
                else if (strcmp(T->type_id, ">") == 0)
                    op = JGT;
                else if (strcmp(T->type_id, ">=") == 0)
                    op = JGE;
                else if (strcmp(T->type_id, "==") == 0)
                    op = EQ;
                else if (strcmp(T->type_id, "!=") == 0)
                    op = NEQ;
                T->code = genIR(op, opn1, opn2, result);
                T->code = merge(4, T->ptr[0]->code, T->ptr[1]->code, T->code, genGoto(T->Efalse));
                break;
            case AND:
            case OR:
                if (T->kind == AND)
                {
                    strcpy(T->ptr[0]->Etrue, new_label());
                    strcpy(T->ptr[0]->Efalse, T->Efalse);
                }
                else
                {
                    strcpy(T->ptr[0]->Etrue, T->Etrue);
                    strcpy(T->ptr[0]->Efalse, new_label());
                }
                strcpy(T->ptr[1]->Etrue, T->Etrue);
                strcpy(T->ptr[1]->Efalse, T->Efalse);
                T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
                bool_exp(T->ptr[0]);
                T->width = T->ptr[0]->width;
                bool_exp(T->ptr[1]);
                if (T->width < T->ptr[1]->width)
                    T->width = T->ptr[1]->width;
                if (T->kind == AND)
                    T->code = merge(3, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue), T->ptr[1]->code);
                else
                    T->code = merge(3, T->ptr[0]->code, genLabel(T->ptr[0]->Efalse), T->ptr[1]->code);
                break;
            case NOT:
                strcpy(T->ptr[0]->Etrue, T->Efalse);
                strcpy(T->ptr[0]->Efalse, T->Etrue);
                bool_exp(T->ptr[0]);
                T->code = T->ptr[0]->code;
                break;
            default:
                break;
        }
    }
}

char *str_catch(char *s1, char *s2)
{
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *new_alias()
{
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    // itoa(no++, s, 10);
    return str_catch("var", s);
}

char *new_label()
{
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    // itoa(no++, s, 10);
    return str_catch("label", s);
}

char *new_temp()
{
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    return str_catch("temp", s);
}

struct codenode *genIR(int op, struct operandStruct opn1, struct operandStruct opn2, struct operandStruct result)
{
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = op;
    h->opn1 = opn1;
    h->opn2 = opn2;
    h->result = result;
    h->next = h->prior = h;
    return h;
}

struct codenode *genLabel(char *label)
{
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = LABEL;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

struct codenode *genGoto(char *label)
{
    struct codenode *h = (struct codenode *)malloc(sizeof(struct codenode));
    h->op = GOTO;
    strcpy(h->result.id, label);
    h->next = h->prior = h;
    return h;
}

struct codenode *merge(int num, ...)
{
    struct codenode *h1, *h2, *t1, *t2;
    va_list ap;//指向参数的指针
    va_start(ap, num);//宏初始化va_list变量，使其指向第一个可变参数的地址
    h1 = va_arg(ap, struct codenode *);//返回可变参数，va_arg的第二个参数是要返回的参数的类型,如果多个可变参数，依次调用va_arg获取各个参数
    while (--num > 0)
    {
        h2 = va_arg(ap, struct codenode *);
        if (h1 == NULL)
            h1 = h2;
        else if (h2)
        {
            t1 = h1->prior;
            t2 = h2->prior;
            t1->next = h2;
            t2->next = h1;
            h1->prior = t2;
            h2->prior = t1;
        }
    }
    va_end(ap);//使用va_end宏结束可变参数的获取
    return h1;
}

void print_IR(struct codenode *head)
{
    char opnstr1[32], opnstr2[32], resultstr[32];
    struct codenode *h = head;
    do
    {
        if (h->opn1.kind == INT)
            sprintf(opnstr1, "#%d", h->opn1.const_int);
        if (h->opn1.kind == FLOAT)
            sprintf(opnstr1, "#%f", h->opn1.const_float);
        if (h->opn1.kind == ID)
            sprintf(opnstr1, "%s", h->opn1.id);
        if (h->opn2.kind == INT)
            sprintf(opnstr2, "#%d", h->opn2.const_int);
        if (h->opn2.kind == FLOAT)
            sprintf(opnstr2, "#%f", h->opn2.const_float);
        if (h->opn2.kind == ID)
            sprintf(opnstr2, "%s", h->opn2.id);
        sprintf(resultstr, "%s", h->result.id);
        switch (h->op)
        {
        case ASSIGNOP:
            printf("  %s := %s\n", resultstr, opnstr1);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            printf("  %s := %s %c %s\n", resultstr, opnstr1,
                   h->op == PLUS ? '+' : h->op == MINUS ? '-' : h->op == STAR ? '*' : '\\', opnstr2);
            break;
        case FUNCTION:
            printf("\nFUNCTION %s :\n", h->result.id);
            break;
        case PARAM:
            printf("  PARAM %s\n", h->result.id);
            break;
        case LABEL:
            printf("LABEL %s :\n", h->result.id);
            break;
        case GOTO:
            printf("  GOTO %s\n", h->result.id);
            break;
        case JLE:
            printf("  IF %s <= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JLT:
            printf("  IF %s < %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGE:
            printf("  IF %s >= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGT:
            printf("  IF %s > %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case EQ:
            printf("  IF %s == %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case NEQ:
            printf("  IF %s != %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case ARG:
            printf("  ARG %s\n", h->result.id);
            break;
        case CALL:
            printf("  %s := CALL %s\n", resultstr, opnstr1);
            break;
        case RETURN:
            if (h->result.kind)
                printf("  RETURN %s\n", resultstr);
            else
                printf("  RETURN\n");
            break;
        }
        h = h->next;
    } while (h != head);
}

/*
static int h_num=1;
void print_IR(struct codenode *head)
{
    char opnstr1[32], opnstr2[32], resultstr[32];
    struct codenode *h = head;
    do
    {
		if(h->op!=FUNCTION) printf("%d\t",h_num++);
        if (h->opn1.kind == INT)//立即数前要加“#”
            sprintf(opnstr1, "#%d", h->opn1.const_int);
        else if (h->opn1.kind == FLOAT)
            sprintf(opnstr1, "#%f", h->opn1.const_float);
        else if (h->opn1.kind == CHAR)
            sprintf(opnstr1, "#%c", h->opn1.const_char);
        else if (h->opn1.kind == ID)
            sprintf(opnstr1, "%s", h->opn1.id);

        if (h->opn2.kind == INT)
            sprintf(opnstr2, "#%d", h->opn2.const_int);
        else if (h->opn2.kind == FLOAT)
            sprintf(opnstr2, "#%f", h->opn2.const_float);
        else if (h->opn2.kind == CHAR)
            sprintf(opnstr2, "#%c", h->opn2.const_char);
        else if (h->opn2.kind == ID)
            sprintf(opnstr2, "%s", h->opn2.id);

        sprintf(resultstr, "%s", h->result.id);//仅变量可作右值

        switch (h->op) {
        case ASSIGNOP:
            printf("  %s := %s\n", resultstr, opnstr1);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            printf("  %s := %s %c %s\n", resultstr, opnstr1,
                   h->op == PLUS ? '+' : h->op == MINUS ? '-' : h->op == STAR ? '*' : '\\', opnstr2);
            break;
        case FUNCTION:
            printf("\n%d\tFUNCTION %s :\n", h_num++,h->result.id);
            break;
        case PARAM:
            printf("  PARAM %s\n", h->result.id);
            break;
        case LABEL:
            printf("LABEL %s :\n", h->result.id);
            break;
        case GOTO:
            printf("  GOTO %s\n", h->result.id);
            break;
        case JLE:
            printf("  IF %s <= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JLT:
            printf("  IF %s < %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGE:
            printf("  IF %s >= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case JGT:
            printf("  IF %s > %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case EQ:
            printf("  IF %s == %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case NEQ:
            printf("  IF %s != %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
        case ARG:
            printf("  ARG %s\n", h->result.id);
            break;
        case CALL:
            printf("  %s := CALL %s\n", resultstr, opnstr1);
            break;
        case RETURN:
            if (h->result.kind)
                printf("  RETURN %s\n", resultstr);
            else
                printf("  RETURN\n");
            break;
        }
        h = h->next;
    } while (h != head);
}
*/









