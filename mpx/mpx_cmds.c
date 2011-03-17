
/*! @file	mpx_cmds.c
 *  @brief	MPX shell commands (help, ls, exit, etc.)
 *  @author	Paul Prince <paul@littlebluetech.com>
 *  @date	2011
 *
 *  This file implements each of the user commands for MPX.
 *
 *  @todo
 * 	We should typedef structs (particularly struct mpx_command).
 */


#include "mpx_cmds.h"
#include "mpx_supt.h"
#include "mpx_util.h"
#include "pcb.h"
#include <string.h>


/*! @brief	A linked-list of MPX shell commands
 *
 */
static struct mpx_command *list_head = NULL;


/*! @brief	Adds a command to the MPX shell.
 *
 */
void add_command(
	/*! [in] The command name that will be made available in the shell. */
	char *name,
	/*! [in] The C function which will implement the shell command. */
	void (*function)(int argc, char *argv[])
)
{
	/* Temporary variable for iterating through the list of commands. */
	struct mpx_command *this_command;

	/* Allocate space for the new command structure. */
	struct mpx_command *new_command =
		(struct mpx_command *)sys_alloc_mem(sizeof(struct mpx_command));
	new_command->name = (char *)sys_alloc_mem(MAX_ARG_LEN+1);
		/*! @bug	This function doesn't check for failure to
 		 *		allocate memory for the new command struct. */

	/* Initialize the structure. */
	strcpy( new_command->name, name );
	new_command->function = function;
	new_command->next = NULL;

	/* Insert the new command into the linked-list of commands. */
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

/*! @brief	Runs the shell command specified by the user, if it is valid.
 *
 *  This function checks to see if the shell command given unabiguously matches
 *  a valid MPX shell command, and if so, runs that command (passing the
 *  provided argc and argv through).
 *
 *  This dispatcher allows abbreviated commands; if the requested command
 *  matches multiple (or zero) valid MPX shell commands, the user is alerted.
 *
 *  @attention	Produces output (via printf)!
 */
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

	printf("\n");
	printf("    The following commands are available to you:\n");
	printf("\n");

	while( this_command != NULL ) {

		printf("        %s\n", this_command->name);

		this_command = this_command->next;
	}
}


