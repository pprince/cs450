
/** \file	mpx_util.c
 *  \brief	Various utility functions used by all of MPX.
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

#include "mpx_util.h"
#include "mpx_supt.h"
#include <string.h>

/** Removes trailing newline, if any.
 *
 * This function checks to see if the last character in a string is a newline, and, if so, removes it.
 * Otherwise, the string is left unchanged.
 *
 * The input must be a valid (allocated and null-terminated) C string, otherwise the results are undefined
 * (but will most likley result in a segmentation fault / protection fault).
 *
 * Returns the number of characters removed from the string.
 */
int mpx_chomp ( char *str /**< The string to chomp. */ ){
	if( strlen(str) > 0 ){
		if( str[ strlen(str)-1 ] == '\n' ){
			str[ strlen(str)-1 ] = '\0';
			return 1;
		}
	}
	return 0;
}


int mpx_validate_date ( int year, int month, int day ) {

	int days_in_month;
	int is_leapyear;

	/* If year is invalid, return false. */
	if ( year<1900 || year > 2999 ) {
		return 1;
	}

	/* Is the specified year a leap year? */
	is_leapyear = ((year%4==0 && year%100!=0)||(year%400==0));

	/* Determine how many days are in the specified month. */
	/* Also, if month is invalid (i.e., not 1-12 incl.), return false. */
	switch (month) {
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			days_in_month = 31;
		break;

		case 4: case 6: case 9: case 11:
			days_in_month = 30;
		break;

		case 2:
			if ( is_leapyear ) {
				days_in_month = 29;
			} else {
				days_in_month = 28;
			}
		break;

		default:
			return 1;
		break;
	}

	/* If day is invalid, return false. */
	if ( day > days_in_month ) return 1;

	/* If we get this far, then the date is indeed valid; return true. */
	return 0;
}
