
/*! @file	mpx.c
 *  @brief	MPX main() function
 *  @author	Paul Prince <paul@littlebluetech.com>
 *  @date	2011
 *
 * This file contains the start-of-execution, i.e. function main(), for MPX,
 * and also the top-level Doxygen documentation that becomes the introductory
 * sections of the developer's manual.
 */


/*! @mainpage Introduction
 *
 *
 * 	@section	Code Repository
 *
 * Version-control information is managed by Git, and hosted by GitHub:
 * 	https://github.com/pprince/cs450
 *
 *
 *	@section 	Documentation
 *
 * Documentation for developers is generated by Doxygen; for detailed
 * information about the files, functions, data structures, etc. that
 * make up MPX and how they relate to each other, refer to:
 *
 * - "MPX Programmer's Manual"
 *
 * which can be found in the doc/ directory. Also, in the same directory,
 * you can find the current version of:
 *
 * - "MPX User's Manual"
 *
 *
 *
 *  @todo
 * 	Generally, documentation is incomplete.
 *
 *  @todo
 *  	Generally, we need to make lines break cleanly at 80-columns;
 *  	Doxygen forces such line-breaks on us in the LaTeX output, but our
 *  	source code frequently uses longer lines (making the PDF version of
 *  	the developer manual very ugly!
 */


#include "mpx_supt.h"
#include "mpx_util.h"
#include "mpx_sh.h"
#include "mpx_cmds.h"
#include "mpx_pcb.h"


/*! This is the start-of-execution for the MPX executable. */
void main(int argc, char *argv[])
{
	sys_init( MODULE_R1 );	/* System-specific initialization.       */

	init_commands();	/* Initialization for MPX user commands. */
	init_pcb_queues();	/* Initialization for PCB queues.	 */s

	mpx_shell();		/* Execute the command-handler loop.     */

	/* mpx_shell() should never return, so if we get here, then
	 * we should exit with error status (but don't actually...). */
	printf("FATAL ERROR: mpx_shell() returned! That shouldn't happen...\n");
	sys_exit();	/* Terminate, after doing MPX-specific cleanup. */
}
