#include "pl0_global.h"
// #include"lexical.h"
// #include"grammar.h"

void *alloc_assert(int len)
{
    void *p = malloc(len);
    if (!p)
    {
        fprintf(stderr, "\nerror allocating memory.\n");
        exit(1);
    }
    return p;
}

int process(FILE* source, FILE* lex_code, FILE* inter_code) {
    if(icode_lex(source, lex_code) < 0) 
        return -1;

    if(icode_gen(inter_code) < 0)
        return -1;
    
    return 0;    
}

int main(int argc, char* argv[]) {

     if (argc < 2) {
        printf("usage: please provide input source file.\n");
        return -1;
    }

	// lexical_main(argv[1]);
	// semantic_main();

    FILE *source, *lex_code, *inter_code;

    source = fopen(argv[1], "r");
    lex_code = fopen("tmp/pre.lex", "w");
    inter_code = fopen("output/icode.txt", "w");


    int status = process(source, lex_code, inter_code);

    fclose(source);
    fclose(lex_code);
    fclose(inter_code);

    if (status != 0) return -1;

    // run(global_entry);

    return 0;
}