void mpxcmd_date( int argc, char *argv[] ) {
	/*!< Temp. storage for the return value of sys_ functions. */
	int retval;	
	/*!< Structure to hold a date (day, month, and year).
	     Will be used for both getting and setting the MPX system date. */
	date_rec date;	

	if ( argc == 1 ){
		sys_get_date(&date);
		printf("Current MPX system date (yyyy-mm-dd): %04d-%02d-%02d\n", date.year, date.month, date.day);
		return;
	}

	if ( argc == 4 ){

		date.year  = atoi(argv[1]);
		date.month = atoi(argv[2]);
		date.day   = atoi(argv[3]);

		if ( ! mpx_validate_date(date.year, date.month, date.day) ) {
			printf("ERROR: Invalid date specified; MPX system date is unchanged.\n");
			printf("       Valid dates are between 1900-01-01 and 2999-12-31, inclusive.\n");
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

	printf("ERROR: Wrong number of arguments to 'date'.\n");
	printf("       Type 'help date' for usage information.\n");
}


void mpxcmd_exit( int argc, char *argv[] ) {
	char buf[21];
	int buf_size=20;
	int retval;

	printf("  ** Are you sure you want to terminate MPX? [y/n] ");

	retval = sys_req( READ, TERMINAL, buf, &buf_size );
	if ( retval < 0 ) {
		printf("ERROR: sys_req() threw error while trying to read from the terminal!\n");
		return;
	}

	mpx_chomp(buf);

	if ( strlen(buf) == 3 ) {
		if ( ( buf[0] == 'y' || buf[0] == 'Y') &&
		     ( buf[1] == 'e' || buf[1] == 'E') &&
		     ( buf[2] == 's' || buf[2] == 'S') ){
				sys_exit();
		}
	}
	if (strlen(buf) == 1 ) {
		if ( buf[0] == 'y' || buf[0] == 'Y' ){
			sys_exit();
		}
	}
}


void mpxcmd_help( int argc, char *argv[] ) {
	/* Must leave space for the path prefix, the command name, the filename suffix, and the \0. */ 
	char helpfile[MAX_ARG_LEN+1+7+4] = "./help/";

	if ( argc == 1 ) {
		mpxcmd_commands(argc, argv);
		printf("\n");
		printf("    For detailed help a specific command, type:  help <command>\n");
		return;
	}

	if ( argc == 2 ) {

		strncat(helpfile, argv[1], MAX_ARG_LEN);
		strncat(helpfile, ".hlp", 4);

		printf("\n");
		if ( ! mpx_cat(helpfile) ){
			printf("No help available for command '%s'\n", argv[1]);
		}
		return;
	}

	printf("ERROR: Wrong number of arguments to 'help'.\n");
	printf("       Type 'help help' for usage information.\n");
}


void mpxcmd_version( int argc, char *argv[] ){
	printf("MPX System Version: %s\n", MPX_VERSION);
}


void mpxcmd_ls( int argc, char *argv[] ){
	int	retval;
	char	*dir;
	int 	num_files;
	char	file_name[MAX_FILENAME_LEN+1];
	long	file_size;

	if ( argc == 1 ){
		dir = MPX_DEFAULT_EXE_DIR;
	}
	else if ( argc == 2 ){
		dir = argv[1];
	}
	else {
		printf("ERROR: Wrong number of arguments to 'ls'.\n");
		printf("       Type 'help ls' for usage information.\n");
		return;
	}

	retval = sys_open_dir( dir );
	if ( retval != 0 ){
		printf("ERROR: sys_open_dir() failed trying to open directory '%s'.\n", dir);
		return;
	}

	printf("\n");
	printf("    Listing of files in directory '%s':\n", dir);
	printf("\n");
	printf("File Name:         File Size (in bytes):\n");
	printf("---------------    ------------------------------\n");

	num_files = 0;
	for(;;){
		retval = sys_get_entry( file_name, MAX_FILENAME_LEN, &file_size );
		if ( retval == 0 ) {
			printf("%-15s    %30ld\n", file_name, file_size);
			num_files++;
		}
		else if ( retval == ERR_SUP_NOENTR ) {
			break;
		}
		else {
			printf("ERROR: sys_get_entry() failed trying to read directory '%s'.\n", dir);
			printf("Giving up on this directory.\n");
			return;
		}
	}

	printf("\n");
	printf("Total files in directory: %d\n", num_files);

	retval = sys_close_dir();
	if ( retval != 0 ){
		printf("ERROR: sys_close_dir() returned an error.\n");
		printf("Internal program state is unknown; you should exit and restart MPX.\n");
	}
}


/*! Implements the <tt>suspend</tt> shell command.
 */
void mpxcmd_suspend ( int argc, char *argv[] )
{
}


/*! Implements the <tt>resume</tt> shell command.
 *
 */
void mpxcmd_resume ( int argc, char *argv[] )
{
}


/*! Implements the <tt>renice</tt> shell command.
 *
 */
void mpxcmd_renice ( int argc, char *argv[] )
{
}


/*! Implements the <tt>ps</tt> shell command.
 *
 */
void mpxcmd_ps ( int argc, char *argv[] )
{
	int i;
	pcb_queue_node_t *iter_node;

	for ( i=0; i<4; i++ ){
		iter_node = queues[i]->head;
		while ( iter_node != NULL ) {
			printf("%16s %4d\n",
				iter_node->pcb->name,
				iter_node->pcb->priority
			);
			iter_node = iter_node->next;
		}
	}

	printf("\nReversed:\n");
	for ( i=3; i>=0; i-- ){
		iter_node = queues[i]->tail;
		while ( iter_node != NULL ) {
			printf("%16s %4d\n",
				iter_node->pcb->name,
				iter_node->pcb->priority
			);
			iter_node = iter_node->prev;
		}
	}


	printf("\n\n**> TEST OF foreach_listitem():\n");

	foreach_listitem( iter_node, queues[0] ){
		printf("%16s %4d\n",
			iter_node->pcb->name,
			iter_node->pcb->priority
		);
	}
		

}


/*! Implements the <tt>create_pcb</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_create_pcb ( int argc, char *argv[] )
{
	pcb_t		*new_pcb;
	int		new_pcb_priority;
	process_class_t	new_pcb_class;
	pcb_queue_t	*new_pcb_dest_queue;

	if ( argc != 4 ){
		printf("ERROR: Wrong number of arguments to create_pcb.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN ) {
		printf("ERROR: Specified process name is too long.\n");
		return;
	}

	new_pcb_priority = atoi(argv[3]);

	if ( new_pcb_priority < -127 || new_pcb_priority > 128 ){
		printf("ERROR: Invalid priority specified.\n");
		printf("Priority must be between -127 and 128 (inclusive).\n");
		return;
	}
	
	if ( strlen(argv[2]) == 1 && argv[2][0] == 'A' ) {
		new_pcb_class = APPLICATION;
	} else if ( strlen(argv[2]) == 1 && argv[2][0] == 'S' ) {
		new_pcb_class = SYSTEM;
	} else {
		printf("ERROR: Invalid process class specified.\n");
		return;
	}
	
	new_pcb = setup_pcb( argv[1], new_pcb_priority, new_pcb_class);

	if ( new_pcb == NULL ){
		printf("ERROR: Failure creating process.\n");
		return;
	}

	new_pcb_dest_queue = insert_pcb( new_pcb );

	if ( new_pcb_dest_queue == NULL ){
		printf("ERROR: Failure enqueuing new process.\n");
	}

	printf("Success: Process created.\n");
}


/*! Implements the <tt>delete_pcb</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_delete_pcb ( int argc, char *argv[] )
{
}


/*! Implements the <tt>block</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_block ( int argc, char *argv[] )
{
}


/*! Implements the <tt>unblock</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_unblock ( int argc, char *argv[] )
{
}


void init_commands(void) {
	/* R1 commands */
	/* add_command("commands", mpxcmd_commands); */
	add_command("date", mpxcmd_date);
	add_command("exit", mpxcmd_exit);
	add_command("help", mpxcmd_help);
	add_command("ls", mpxcmd_ls);
	add_command("version", mpxcmd_version);

	/* R2 commands */
	add_command("suspend", mpxcmd_suspend);
	add_command("resume", mpxcmd_resume);
	add_command("renice", mpxcmd_renice);
	add_command("ps", mpxcmd_ps);

	/* R2 Tempoary commands */
	add_command("create_pcb", mpxcmd_create_pcb);
	add_command("delete_pcb", mpxcmd_delete_pcb);
	add_command("block", mpxcmd_block);
	add_command("unblock", mpxcmd_unblock);
}
