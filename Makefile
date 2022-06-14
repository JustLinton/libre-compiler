SOURCE_CODE := prog.pas

main:  icode_lex.o icode_gen.o actions.o main.o
	cc -g -o main main.o icode_lex.o icode_gen.o actions.o

main.o: main.c
	cc -g -c main.c

icode_lex.o: icode_lex.c pl0_global.h
	cc -g -c icode_lex.c

icode_gen.o: icode_gen.c pl0_global.h   table.grammar actions.o
	cc -g -c icode_gen.c

actions.o: actions.c pl0_global.h
	cc -g -c actions.c

table.grammar: LR_table_gen/grammar
	python3 LR_table_gen/LR_table.py "LR_table_gen/grammar" > table.grammar

all: main

clean:
	rm -f main *.o _*
