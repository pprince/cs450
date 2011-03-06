#ifndef MPX_CMDS_H_GUARD
#define MPX_CMDS_H_GUARD

#include "pcb.h"
extern pcb_queue_t *queues[];

/*! Node type for a singly-linked list of MPX commands. */
struct mpx_command {
	char *name;
	void (*function)(int argc, char *argv[]);
	struct mpx_command *next;
};

void init_commands(void); 
void add_command( char *name, void (*function)(int argc, char *argv[]) );
void dispatch_command( char *name, int argc, char *argv[] );

void mpxcmd_commands( int argc, char *argv[] );


#endif
