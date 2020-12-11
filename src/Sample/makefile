src = lex.l parser.y def.h ast.c
temp = parser.tab.c parser.tab.h lex.yy.c

func_test : $(src)
	bison -d parser.y
	flex lex.l
	gcc -o func_test parser.tab.c ast.c lex.yy.c -lfl

.PHONY : clean
clean : 
	-rm func_test $(temp)