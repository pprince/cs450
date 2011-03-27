
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
void dispatch_command( char *name, int argc, char *argv[] )
{

	/* Temporary variable for iterating through the list of commands. */
	struct mpx_command *this_command = list_head;

	/* Temporary variables to keep track of matching command names. */
	int num_matches = 0;
	struct mpx_command *first_match;

	/* Iterate through the linked list of commands, */
	while( this_command != NULL ) {

		/* Check to see if the given command is a valid abbrev. for the
		 * current command from the list: */
		if( strncmp( this_command->name, name, strlen(name) ) == 0 ) {
			/* If so, keep track of how many matches thus far, */
			num_matches++;
			if (num_matches == 1) {
				/* This is the first match in the list for
				 * the given command. */
				first_match = this_command;
			} else if (num_matches == 2) {
				/* This is the first duplicate match; print
				 * out the 'ambiguous command' header, plus
				 * the first AND current ambiguous commands. */
				printf("Ambiguous command: %s\n", name);
				printf("    Matches:\n");
				printf("        %s\n", first_match->name);
				printf("        %s\n", this_command->name);
			} else {
				/* This is a subsequent duplicate match;
				 * by this time, the header etc. has already
				 * been printed, so we only need to print out
				 * the current command name. */
				printf("        %s\n", this_command->name);
			}
		}

		this_command = this_command->next;
	}

	/* If we got a command name that matches unambiguously, run that cmd: */
	if ( num_matches == 1 ){
		first_match->function(argc, argv);
	}
	
	/* Otherwise, if we got no matches at all, say so: */
	if ( num_matches == 0 ){
		printf("ERROR: Invalid command name.\n");
		printf("Type \"help\" to see a list of valid commands.\n");
	}
}

