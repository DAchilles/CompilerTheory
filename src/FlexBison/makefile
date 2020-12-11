src = ./src/fb1_5.l ./src/fb1_5.y

fb1_5 : $(src)
	bison -d ./src/fb1_5.y
	flex ./src/fb1_5.l
	cc -o fb1_5 fb1_5.tab.c lex.yy.c -lfl

.PHONY : clean
clean : 
	-rm fb1_5 fb1_5.tab.c fb1_5.tab.h lex.yy.c 
