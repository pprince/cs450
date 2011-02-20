#ifndef MPX_SH_H_GUARD
#define MPX_SH_H_GUARD


/** Defines the default prompt string for the MPX command-line user interface. */
#define MPX_DEFAULT_PROMPT	"MPX$  "

void mpx_shell(void);
void mpx_setprompt(char *new_prompt);

#endif
