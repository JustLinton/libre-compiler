#include "pl0_global.h"

const char keychars[] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};

const char *SYM_table[] = {
	[NUL_TYPE] "", // null

	// Keywords
	[K_BEGIN] "begin",
	[K_CALL] "call",
	[K_CONST] "const",
	[K_DO] "do",
	[K_END] "end",
	[K_IF] "if",
	[K_ODD] "odd",
	[K_PROCEDURE] "procedure",
	[K_READ] "read",
	[K_THEN] "then",
	[K_VAR] "var",
	[K_WHILE] "while",
	[K_WRITE] "write",

	// Operators
	[O_PLUS] "+",
	[O_MINUS] "-",
	[O_MULTI] "*",
	[O_DIV] "/",
	[O_ASSIGN] ":=",
	[O_EQ] "=",
	[O_LESS] "<",
	[O_LEQ] "<=",
	[O_GTR] ">",
	[O_GEQ] ">=",
	[O_NEQ] "#",

	// Delimiters
	[D_COMMA] ",",
	[D_SEMICOLON] ";",
	[D_PERIOD] ".",
	[D_LP] "(",
	[D_RP] ")",
};

const char *SYM_name[] = {
	[NUL_TYPE] "NUL_TYPE", // null

	// Keywords
	[K_BEGIN] "K_BEGIN",
	[K_CALL] "K_CALL",
	[K_CONST] "K_CONST",
	[K_DO] "K_DO",
	[K_END] "K_END",
	[K_IF] "K_IF",
	[K_ODD] "K_ODD",
	[K_PROCEDURE] "K_PROCEDURE",
	[K_READ] "K_READ",
	[K_THEN] "K_THEN",
	[K_VAR] "K_VAR",
	[K_WHILE] "K_WHILE",
	[K_WRITE] "K_WRITE",

	// Operators
	[O_PLUS] "O_PLUS",
	[O_MINUS] "O_MINUS",
	[O_MULTI] "O_MULTI",
	[O_DIV] "O_DIV",
	[O_ASSIGN] "O_ASSIGN",
	[O_EQ] "O_EQ",
	[O_LESS] "O_LESS",
	[O_LEQ] "O_LEQ",
	[O_GTR] "O_GTR",
	[O_GEQ] "O_GEQ",
	[O_NEQ] "O_NEQ",

	// Delimiters
	[D_COMMA] "D_COMMA",
	[D_SEMICOLON] "D_SEMICOLON",
	[D_PERIOD] "D_PERIOD",
	[D_LP] "D_LP",
	[D_RP] "D_RP",
};

SYM getType(const char *str)
{
	int i;
	for (i = K_BEGIN; i <= D_RP; i++)
	{
		if (strcmp(str, SYM_table[i]) == 0)
		{
			return (SYM)i;
		}
	}

	return NUL_TYPE;
}

SYM getKeywordType(const char *str)
{
	int i;
	for (i = K_BEGIN; i <= K_WRITE; i++)
	{
		if (strcmp(str, SYM_table[i]) == 0)
		{
			return (SYM)i;
		}
	}

	return NUL_TYPE;
}

SYM getSpecialType(const char *str)
{
	int i;
	for (i = O_PLUS; i <= D_RP; i++)
	{
		if (strcmp(str, SYM_table[i]) == 0)
		{
			return (SYM)i;
		}
	}

	return NUL_TYPE;
}

