#include "pl0_global.h"


const char *code_name[] = {
    [C_ASSIGN] ":=",
    [C_ODD] "odd",
    [C_NEG] "@",
    [C_PLUS] "+",
    [C_MINUS] "-",
    [C_MULT] "*",
    [C_DIV] "/",
    [C_GT] ">",
    [C_GE] ">=",
    [C_LS] "<",
    [C_LE] "<=",
    [C_EQ] "=",
    [C_NEQ] "#",
    [C_JMP] "j",
    [C_J0] "j0",
    [C_READ] "read",
    [C_WRITE] "write",
    [C_CALL] "call",
    [C_RET] "ret",
};

// item stack

void stack_push_NT(NT *nt)
{
    Item *i = &item_stack[++item_top];
    i->kind = STK_NONT;
    i->u.V = nt;
}
void stack_push_T(Token *t)
{
    Item *i = &item_stack[++item_top];
    i->kind = STK_T;
    i->u.T = t;
}
void stack_pop()
{
    item_top--;
}

Token *get_T(int i)
{
    return item_stack[item_top - i].u.T;
}

NT *get_NT(int i)
{
    return item_stack[item_top - i].u.V;
}

// table

void table_pop(char *name)
{
    strcpy(tables_stack[table_top]->name, name);
    table_top--;
}
Var *table_enter(char *name, TableTermType type, int val)
{
    Table *cur_tbl = tables_stack[table_top];
    Var *v = alloc_assert(sizeof *v);

    strcpy(v->name, name);
    v->kind = type;
    v->val = val;
    cur_tbl->variables[cur_tbl->val_len++] = v;

    return v;
}

void table_make(char *name)
{
    Table *tbl = &tables[table_n++];
    strcpy(tbl->name, name);
    tbl->prev = tables_stack[table_top];
    cur_table = tbl;

    tables_stack[++table_top] = tbl;
}
Var *table_lookup(char *name)
{
    Table *tbl;
    for (tbl = cur_table; tbl; tbl = tbl->prev)
    {
        int i;
        for (i = 0; i < tbl->val_len; i++)
        {
            if (strcmp(tbl->variables[i]->name, name) == 0)
            {
                return tbl->variables[i];
            }
        }
    }
    return 0;
}

// void table_print_all(FILE *out)
// {
//     int i, j;
//     fprintf(out, "%10s\t%5s\t%5s\n================================\n", "name", "type", "val");
//     for (i = 0; i < table_n; i++)
//     {
//         fprintf(out, "[%s]\n", tables[i].name);
//         for (j = 0; j < tables[i].val_len; j++)
//         {
//             Var *v = tables[i].variables[j];

//             if (v->kind == T_PROCEDURE)
//             {
//                 fprintf(out, "%10s\t%5s\t%5d\n", v->name, "proc", v->val);
//             }
//             else if (v->kind == T_CONST)
//             {
//                 fprintf(out, "%10s\t%5s\t%5d\n", v->name, "const", v->val);
//             }
//             else
//             {
//                 fprintf(out, "%10s\t%5s\t%5d\n", v->name, "var", v->val);
//             }
//         }
//         fprintf(out, "--------------------------------\n");
//     }
// }

// SLR dealer

void read_map_table()
{
    FILE *in = fopen("table.grammar", "r");

    // table info

    fscanf(in, "%d%d", &state_n, &symbols_n);

    int i, j;
    for (i = 0; i < state_n; i++)
    {
        for (j = 0; j < symbols_n; j++)
        {
            fscanf(in, "%d", &map_table[i][j]);
        }
    }

    // grammar length

    fscanf(in, "%d", &grammar_n);
    for (i = 0; i < grammar_n; i++)
    {
        fscanf(in, "%d", &grammar_length[i]);
    }

    // symbols indexed by table

    fscanf(in, "%d", &terminal_n);
    char t[16];

    for (i = 0; i < terminal_n; i++)
    {
        fscanf(in, "%s", t);
        if (strcmp(t, "$") == 0)
        {
            action_header[i] = D_PERIOD;
        }
        else if (strcmp(t, "id") == 0)
        {
            action_header[i] = ID;
        }
        else if (strcmp(t, "num") == 0)
        {
            action_header[i] = NUMBER;
        }
        else
        {
            action_header[i] = getType(t);
        }
    }

    // grammar LHS index in table goto header

    for (i = 0; i < grammar_n; i++)
    {
        fscanf(in, "%d", &grammar_index[i]);
    }

    fclose(in);
}

