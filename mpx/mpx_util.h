#ifndef MPX_UTIL_H_GUARD
#define MPX_UTIL_H_GUARD


/*! Defines the version of MPX. */
#define MPX_VERSION		"R1 (2011-02-21)"

/*! Defined the default directory for MPX executables. */
#define MPX_DEFAULT_EXE_DIR	"./exe/"

/*! Defines the maximum length for a single command-line. */
#define MAX_CMDLINE_LEN		255

/*! Defines the maximum length for a single argument to an MPX command (and for the command names, also). */
#define MAX_ARG_LEN		64

/*! Defines the maximum number of arguments to an MPX command. */
#define MAX_ARGS		16

/*! Defines the maximum length for file names in MPX; set to 12 for MS-DOS 8.3 names. */
#define MAX_FILENAME_LEN	12

int mpx_chomp ( char *str );
int mpx_validate_date ( int year, int month, int day );
int mpx_cat ( char *file_name );


#endif
