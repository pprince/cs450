
/** \file	mpx_sh.c
 *  \brief	MPX Shell, aka Command Handler
 *
 * 	This file contains the functions etc. to implement the user interface for MPX.
 *
**/

/* Author: Paul Prince <paul@littlebluetech.com>
 *
 * Version-control information is managed by Git, and hosted by GitHub:
 *	- Website:	https://github.com/pprince/cs450
 *	- Public Repo:	git://github.com/pprince/cs450.git
 *	- Comitters:	git@github.com:pprince/cs450.git
 *
 * Documentation for developers is generated by Doxygen; for detailed
 * information about the files, functions, data structures, etc. that
 * make up MPX and how they relate to each other, refer to:
 *
 * 	"MPX Programmer's Manual"
 *
 * which can be found in the doc/ directory, or re-generated by running:
 *
 * 	`doxygen doxygen_config`
 *
 * from the top-level repository directory.
 *
**/


#include "mpx_sh.h"
#include "mpx_supt.h"
#include "mpx_util.h"
#include <string.h>


/** This function implements the MPX shell (command-line user interface).
 *
 * mpx_shell() never returns!
 *
 */
void mpx_shell(void) {

	/** A buffer to hold the command line input by the user.
	 *  We include space for the \r, \n, and \0 characters, if any. */
	char cmdline[ MAX_CMDLINE_LEN+2 ];

	/** Buffer size argument for passing to sys_req(). */
	int line_buf_size = MAX_CMDLINE_LEN;

	/** Used to capture the return value of sys_req(). */
	int err;

	/** argc to be passed to MPX command; works just like the one passed to main(). */
	int argc;
	/** argv array to be passed to MPX command; works almost just like the one passed to main().
	 *
	 * But there is one caveat: argv[argc] is undefined in my implementation, not garanteed to be NULL. */
	char **argv;

	/** Temporary pointer for use in string tokenization. */
	char *token;

	/** Delimiters that separate arguments in the MPX shell command-line environment. */
	char *delims = "\t \n";

	/** An index for use in for(;;) loops. */
	int i;
	/** An index for use in nested for(;;) loops. */
	int j;

	/* Loop Forever; this is the REPL. */
	/* This loop terminates only via the MPX 'exit' command. */
	for(;;) {
		/* Read in a line of input from the user. */
		sys_req( READ, TERMINAL, cmdline, &line_buf_size );

		/* Remove trailing newline. */
		mpx_chomp(cmdline);

		/* Allocate space for the argv argument that is to be sent to an MPX command. */
		argv = (char **)sys_alloc_mem( sizeof(char**) * (MAX_ARGS+1) );	/* +1 for argv[0] */
		for( i=0; i < MAX_ARGS+1; i++ ){				/* +1 for argv[0] */
			argv[i] = sys_alloc_mem(MAX_ARG_LEN+1);			/* +1 for \0 */
		}

		/* Tokenize the command line entered by the user, and set argc. */
		/* 0 is a special value here for argc; a value > 0 after the for loop indicates
		 * that tokenizing was successful and that argc and argv contain valid data. */
		argc = 0; token = NULL;

		for( i=0; i < MAX_ARGS+1; i++ ){

			if (i==0) {
				token = strtok( cmdline, delims );
			} else {
				token = strtok( NULL, delims );
			}

			if (token == NULL) {
				/* No more arguments. */
				break;
			}

			if (strlen(token) > MAX_ARG_LEN) {
				/* This argument is too long. */
				printf("ERROR: Argument too long. MAX_ARG_LEN is %d.\n", MAX_ARG_LEN);
				argc = 0;
				break;
			}

			argc++;
			strcpy( argv[i], token );
		}

		if ( strtok( NULL, delims ) != NULL ){
			/* Too many arguments. */
			printf("ERROR: Too many arguments. MAX_ARGS is %d.\n", MAX_ARGS);
			continue;
		}

		if ( argc <= 0 ) {
			printf("ERROR: Invalid command-line.");
			continue;
		}

		/* Temporary: Print out tokenization. */
		for( i=0; i < argc+1; i++ ){
			printf("    argv[%d] = \"%s\"\n", i, argv[i]);
		}

		/* Free the memory for the dynamically-allocated *argv[] */
		for( i=0; i < MAX_ARGS+1; i++ ){
			sys_free_mem( argv[i] );
		}
		sys_free_mem( argv );
	}
}
