
/*! @file	mpx_sh.c
 *  @brief	MPX Shell, aka Command Handler
 *
 * 	This file implements the user interface for MPX.
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
 * which can be found in the doc/ directory.
 *
**/


#include "mpx_sh.h"
#include "mpx_supt.h"
#include "mpx_util.h"
#include "mpx_cmds.h"
#include <string.h>


/*! @brief The current prompt string. */
static char *mpx_prompt_string = NULL;


/*! @brief Sets the current prompt to whatever string is given.
 *
 * If new_prompt is NULL, this is a no-op. */
void mpx_setprompt( char *new_prompt ){
	if (new_prompt == NULL) return;
	if (mpx_prompt_string != NULL) {
		sys_free_mem(mpx_prompt_string);
	}
	mpx_prompt_string = (char *)sys_alloc_mem(strlen(new_prompt)+1);
	strcpy(mpx_prompt_string, new_prompt);
}


/*! This function implements the MPX shell (command-line user interface).
 *
 * mpx_shell() never returns!
 *
 * \bug A command should be able to depend on argv[argc] == NULL, but we do
 * not currently implement this feature.
 *
 */
void mpx_shell(void) {

	/* A buffer to hold the command line input by the user.
	 * We include space for the \r, \n, and \0 characters, if any. */
	char cmdline[ MAX_CMDLINE_LEN+2 ];

	/* Buffer size argument for passing to sys_req(). */
	int line_buf_size = MAX_CMDLINE_LEN;

	/* Used to capture the return value of sys_req(). */
	int err;

	/* argc to pass to MPX command; works like the one passed to main. */
	int argc;
	/* argv to pass to MPX command; works almost like the one passed to main
	 *
	 * But there is one caveat: argv[argc] is undefined in my
	 * implementation, not garanteed to be NULL. */
	char **argv;

	/* Temporary pointer for use in string tokenization. */
	char *token;

	/* Delimiters that separate arguments in the MPX shell command-line
	 * environment. */
	char *delims = "\t \n";

	/* An index for use in for(;;) loops. */
	int i;
	/* An index for use in nested for(;;) loops. */
	int j;

	/* A flag to track if a single argument was too long...
	 * This is kind of a quick-and-dirty workaround for C not having
	 * the 'continue LABEL' feature. */
	int arg_too_long;

	/* We must initialize the prompt string. */
	mpx_setprompt(MPX_DEFAULT_PROMPT);

	/* Loop Forever; this is the REPL. */
	/* This loop terminates only via the MPX 'exit' command. */
	for(;;) {

		arg_too_long = 0;

		/* Output the current MPX prompt string. */
		printf("%s", mpx_prompt_string);

		/* Read in a line of input from the user. */
		sys_req( READ, TERMINAL, cmdline, &line_buf_size );

		/* Remove trailing newline. */
		mpx_chomp(cmdline);

		/* Allocate space for argv */
		/* *********************** */

		/* +1 for argv[0] */
		argv = (char **)sys_alloc_mem( sizeof(char**) * (MAX_ARGS+1) );
		/* +1 for argv[0] */
		for( i=0; i < MAX_ARGS+1; i++ ){				
			/* +1 for \0 */
			argv[i] = sys_alloc_mem(MAX_ARG_LEN+1);
		}


		/* Tokenize the command line entered by the user + set argc. */
		/* ********************************************************* */

		/* 0 is a special value here for argc; a value > 0 after the
		 * for loop indicates that tokenizing was successful and that
		 * argc and argv contain valid data.
		 * 
		 *****  NOTE:  argc includes argv[0], but MAX_ARGS does not! */

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
				arg_too_long = 1;
				argc = 0;
				break;
			}

			argc++;
			strcpy( argv[i], token );
		}

		if ( arg_too_long ){
			printf("ERROR: Argument too long. MAX_ARG_LEN is %d.\n",
				MAX_ARG_LEN
			);
			continue;
			/*! @bug Allocated memory not freed in error cases. */
		}

		if ( strtok( NULL, delims ) != NULL ){
			/* Too many arguments. */
			printf("ERROR: Too many arguments. MAX_ARGS is %d.\n", MAX_ARGS);
			continue;
		}

		if ( argc <= 0 ) {
			/* Blank command; just re-print the prompt. */
			continue;
		}

		/* Run the command, or print an error if it is invalid. */
		dispatch_command( argv[0], argc, argv );

		/* Free the memory for the dynamically-allocated *argv[] */
		for( i=0; i < MAX_ARGS+1; i++ ){
			sys_free_mem( argv[i] );
		}
		sys_free_mem( argv );
	}
}