int isLetter(char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int isDigit(char ch)
{
	return ch >= '0' && ch <= '9';
}

int isSpecial(char ch)
{
	if (ch == ' ' || ch == 0)
		return 0;

	int i;
	for (i = 0; i < KEYCHAR_N; i++)
	{
		if (keychars[i] == ch)
			return 1;
	}
	return -1;
}

void print_token(FILE *out, Token *token)
{
	SYM symbol = token->sym;

	if (symbol == ID)
	{
		fprintf(out, "ID             %-19s%3d\n", token->id, token->row);
	}
	else if (symbol == NUMBER)
	{
		fprintf(out, "NUM            %-5d%17d\n", token->num, token->row);
	}
	else
	{
		fprintf(out, "%-15s%-15s%7d\n", SYM_name[token->sym], SYM_table[token->sym], token->row);
	}
}


int row[MAX_PROG_LEN];

char lookAhead(int i) {
	if (!prog[i + 1] || prog[i + 1] == ' ') return 0;

	return prog[i + 1];
}

STATE next_state(STATE cur_state, char last_read_ch, char nxt_ch) {
	int letter = isLetter(nxt_ch), digit = isDigit(nxt_ch), special = isSpecial(nxt_ch);

	if (!letter && !digit && !special && nxt_ch) return S_ERROR;

	if (!nxt_ch) return S_CHECK;
	if (nxt_ch == -1) return S_ERROR;

	switch (cur_state)
	{
	case S_START:
		if (letter) return S_STRING;
		if (digit) return S_NUM;
		if (special) return S_SPEC1;

		break;
	case S_STRING:
		if (letter) return S_STRING;
		if (digit) return S_ID;
		if (special) return S_CHECK;

		break;
	case S_ID:
		if (letter) return S_ID;
		if (digit) return S_ID;
		if (special) return S_CHECK;

		break;
	case S_NUM:
		if (letter) return S_ERROR;
		if (digit) return S_NUM;
		if (special) return S_CHECK;
	
		break;
	case S_SPEC1:
		if (letter) return S_CHECK;
		if (digit) return S_CHECK;
		if (special) {
			if (nxt_ch == '=') {
				if (last_read_ch == ':' || last_read_ch == '>' || last_read_ch == '<') return S_SPEC2;
				else return S_CHECK;
			} else {
				return S_CHECK;
			}
		}
		break;	
	case S_SPEC2:
		return S_CHECK;
		break;
	case S_CHECK:
		return S_START;

	default:
		printf("[lexical]: unknown status\n");

		exit(-2);
	}

	return S_ERROR;
}

int getSYM(int len) {
	token_n = 0;

	int i = 0, str_len = 0, num = 0, start_i = 0;

	char ch, str[MAX_ID_LEN];

	STATE status = S_START, last_status;

	while(i < len) {
		ch = prog[i];

		if (ch ==' ') {
			i++;
			continue;
		}

		if (status != S_CHECK) last_status = status;

		switch (status)
		{
		case S_START:
			start_i = i;
			status = next_state(status, ch, ch);

			memset(str, 0, sizeof(str));
			str_len = 0;
			num = 0;

			break;
		
		case S_STRING:
			str[str_len ++] = ch;
			
			status = next_state(status, ch, lookAhead(i));

			if (status != S_CHECK) i++;

			break;

		case S_ID:
			str[str_len ++] = ch;

			status = next_state(status, ch, lookAhead(i));

			if (status != S_CHECK) i++;

			break;

		case S_NUM:
			num = num * 10 + ch - '0';

			status = next_state(status, ch, lookAhead(i));

			if (status != S_CHECK) i++;

			break;

		case S_SPEC1:
			status = next_state(status, ch, lookAhead(i));

			str[str_len ++] = ch;
			if (status != S_CHECK) i++;

			break;

		case S_SPEC2:
			status = next_state(status, ch, lookAhead(i));
			str[str_len ++] = ch;

			if (status != S_CHECK) i++;

			break;

		case S_ERROR:
			// fprintf(err, "[lexical]: analysis error unknown %s. [location]: %d \n", str, i);

			return -1;

		case S_CHECK:
			tokens[token_n].row = row[i];
			if (last_status == S_ID) {
				tokens[token_n].sym = ID;
				memcpy(tokens[token_n].id, str, str_len);
				token_n ++;
			} else if (last_status == S_STRING) {
				SYM type = getKeywordType(str);

				if (type) {
					tokens[token_n].sym = type;
					memcpy(tokens[token_n].id, str, str_len);
					token_n ++;
				} else {
					tokens[token_n].sym = ID;
					memcpy(tokens[token_n].id, str, str_len);
					token_n ++;
				}
			} else if (last_status == S_NUM) {
				tokens[token_n].sym = NUMBER;
				tokens[token_n].num = num;
				token_n ++;
			} else if (last_status == S_SPEC1 || last_status == S_SPEC2) {
				SYM type = getSpecialType(str);

				if (type) {
					tokens[token_n].sym = type;
					token_n ++;
				} else {
					status = S_ERROR;

					break;
				}
			}
			status = S_START;
			i ++;
		}
	}

	return 0;
}

int icode_lex(FILE *in, FILE *out) {
	char ch;

	int len = 0, cur_row = 1, i;

	if (in) while((ch = fgetc(in)) != EOF ) {
		if (ch == '\r' || ch == '\n' || ch == '\t') prog[len++] = ' ';
		else prog[len ++] = ch;
		if (ch == '\n') cur_row ++;
		row[len - 1] = cur_row;
	}
	prog[len++] = ' ';

	fprintf(out, "\n=======\nafter preprocessing\n%s\n", prog);
	fputc('\n', out);

	if (getSYM(len)) {
		printf("error when lex.\n");
		return -1;
	}

	fprintf(out, "SYM name       SYM value            SYM row\n");

	for (i = 0; i < token_n; i++) print_token(out, &tokens[i]);

	return 0;
}

