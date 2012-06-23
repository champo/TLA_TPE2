#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "cfg.h"


void memory_error( void ) {

    printf( "Aborting due to memory error\n" );
    exit(1);
}


struct grammar* new_grammar( void ) {

    struct grammar *grammar = calloc( 1, sizeof( struct grammar ) );

    if ( !grammar ) {

        memory_error();
    }

    grammar->terminals = calloc( 1, 0x100 );

    if ( !grammar->terminals ) {

        free_grammar( grammar );
        memory_error();
    }

    grammar->non_terminals = calloc( 1, 0x100 );

    if ( !grammar->non_terminals ) {

        free_grammar( grammar );
        memory_error();
    }

    memset( grammar->productions, '\0', sizeof( grammar->productions ) );

    return grammar;
}


void free_grammar( struct grammar *grammar ) {

    if ( !grammar )
        return;

    if ( grammar->name )
        free( grammar->name );

    if ( grammar->terminals )
        free( grammar->terminals );

    if ( grammar->non_terminals )
        free( grammar->non_terminals );

    for ( int i = 0; i < 0x100; i++ ) {
        if ( grammar->productions[i].rights ) {
            for ( int j = 0; j < grammar->productions[i].num_rights; j++ )
                free( grammar->productions[i].rights[j] );
            free( grammar->productions[i].rights );
        }
    }

    free( grammar );
}


char **grammar_new_production( struct grammar *grammar, char left ) {

    if ( !grammar )
        return NULL;

    struct production *production = grammar->productions + left;

    if ( production->num_rights >= production->cap_rights ) {

        production->cap_rights *= 2;
        production->cap_rights += 1;

        char **rights = realloc( production->rights, production->cap_rights * sizeof( char* ) );

        if ( !rights ) {

            free_grammar( grammar );
            memory_error();
        }

        memset( rights + production->num_rights, '\0',
                (production->cap_rights - production->num_rights) * sizeof( char* ) );

        production->rights = rights;
    }

    return production->rights + production->num_rights++;
}


//static void add_production( struct grammar *grammar, int left, char right[2] ) {
//
//    struct production *production = grammar->productions + left;
//
//    for ( int i = 0; i < production->num_rights; i++ )
//
//        if ( !memcmp( right, production->rights[i], 2 ) )
//            return;
//
//    memcpy( grammar_new_production( grammar, left ), right, 2 );
//}


void print_grammar( FILE* file, struct grammar *grammar ) {

    fprintf( file, "%s = (\n", grammar->name );

    fprintf( file, "\t{" );
    for ( char *nt = grammar->non_terminals; *nt; nt++ )
        fprintf( file, " %c,", *nt );
    fprintf( file, " }\n" );

    fprintf( file, "\t{" );
    for ( char *nt = grammar->terminals; *nt; nt++ )
        fprintf( file, " %c,", *nt );
    fprintf( file, " }\n" );

    fprintf( file, "\t%c,\n", grammar->initial );

    fprintf( file, "\t{\n" );

    for ( int i = 0; i < 0x100; i++ ) {

        struct production *production = grammar->productions + i;

        for ( int j = 0; j < production->num_rights; j++ )

            fprintf( file, "\t\t%c->%.2s\n", i, production->rights[j] );
    }

    fprintf( file, "\t}\n)\n" );
}


struct grammar *grlval;


int grlex( void );

void* gr_create_buffer( FILE *file, int size );
void  gr_switch_to_buffer( void* new_buffer );

#define YY_BUF_SIZE 16384


int main ( int argc, char **argv ) {

    if ( argc < 2 || argc > 3 ) {

        printf( "Usage:\n\tcfg input_file [output_file]\n" );
        return 1;
    }

    FILE* file = fopen( argv[1], "r" );

    if ( !file ) {

        printf( "Error opening file\n" );
        return 1;
    }

    gr_switch_to_buffer( gr_create_buffer( file, YY_BUF_SIZE ) );

    if ( !grlex() ) {

        print_grammar( stdout, grlval );
        free_grammar( grlval );
    }

    fclose( file );

    return 0;
}

