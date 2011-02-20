#ifndef MPX_UTIL_H_GUARD
#define MPX_UTIL_H_GUARD

/** Defines the maximum length for a single command-line. */
#define MAX_CMDLINE_LEN		255

/** Defines the maximum length for a single argument to an MPX command. */
#define MAX_ARG_LEN		64

/** Defines the maximum number of arguments to an MPX command. */
#define MAX_ARGS		16

int mpx_chomp ( char *str );

#endif
