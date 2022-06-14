SOURCE_CODE := prog.pas

grammar:  icode_lex.o icode_gen.o actions.o grammar.o
	cc -g -o grammar grammar.o icode_lex.o icode_gen.o actions.o

grammar.o: grammar.c
	cc -g -c grammar.c

icode_lex.o: icode_lex.c pl0_global.h
	cc -g -c icode_lex.c

icode_gen.o: icode_gen.c pl0_global.h   table.grammar actions.o
	cc -g -c icode_gen.c

actions.o: actions.c pl0_global.h
	cc -g -c actions.c

table.grammar: LR_table_gen/grammar
	python3 LR_table_gen/LR_table.py "LR_table_gen/grammar" > table.grammar

all: grammar

clean:
	rm -f grammar *.o _*
