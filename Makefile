
all:
	flex -P gr context_free_grammar.l
	gcc -m32 -g -std=c99 -pedantic -Wall -Wextra lex.gr.c cfg.c -o cfg
	rm *.gr.*