void mpxcmd_commands( int argc, char *argv[] )
{

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


void mpxcmd_date( int argc, char *argv[] )
{
	/* Temporary storage for the return value of sys_ functions. */
	int retval;	

	/* Structure to hold a date (day, month, and year). */
	date_rec date;	

	/* Called with no arguments, we should just print the date: */
	if ( argc == 1 ){
		sys_get_date(&date);
		printf("Current MPX system date (yyyy-mm-dd): %04d-%02d-%02d\n",
				date.year, date.month, date.day);
		return;
	}

	/* Called with three arguments, we need to try to set the date
	 * from the values provided by the user. */
	if ( argc == 4 ){

		/* Convert string arguments to integer values: */
		date.year  = atoi(argv[1]);
		date.month = atoi(argv[2]);
		date.day   = atoi(argv[3]);

		/* Check that the user's input represents a valid date: */
		if ( ! mpx_validate_date(date.year, date.month, date.day) ) {
			/* Lines broken confusingly due to 80-column limit: */
			printf("ERROR: Invalid date specified; ");
			printf("MPX system date is unchanged.\n");
			printf("       Valid dates are between 1900-01-01 ");
			printf("and 2999-12-31, inclusive.\n");
			return;
		}

		/* Attempt to set that date, catching errors: */
		retval = sys_set_date(&date);
		if ( retval != 0 ) {
			printf("ERROR: sys_set_date() returned an error.\n");
			return;
		}

		/* Success! */
		printf("The MPX system date has been changed.\n");
		return;
	}

	/* If we get to here, the user invoked the date command incorrectly. */
	printf("ERROR: Wrong number of arguments to 'date'.\n");
	printf("       Type 'help date' for usage information.\n");
}


void mpxcmd_exit( int argc, char *argv[] )
{
	char buf[21];
	int buf_size=20;
	int retval;

	printf("  ** Are you sure you want to terminate MPX? [y/n] ");

	retval = sys_req( READ, TERMINAL, buf, &buf_size );
	if ( retval < 0 ) {
		printf("ERROR: sys_req() threw error while trying to read ");
		printf("from the terminal!\n");
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


void mpxcmd_help( int argc, char *argv[] )
{

	/* Buffer to hold the name of the Help File we will display.
	 *   Note: we must leave space for the path prefix, the command name,
	 *   the filename suffix, and the \0.	*/ 
	char helpfile[MAX_ARG_LEN+1+7+4] = "./help/";

	if ( argc == 1 ) {
		mpxcmd_commands(argc, argv);
		printf("\n");
		printf("    For detailed help a specific command, type:\n");
		printf("\n");
		printf("            help <command>\n");
		printf("            --------------\n");
		printf("\n");
		printf("   at the MPX shell prompt.\n");
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


void mpxcmd_version( int argc, char *argv[] )
{
	printf("MPX System Version: %s\n", MPX_VERSION);
}


void mpxcmd_ls( int argc, char *argv[] )
{
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
		printf("ERROR: sys_open_dir() failed");
		printf("trying to open directory '%s'.\n", dir);
		return;
	}

	printf("\n");
	printf("    Listing of files in directory '%s':\n", dir);
	printf("\n");
	printf("File Name:         File Size (in bytes):\n");
	printf("---------------    ------------------------------\n");

	num_files = 0;
	for(;;){
		retval = sys_get_entry(file_name, MAX_FILENAME_LEN, &file_size);
		if ( retval == 0 ) {
			printf("%-15s    %30ld\n", file_name, file_size);
			num_files++;
		}
		else if ( retval == ERR_SUP_NOENTR ) {
			break;
		}
		else {
			printf("ERROR: sys_get_entry() failed");
			printf("trying to read directory '%s'.\n", dir);
			printf("Giving up on this directory.\n");
			return;
		}
	}

	printf("\n");
	printf("Total files in directory: %d\n", num_files);

	retval = sys_close_dir();
	if ( retval != 0 ){
		printf("ERROR: sys_close_dir() returned an error.\n");
	}
}


/*! Implements the <tt>suspend</tt> shell command.
 */
void mpxcmd_suspend ( int argc, char *argv[] )
{
	pcb_t		*pcb;

	if ( argc != 2 ){
		printf("ERROR: Wrong number of arguments to suspend.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN || strlen(argv[1]) < 1 ){
		printf("ERROR: Invalid process name.\n");
		return;
	}

	pcb = find_pcb( argv[1] );
	if ( pcb == NULL ){
		printf("ERROR: Specified process does not exist.\n");
		return;
	}

	if ( is_suspended(pcb) ){
		printf("ERROR: Specified process is already suspended.\n");
		return;
	}

	/* Suspend PCB, checking for error return. */
	if ( ! suspend_pcb(pcb) ){
		printf("ERROR: Unspecified error suspending process.");
		return;
	}

	/* Let the user know that the operation was successful. */
	printf("Success: Process '%s' is now suspended.\n", argv[1]);
}


/*! Implements the <tt>resume</tt> shell command.
 *
 */
void mpxcmd_resume ( int argc, char *argv[] )
{
	pcb_t		*pcb;

	if ( argc != 2 ){
		printf("ERROR: Wrong number of arguments to resume.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN || strlen(argv[1]) < 1 ){
		printf("ERROR: Invalid process name.\n");
		return;
	}

	pcb = find_pcb( argv[1] );
	if ( pcb == NULL ){
		printf("ERROR: Specified process does not exist.\n");
		return;
	}

	if ( ! is_suspended(pcb) ){
		printf("ERROR: Specified process is not suspended.\n");
		return;
	}

	/* Un-suspend PCB, checking for error return. */
	if ( ! resume_pcb(pcb) ){
		printf("ERROR: Unspecified error resuming process.");
		return;
	}

	/* Let the user know that the operation was successful. */
	printf("Success: Process '%s' is no longer suspended.\n", argv[1]);
}


/*! Implements the <tt>renice</tt> shell command.
 *
 */
void mpxcmd_renice ( int argc, char *argv[] )
{
}


void print_pcb_info( pcb_t *pcb ){
	char *process_state = process_state_to_string(pcb->state);
	char *process_class = process_class_to_string(pcb->class);
	
	printf("\n");
	printf("+-PROCESS----- Name: %-24s",  pcb->name);
		printf(" --------------------\n");
	printf("|             Class: %s\n",   process_class);
	printf("|          Priority: %-4d\n", pcb->priority);
	printf("|             State: %s\n",   process_state);
	printf("|       Memory Size: %-8d\n", pcb->memory_size);
	printf("|        Stack Size: %-8d\n", pcb->stack_top - pcb->stack_base);
	printf("+----------------------------------------------------------\n");
}


void print_pcb_info_oneline( pcb_t *pcb ){
	char *process_state = process_state_to_string(pcb->state);
	char process_class = process_class_to_char(pcb->class);

	printf("%-24s    %c    %4d  %8d  %8d  \n",
		pcb->name,
		process_class,
		pcb->priority,
		pcb->memory_size,
		(pcb->stack_top - pcb->stack_base)
		/* process_state */
	);
}


/*! Implements the <tt>ps</tt> shell command.
 *
 */
void mpxcmd_ps ( int argc, char *argv[] )
{
	int i;
	pcb_t *specified_pcb;
	pcb_queue_node_t *iter_node;

	int print_ready		= 0;
	int print_suspended	= 0;
	int print_blocked	= 0;
	int print_in_reverse	= 0;

	if ( argc == 3 && strcmp("--", argv[2]) == 0 ){
		specified_pcb = find_pcb( argv[1] );
		if (specified_pcb == NULL ){
			printf("ERROR: Specified process does not exist.\n");
			return;
		}
		print_pcb_info( specified_pcb );
		return;
	}

	if ( argc == 2 && argv[1][0] != '-' ){
		specified_pcb = find_pcb( argv[1] );
		if (specified_pcb == NULL ){
			printf("ERROR: Specified process does not exist.\n");
			return;
		}
		print_pcb_info( specified_pcb );
		return;
	}

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp("-r", argv[i]) == 0 ){
			print_ready = 1;
		} else
		if (strcmp( "-s", argv[i]) == 0 ){
			print_suspended = 1;
		} else
		if (strcmp( "-b", argv[i]) == 0 ){
			print_blocked = 1;
		} else
		if (strcmp( "-a", argv[i]) == 0 ){
			print_ready = 1;
			print_suspended = 1;
			print_blocked = 1;
		} else
		if (strcmp( "-R", argv[i]) == 0 ){
			print_in_reverse = 1;
		}
		else {
			printf("ERROR: Invalid argument '%s'.", argv[i]);
			printf("Remember, flags are case-sensitive.\n");
			return;
		}
	}

	if ( argc == 1 || (argc == 2 && print_in_reverse) ) {
		print_ready = 1;
		print_suspended = 1;
		print_blocked = 1;
	}

	printf("\n");
	printf(" ===");
	printf(" =======================  =====  ====  ========  ========");
	printf("  ================\n");
	printf("    ");
	printf(" Process Name             Class  Prio  Mem Size  Stk Size");
	printf("  State\n");
	printf(" ===");
	printf(" =======================  =====  ====  ========  ========");
	printf("  ================\n");

	if ( print_ready ){
		printf("\n");
		printf("  Processes in state READY:\n");
		printf("  -------------------------\n");
		foreach_listitem_rev(
			iter_node,
			get_queue_by_state(READY),
			print_in_reverse
		){
			printf("    ");
			print_pcb_info_oneline( iter_node->pcb );
		}
	}

	if ( print_blocked ){
		printf("\n");
		printf("  Processes in state BLOCKED:\n");
		printf("  ---------------------------\n");
		foreach_listitem_rev(
			iter_node,
			get_queue_by_state(BLOCKED),
			print_in_reverse
		){
			printf("    ");
			print_pcb_info_oneline( iter_node->pcb );
		}
	}
		
	if ( print_ready || print_suspended ){
		printf("\n");
		printf("  Processes in state SUSP_READY:\n");
		printf("  ------------------------------\n");
		foreach_listitem_rev(
			iter_node,
			get_queue_by_state(SUSP_READY),
			print_in_reverse
		){
			printf("    ");
			print_pcb_info_oneline( iter_node->pcb );
		}
	}

	if ( print_blocked || print_suspended ){
		printf("\n");
		printf("  Processes in state SUSP_BLOCKED:\n");
		printf("  --------------------------------\n");
		foreach_listitem_rev(
			iter_node,
			get_queue_by_state(SUSP_BLOCKED),
			print_in_reverse
		){
			printf("    ");
			print_pcb_info_oneline( iter_node->pcb );
		}
	}
}


/*! Implements the <tt>create_pcb</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_create_pcb ( int argc, char *argv[] )
{
	pcb_t		*new_pcb;
	int		 new_pcb_priority;
	process_class_t	 new_pcb_class;
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
	
	if ( strlen(argv[2]) == 1 &&
				(argv[2][0] == 'A' || argv[2][0] == 'a') ){
		new_pcb_class = APPLICATION;
	} else if ( strlen(argv[2]) == 1 &&
				(argv[2][0] == 'S' || argv[2][0] == 's') ){
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
	pcb_t		*pcb;
	pcb_queue_t	*retval;

	if ( argc != 2 ){
		printf("ERROR: Wrong number of arguments to create_pcb.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN || strlen(argv[1]) < 1 ){
		printf("ERROR: Invalid process name.\n");
		return;
	}
	
	pcb = find_pcb( argv[1] );
	if ( pcb == NULL ){
		printf("ERROR: Specified process does not exist.\n");
		return;
	}

	retval = remove_pcb( pcb );
	if ( retval == NULL ){
		printf("ERROR: Unspecified error removing PCB.\n");
		return;
	}

	free_pcb( pcb );

	printf("Success: PCB for process '%s' removed.\n", argv[1]);
}


/*! Implements the <tt>block</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_block ( int argc, char *argv[] )
{
	pcb_t		*pcb;

	if ( argc != 2 ){
		printf("ERROR: Wrong number of arguments to block.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN || strlen(argv[1]) < 1 ){
		printf("ERROR: Invalid process name.\n");
		return;
	}

	pcb = find_pcb( argv[1] );
	if ( pcb == NULL ){
		printf("ERROR: Specified process does not exist.\n");
		return;
	}

	if ( is_blocked(pcb) ){
		printf("ERROR: Specified process is already blocked.\n");
		return;
	}

	/* Block PCB, checking for error return. */
	if ( ! block_pcb(pcb) ){
		printf("ERROR: Unspecified error blocking process.");
		return;
	}

	/* Let the user know that the operation was successful. */
	printf("Success: Process '%s' is now blocked.\n", argv[1]);
}


/*! Implements the <tt>unblock</tt> shell command.
 *
 * \attention This TEMPORARY command will be replaced later. */
void mpxcmd_unblock ( int argc, char *argv[] )
{
	pcb_t		*pcb;

	if ( argc != 2 ){
		printf("ERROR: Wrong number of arguments to unblock.\n");
		return;
	}

	if ( strlen(argv[1]) > MAX_ARG_LEN || strlen(argv[1]) < 1 ){
		printf("ERROR: Invalid process name.\n");
		return;
	}

	pcb = find_pcb( argv[1] );
	if ( pcb == NULL ){
		printf("ERROR: Specified process does not exist.\n");
		return;
	}

	if ( ! is_blocked(pcb) ){
		printf("ERROR: Specified process is not blocked.\n");
		return;
	}

	/* Unblock PCB, checking for error return. */
	if ( ! unblock_pcb(pcb) ){
		printf("ERROR: Unspecified error unblocking process.");
		return;
	}

	/* Let the user know that the operation was successful. */
	printf("Success: Process '%s' is now unblocked.\n", argv[1]);

}


void init_commands(void)
{
	/* R1 commands */
	add_command("date", mpxcmd_date);
	add_command("exit", mpxcmd_exit);
	add_command("help", mpxcmd_help);
	add_command("ls", mpxcmd_ls);
	add_command("version", mpxcmd_version);
		/* Removed after R1:
		 *   add_command("commands", mpxcmd_commands); */

	/* R2 commands */
	add_command("suspend", mpxcmd_suspend);
	add_command("resume", mpxcmd_resume);
	add_command("renice", mpxcmd_renice);
	add_command("ps", mpxcmd_ps);

	/* R2 Temporary commands */
	add_command("create_pcb", mpxcmd_create_pcb);
	add_command("delete_pcb", mpxcmd_delete_pcb);
	add_command("block", mpxcmd_block);
	add_command("unblock", mpxcmd_unblock);
}
