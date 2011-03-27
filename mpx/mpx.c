
/*! @file	mpx.c
 *  @brief	MPX main() function
 *  @author	Paul Prince <paul@littlebluetech.com>
 *  @date	2011
 *
 * This file contains the start-of-execution, i.e. function main(), for MPX,
 * and also the top-level Doxygen documentation that becomes the introductory
 * sections of the developer's manual.
 */


#include "mpx_supt.h"
#include "mpx_util.h"
#include "mpx_sh.h"
#include "mpx_cmds.h"
#include "pcb.h"


/*! This is the start-of-execution for the MPX executable. */
void main(int argc, char *argv[])
{
	/* System-specific initialization, provided by support software. */
	sys_init( MODULE_R2 );

	/* Initialization for MPX user commands. */
	init_commands();

	/* Initialization for PCB queues. */
	init_pcb_queues();

	/* Execute the command-handler loop. */
	mpx_shell();

	/* mpx_shell() should never return, so if we get here, then
	 * we should exit with error status (but don't actually...). */
	printf("FATAL ERROR: mpx_shell() returned! That shouldn't happen...\n");
	sys_exit();	/* Terminate, after doing MPX-specific cleanup. */
}
