#ifndef PCB_H_GUARD
#define PCB_H_GUARD


/*!
 * @file	pager.h
 * @brief	Provides a pager feature to MPX, like the Unix \c more command
 * @author	Paul Prince <paul@littlebluetech.com>
 * @date	2011
 *
 * @ingroup	pager
 */

/*! @addtogroup pager
 *  @{  */


/*! Defines the number of text rows on the MPX screen. */
#define SCREEN_ROWS 25

/*! Defines the number of text columns on the MPX screen. */
#define SCREEN_COLS 80


/*! @} */


void pager_init (void);
void pager_stop (void);
int pager_printf (const char *format, ...);


#endif
