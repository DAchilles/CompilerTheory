src = lex.l parser.y def.h ast.c
temp = parser.tab.c parser.tab.h lex.yy.c

func_test : $(src)
	flex lex.l
	bison -d -v parser.y
	gcc ast.c parser.tab.c lex.yy.c -lfl -o func_test

.PHONY : clean
clean : 
	-rm func_test $(temp)