#ifndef __CFG_H__
#define __CFG_H__


struct grammar {

    char *name;

    int num_terminals;
    int num_non_terminals;
    int num_productions;

    char *terminals;
    char *non_terminals;
    char initial;
    char empty;

    struct production {

        char **rights;
        int num_rights;
        int cap_rights; /* for internal use */

    } productions[ 0x100 ];

};


void memory_error( void );

struct grammar* new_grammar( void );

void free_grammar( struct grammar *grammar );

char **grammar_new_production( struct grammar *grammar, char left );

void print_grammar( FILE *file, struct grammar *grammar );

void print_parser(FILE* file, struct grammar* grammar);



#endif /* __CFG_H__ */

