
all:
	flex -P gr context_free_grammar.l
	gcc -g -std=c99 -pedantic -Wall -Wextra lex.gr.c cfg.c -o genASDR
	rm *.gr.*
