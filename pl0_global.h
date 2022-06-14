#ifndef PL0_H
#define PL0_H

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//..
#define MAX_ID_LEN 10
#define MAX_PROG_LEN 2048
#define MAX_TOKEN_N 1024

typedef enum {
	NUL_TYPE,	// error

	// Keywords
	K_BEGIN, 	// begin
	K_CALL, 	// call
	K_CONST,	// const
	K_DO,		// do
	K_END,		// end
	K_IF,		// if
	K_ODD,		// odd
	K_PROCEDURE,// procedure
	K_READ,		// read
	K_THEN,		// then
	K_VAR,		// var
	K_WHILE,	// while
	K_WRITE,	// write

	// Operators
	O_PLUS,		// +
	O_MINUS,	// -
	O_MULTI,	// *
	O_DIV,		// /
	O_ASSIGN,	// :=
	O_EQ,		// =
	O_LESS,		// <
	O_LEQ,		// <=
	O_GTR,		// >
	O_GEQ,		// >=
	O_NEQ,		// # (NE, not equal)

	// Delimiters
	D_COMMA,	// ,
	D_SEMICOLON,// ;
	D_PERIOD,	// .
	D_LP,		// (
	D_RP,		// )

	// identifier
	ID,

	// number
	NUMBER,
} SYM;

//.........
#define KEYWORD_N 13

#define KEYCHAR_N 13

#define SPECIAL_N 16

//.........
#define MAX_VAR_PER_TABLE 32
#define MAX_TABLE_N 8
#define MAX_STACK_SIZE 1024
#define MAX_STATE_N 128
#define MAX_TERMS 64
#define MAX_LEV 12
#define MAX_ITEM_N 1024
#define MAX_CODE_N 2048

typedef enum {
	T_CONST,
	T_VARIABLE,
	T_PROCEDURE
} TableTermType;

typedef enum {
	C_ASSIGN,
	C_ODD,
	C_NEG,
	C_PLUS,
	C_MINUS,
	C_MULT,
	C_DIV,
	C_EQ,
	C_LS,
	C_LE,
	C_GT,
	C_GE,
	C_NEQ,
	C_JMP,
	C_J0,
	C_READ,
	C_WRITE,
	C_CALL,
	C_RET
} CODE_OPR;

typedef struct {
	char name[MAX_ID_LEN];
	TableTermType kind;
	int val;
} Var; // used in table

typedef struct {
	union {
		Var* place;
		int val;
	} u;
	enum {VAR, CONST} kind;
} Operand;

typedef struct {
	CODE_OPR instr;
	Operand *s1, *s2, *dst;
} Code;


//.........

void *alloc_assert(int len);

typedef struct {
	SYM sym;
	char id[MAX_ID_LEN];
	int num, row; // row number in original code. start with 1.
} Token;

char prog[MAX_PROG_LEN];

int token_n;

Token tokens[MAX_TOKEN_N];

/*************** lexical part ***************/

const char keychars[KEYCHAR_N];

int icode_lex(FILE *in, FILE *out);

SYM getType(const char* str);
SYM getKeywordType(const char* str);
SYM getSpecialType(const char* str);

int isLetter(char ch);
int isDigit(char ch);

int isSpecial(char ch);

void print_token(FILE*, Token *token);

typedef enum {
    S_START,
    S_STRING,
    S_ID,
    S_NUM,
    S_SPEC1,
    S_SPEC2,
	S_CHECK,
    S_ERROR,
} STATE;

/*************** syntax part ***************/

typedef struct {
	Var *place;
	int val;
} NT; // non terminals

typedef enum {
	STK_NONT,
	STK_T
} StackTermType;

typedef struct {
	union {
		NT * V;
		Token * T;
	} u;

	StackTermType kind;
} Item; // used in the stack

struct Table_{
	Var* variables[MAX_VAR_PER_TABLE];
	char name[MAX_ID_LEN];
	int val_len;
	struct Table_* prev;
};

typedef struct Table_ Table;

// item stack

Item item_stack[MAX_STACK_SIZE];
int item_top;

void stack_push_NT(NT * nt);
void stack_push_T(Token * t);
void stack_pop();
Token* get_T(int);
NT* get_NT(int);

// table

Table* tables_stack[MAX_STACK_SIZE];
int table_top;

int table_n;

Table tables[MAX_TABLE_N];
Table* cur_table;

void table_pop(char* name);
Var* table_enter(char* name, TableTermType type, int val);
void table_make();
Var* table_lookup(char* name);

void table_print_all();

// SLR dealer

int state_stack[MAX_STACK_SIZE];
int state_top;

int state_n, symbols_n;

int map_table[MAX_STATE_N][MAX_TERMS];

SYM action_header[MAX_TERMS];

void read_map_table();

int get_next_action(int state, SYM input_sym);

int cur_state();

void action_shift(int nxt_state);
int action_reduction(int grammar);

int icode_gen(FILE* intercode);

// intercode gen part

Var* new_temp(); // new temperory variable

void gen(CODE_OPR, Var* s1, Var* s2, Var* dst);

void gen2(CODE_OPR, int s1, int s2, Var* dst);

int global_entry;

Code codes[MAX_CODE_N];

int code_n;

void print_codes(FILE* out);

/***grammar**/

#define MAX_GRAMMAR_N 64

int grammar_n;

int terminal_n;

int grammar_length[MAX_GRAMMAR_N];

int grammar_index[MAX_GRAMMAR_N];

NT *(*grammar_action[64])(int *);

#endif