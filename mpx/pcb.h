#ifndef PCB_H_GUARD
#define PCB_H_GUARD

/*!
 * @file	pcb.h
 * @brief	PCBs, process queues, and functions to operate on them
 * @author	Paul Prince <paul@littlebluetech.com>
 * @date	2011
 */


#include "mpx_util.h"


/*! Amount of stack space to allocate for each process (in bytes). */
#define STACK_SIZE		1024


/*! Type for variables that hold the state of a process. */
typedef enum {

	RUNNING,
	READY,
	BLOCKED,
	SUSP_READY,
	SUSP_BLOCKED

} process_state_t;


/*! Type for variables that hold the class of a process. */
typedef enum {

	APPLICATION,
	SYSTEM

} process_class_t;


/*! Process control block structure */
typedef struct {

	/*! Name of the process (i.e., its argv[0] in unix-speak). */
	char			name[MAX_ARG_LEN+1]; 

	/*! Process class (differentiates applications from system processes. */
	process_class_t		class;

	/*! Process priority. Higher numerical value = higher priority.
	 *
	 * Valid values are -128 through 127 (inclusive). */
	int			priority;

	/*! Process state (Ready, Running, or Blocked). */
	process_state_t		state;

	/*! Pointer to the top of this processes's stack. */
	unsigned char		*stack_top;

	/*! Pointer to the bottom of this processes's stack. */
	unsigned char		*stack_base;

	/*! Memory size ... will be used in R3 and R4. */
	int			memory_size;

	/*! Load address ... will be used in R3 and R4. */
	unsigned char		*load_address;

	/*! Execution address ... will be used in R3 and R4. */
	unsigned char		*exec_address;

} pcb_t;


/*! Enum constants for process sort order (i.e., queue insertion order.) */
typedef enum {

	FIFO,
	PRIORITY

} pcb_queue_sort_order_t;


/* PCB queue node; points to a single PCB that is associated with this node. */
typedef struct pcb_queue_node {

	/*! Pointer to the next PCB node in the queue. */
	struct pcb_queue_node	*next;

	/*! Pointer to the previous PCB node in the queue. */
	struct pcb_queue_node	*prev;

	/*! Pointer to the actual PCB associated with this node. */
	pcb_t			*pcb;

} pcb_queue_node_t;


/*! PCB queue; represents a queue of processes. */
typedef struct {

	/*! Pointer to the first element in the queue. */
	pcb_queue_node_t	*head;

	/*! Pointer to the last element in the queue. */
	pcb_queue_node_t	*tail;

	/*! Number of elements in the queue. */
	unsigned int		length;

	/*! Specifies how elements in this queue are sorted at insert-time. */
	pcb_queue_sort_order_t	sort_order;

} pcb_queue_t;


/* FUNCTIONS
 * --
 *  These are documented in the .c file.
 * --
 */

void init_pcb_queues ( void );
pcb_t* setup_pcb ( char *name, int priority, process_class_t class );
pcb_t* find_pcb ( char *name );
pcb_queue_t* insert_pcb ( pcb_t *pcb );


#endif