int get_next_action(int state, SYM input_sym)
{
    int i;
    for (i = 0; i < terminal_n; i++)
    {
        if (input_sym == action_header[i])
        {
            return map_table[state][i];
        }

        if (input_sym == O_MINUS && action_header[i] == O_PLUS)
        {
            return map_table[state][i];
        }

        if (input_sym == O_DIV && action_header[i] == O_MULTI)
        {
            return map_table[state][i];
        }
    }

    return 0;
}

int cur_state()
{
    return state_stack[state_top];
}

void action_shift(int nxt_state)
{
    state_stack[++state_top] = nxt_state;
}

int action_reduction(int grammar)
{
    state_top -= grammar_length[grammar];

    if (grammar_index[grammar] < 0)
    {
        puts("[action_reduction]: grammar_index < 0!!!");
        exit(-1);
    }

    action_shift(map_table[cur_state()][grammar_index[grammar] + terminal_n]);

    NT *nt = NULL;

    int status = 0;

    if (grammar_action[grammar])
        nt = grammar_action[grammar](&status);

    item_top -= grammar_length[grammar];

    stack_push_NT(nt);

    return status;
}

// intercode gen part

Var *new_temp()
{
    static int cur_tmp_cnt;
    static char cur_tmp_name[MAX_ID_LEN];

    strcpy(cur_tmp_name, "T_");
    char tmp[10];
    sprintf(tmp, "%d", cur_tmp_cnt++);
    strcat(cur_tmp_name, tmp);
    return table_enter(cur_tmp_name, T_VARIABLE, 0);
}

Operand *get_operand_var(Var *o)
{
    if (o)
    {
        Operand *t = alloc_assert(sizeof *t);
        t->kind = VAR;
        t->u.place = o;
        return t;
    }
    return 0;
}
Operand *get_operand_int(int o)
{
    Operand *t = alloc_assert(sizeof *t);
    t->kind = CONST;
    t->u.val = o;
    return t;
}

void gen_(CODE_OPR opr, Operand *s1, Operand *s2, Operand *dst)
{
    codes[code_n].instr = opr;
    codes[code_n].s1 = s1;
    codes[code_n].s2 = s2;
    codes[code_n].dst = dst;

    code_n++;
}

void gen(CODE_OPR opr, Var *s1, Var *s2, Var *dst)
{
    gen_(opr, get_operand_var(s1), get_operand_var(s2), get_operand_var(dst));
}

void gen2(CODE_OPR opr, int s1, int s2, Var *dst)
{
    gen_(opr, get_operand_int(s1), 0, get_operand_var(dst));
}

void print_operand(Operand *o, FILE *out)
{
    if (o)
    {
        if (o->kind == CONST)
        {
            fprintf(out, "%10d ", o->u.val);
        }
        else
        {
            fprintf(out, "%10s ", o->u.place->name);
        }
    }
    else
    {
        fprintf(out, "%10s ", " ");
    }
}

void print_codes(FILE *out)
{
    int i;
    for (i = 0; i < code_n; i++)
    {
        fprintf(out, "%2d | %5s ", i+100, code_name[codes[i].instr]);
        print_operand(codes[i].s1, out);
        print_operand(codes[i].s2, out);
        print_operand(codes[i].dst, out);
        fputs("\n", out);
    }
}

// void error_print(int i, const char* errmsg, FILE* err) {
//     puts("error!!!");

//     fprintf(err, "                SYM name       SYM value           SYM row\n");

//     fprintf(err, "previous token: ");
//     print_token(err, &tokens[i - 2]);
//     fprintf(err, "----error---->: ");
//     print_token(err, &tokens[i - 1]);
//     fprintf(err, "follow   token: ");
//     print_token(err, &tokens[i]);
//     fputc('\n', err);

//     fprintf(err, "%s on %d th token in %d th row\n", errmsg, i, tokens[i - 1].row);
// }

int icode_gen(FILE* intercode) {
    read_map_table();
    
    state_top = 0;
    code_n = 0;

    int i = 0;
    while(i < token_n) {
        int nxt_action = get_next_action(cur_state(), tokens[i].sym);

        // if (!nxt_action) {
        //     error_print(i, "syntax error ", err);

        //     return -1;
        // }

        if (nxt_action > state_n) {
            // table_print_all(out);

            print_codes(intercode);

            return 0; // acc
        }

        if (nxt_action > 0) {
            action_shift(nxt_action);
            
            stack_push_T(&tokens[i]);
            
            i ++;
        } else {
            int status = action_reduction(-nxt_action);

            // if (status == -1) {
            //     error_print(i, "multiple definition ", err);

            //     return -1;                
            // }
            // if (status == -2) {
            //     error_print(i, "no definition ", err);

            //     return -1; 
            // }
        }
    }
}