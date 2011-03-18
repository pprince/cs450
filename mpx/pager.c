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

#include <stdio.h>


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
 * 	Returns the number of bytes written, or EOF to indicate that and
 * 	error occurred.
 */
int pager_printf (const char *format, ...)
{
	va_list args;
	va_start(args, format);

	/* Pass the format string and the rest of the args onto vprintf. */
	vprintf(format, args);

	va_end(args);
}


/*! @} */
