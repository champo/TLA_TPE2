
all:
	flex -P gr context_free_grammar.l
	gcc -m32 -g -std=c99 -pedantic -w lex.gr.c cfg.c -o genASDR
	rm *.gr.*
