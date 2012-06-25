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


int main( int argc, char **argv ) {

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
        print_parser( stdout, grlval );
        free_grammar( grlval );
    }

    fclose( file );

    return 0;
}


static void print_declarations( FILE *file, struct grammar *grammar ) {

    fprintf( file, "#include <stdio.h>\n" );
    fprintf( file, "#include <stdlib.h>\n" );
    fprintf( file, "#include <string.h>\n" );
    fprintf( file, "#include <ctype.h>\n" );
    fprintf( file, "#include <stdbool.h>\n" );
    fprintf( file, "\n" );
    fprintf( file, "struct production {\n" );
    fprintf( file, "\tchar *data;\n" );
    fprintf( file, "\tstruct production *next;\n" );
    fprintf( file, "};\n" );
    fprintf( file, "\n" );

    fprintf( file, "static bool process  ( char *str, struct production prod );\n");

    for ( int i = 0; i < grammar->num_non_terminals; i++ ) {
        fprintf( file, "static bool process_%c( char *str, struct production prod );\n",
                 grammar->non_terminals[i] );
    }

    fprintf( file, "\n" );
    fprintf( file, "static bool (*non_terminal_function[0x100]) ( char*, struct production ) = {\n" );

    for ( int i = 0; i < grammar->num_non_terminals; i++ ) {
        fprintf( file, "\t['%c'] = process_%c,\n", grammar->non_terminals[i], grammar->non_terminals[i] );
    }

    fprintf( file, "};\n" );
}


static void print_process_function( FILE *file, struct grammar *grammar ) {

    fprintf( file, "\n" );
    fprintf( file, "\n" );
    fprintf( file, "static bool process( char *str, struct production prod ) {\n");
    fprintf( file, "\n" );
    fprintf( file, "\twhile ( *prod.data ) {\n" );
    fprintf( file, "\n" );
    fprintf( file, "\t\tchar c = *prod.data++;\n" );
    fprintf( file, "\n" );
    fprintf( file, "\t\tif ( isupper( c ) ) {\n" );
    fprintf( file, "\t\t\treturn non_terminal_function[ c ]( str, prod );\n" );
    fprintf( file, "\t\t}\n" );
    fprintf( file, "\n" );
    fprintf( file, "\t\tif ( c == '\\\\' ) {\n" );
    fprintf( file, "\t\t\tcontinue;\n" );
    fprintf( file, "\t\t}\n" );
    fprintf( file, "\n" );
    fprintf( file, "\t\tif ( *str++ != c ) {\n" );
    fprintf( file, "\t\t\treturn false;\n" );
    fprintf( file, "\t\t}\n" );
    fprintf( file, "\t}\n" );
    fprintf( file, "\n" );
    fprintf( file, "\tif ( prod.next ) {\n" );
    fprintf( file, "\t\treturn process( str, *prod.next );\n" );
    fprintf( file, "\t}\n" );
    fprintf( file, "\n" );
    fprintf( file, "\treturn !*str;\n" );
    fprintf( file, "}\n" );
}


static void print_non_terminal_function( FILE *file, struct grammar *grammar, int non_terminal ) {

    struct production* production = &grammar->productions[ non_terminal ];

    fprintf( file, "\n" );
    fprintf( file, "\n" );
    fprintf( file, "bool process_%c( char *str, struct production prod ) {\n", non_terminal );
    fprintf( file, "\n" );

    for ( int i = 0; i < production->num_rights; i++ ) {

        fprintf( file, "\tif ( process( str, (struct production){ \"%s\", &prod } ) ) {\n", production->rights[i] );
        fprintf( file, "\t\treturn true;\n" );
        fprintf( file, "\t}\n" );
        fprintf( file, "\n" );
    }

    fprintf( file, "\treturn false;\n" );
    fprintf( file, "}\n" );
}


static void print_main( FILE *file, struct grammar *grammar ) {

    fprintf( file, "\n" );
    fprintf( file, "\n" );
    fprintf( file, "int main( int argc, char **argv ) {\n" );
    fprintf( file, "\n" );
    fprintf( file, "\tif ( process_%c( argv[1], (struct production){ \"\", NULL } ) ) {\n", grammar->initial );
    fprintf( file, "\t\tprintf( \"%%s belongs.\\n\", argv[1] );\n" );
    fprintf( file, "\t} else {\n" );
    fprintf( file, "\t\tprintf( \"%%s doesn't belong.\\n\", argv[1] );\n" );
    fprintf( file, "\t}\n" );
    fprintf( file, "\n" );
    fprintf( file, "\treturn 0;\n" );
    fprintf( file, "}\n" );
}


void print_parser( FILE *file, struct grammar *grammar ) {

    print_declarations( file, grammar );

    print_process_function( file, grammar );

    for ( int i = 0; i < grlval->num_non_terminals; i++ ) {
        print_non_terminal_function( file, grammar, grammar->non_terminals[i] );
    }

    print_main( file, grammar );
}

