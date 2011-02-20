
/** \file	mpx_cmds.c
 *  \brief	MPX User Commands
 *
 * 	This file implements each of the user commands for MPX.
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


#include "mpx_cmds.h"
#include "mpx_supt.h"
#include "mpx_util.h"
#include <string.h>


static struct mpx_command *list_head = NULL;

void add_command( char *name, void (*function)(int argc, char *argv[]) ){

	/* Temporary variable for iterating through the list of commands. */
	struct mpx_command *this_command;

	/* Allocate space for the new command structure. */
	struct mpx_command *new_command = (struct mpx_command *)sys_alloc_mem(sizeof(struct mpx_command));
	new_command->name = (char *)sys_alloc_mem(MAX_ARG_LEN+1);
		/* FIXME: check for malloc failure! */

	/* Initialize the structure. */
	strcpy( new_command->name, name );
	new_command->function = function;
	new_command->next = NULL;

	/* Insert the new command into the list of commands. */
	this_command = list_head;
	if ( this_command == NULL ) {
		list_head = new_command;
	} else {
		while ( this_command->next != NULL ){
			this_command = this_command->next;
		}
		this_command->next = new_command;
	}
}

void dispatch_command( char *name, int argc, char *argv[] ) {

	/* Temporary variable for iterating through the list of commands. */
	struct mpx_command *this_command = list_head;

	/* Temporary variables to keep track of matching command names. */
	int num_matches = 0;
	struct mpx_command *first_match;

	/* Iterate through the linked list of commands, */
	while( this_command != NULL ) {

		/* Check to see if the given command is a valid abbrev. for the current command from the list */
		if( strncmp( this_command->name, name, strlen(name) ) == 0 ) {
			/* If so, keep track of how many matches thus far, */
			num_matches++;
			if (num_matches == 1) {
				/* This is the first match in the list for the given command. */
				first_match = this_command;
			} else if (num_matches == 2) {
				/* This is the first duplicate match in the list;
				 * Print out the 'ambiguous command' header,
				 * plus the first AND current ambiguous commands. */
				printf("Ambiguous command: %s\n", name);
				printf("    Matches:\n");
				printf("        %s\n", first_match->name);
				printf("        %s\n", this_command->name);
			} else {
				/* This is a subsequent duplicate match;
				 * by this time, the header etc. has already been printed,
				 * so we only need to print out the current command name. */
				printf("        %s\n", this_command->name);
			}
		}

		this_command = this_command->next;
	}

	/* If we got a command name that matches unambiguously, run that command. */
	if ( num_matches == 1 ){
		first_match->function(argc, argv);
	}
	
	/* Otherwise, if we got no matches at all, say so. */
	if ( num_matches == 0 ){
		printf("ERROR: Invalid command name.\n");
		printf("Type \"commands\" to see a list of valid commands.\n");
	}
}

void mpxcmd_commands( int argc, char *argv[] ) {

	/* Temporary variable for iterating through the list of commands. */
	struct mpx_command *this_command = list_head;

	while( this_command != NULL ) {

		printf("        %s\n", this_command->name);

		this_command = this_command->next;
	}
}


void mpxcmd_date( int argc, char *argv[] ) {
	int retval;	/**< Temp. storage for the return value of sys_ functions. */
	date_rec date;	/**< Structure to hold a date (day, month, and year).
	                     Will be used for both getting and setting the MPX system date. */

	if ( argc == 1 ){
		sys_get_date(&date);
		printf("Current MPX system date (yyyy-mm-dd): %d-%d-%d\n", date.year, date.month, date.day);
		return;
	}

	if ( argc == 4 ){

		date.year  = atoi(argv[1]);
		date.month = atoi(argv[2]);
		date.day   = atoi(argv[3]);

		if ( ! mpx_validate_date(date.year, date.month, date.day) ) {
			printf("ERROR: Invalid date specified; MPX system date is unchanged.\n");
			return;
		}

		retval = sys_set_date(&date);
		if ( retval != 0 ) {
			printf("ERROR: sys_set_date() returned an error.\n");
			return;
		}

		printf("The MPX system date has been changed.\n");
		return;
}


void init_commands(void) {
	add_command("commands", mpxcmd_commands);
	add_command("date", mpxcmd_date);
}
