/*!
 * @file	pager.c
 * @brief	Provides a pager feature to MPX, like the Unix \c more command
 * @author	Paul Prince <paul@littlebluetech.com>
 * @date	2011
 *
 * @ingroup	pager
 */

/*!
 * @defgroup	pager
 * 
 * @brief	Brief description of pager group.
 *
 * Detailed description of pager group.
 *
 */

/*! @addtogroup	pager
 *  @{  */


#include "pager.h"
#include "mpx_supt.h"
#include <stdio.h>


/*!
 */
void end_of_page_prompt (void)
{
	char buf[5];
	int buf_size=4;
	int retval;

	printf("<<_ PRESS [ RETURN ] for more output _>>");

	retval = sys_req( READ, TERMINAL, buf, &buf_size );
	if ( retval < 0 ) {
		printf("ERROR: sys_req() threw error while trying to read ");
		printf("from the terminal!\n");
		return;
	}
}

/*! Keeps track of how many rows have been printed on the current screen.
 *
 * Note that this is a file-static variable, and thus is only accessible
 * inside the \c pager.c file.
 */
static rows_printed = 0;


/*! This function is called before the first line of paged output is printed.
 */
void pager_init (void)
{
	/* Reset the rows_printed accumulator to begin a paged output afresh. */
	rows_printed = 0;
}


/*! This function is called before the last line of paged output is printed.
 */
void pager_stop (void)
{
	/* Currently a no-op? */
	return;
}


/*! This function replaces \c printf() when paged output is desired.
 *
 * Use only this function for output to the screen between calls to
 * \c pager_init() and \c pager_stop(). Writing to the terminal with any
 * other routine/method while paging will cause the output to be garbled,
 * or lines to be missed.
 *
 * This function makes use of two ANSI-standard C features: variable-length
 * argument lists (va_list), and vprintf.
 *
 * @return
 * 	Returns the number of bytes output to the screen,
 * 	or EOF to indicate that and error occurred.
 */
int pager_printf (const char *format, ...)
{
	int bytes_written;

	va_list args;
	va_start(args, format);

	/* Pass the format string and the rest of the args onto vprintf. */
	bytes_written = vprintf(format, args);

	va_end(args);

	rows_printed++;

	if ( (rows_printed % (SCREEN_ROWS-1)) == 0 ){
		rows_printed = 0;
	}

	return bytes_written;
}


/*! @} */
