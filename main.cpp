#include"lexical.h"
#include"grammar.h"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("usage: please provide input source file.\n");
        return -1;
    }

	lexical_main(argv[1]);
	grammar_main();
	
	return 0;	
} 
