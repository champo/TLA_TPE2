
all:
	flex -P gr context_free_grammar.l
	gcc -m32 -g -std=c99 -pedantic -Wall -Wextra lex.gr.c rg2nfa.c -o rg2nfa
	rm *.gr.*
