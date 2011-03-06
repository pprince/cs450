/*!
 * @file	pcb.c
 * @brief	PCBs, process queues, and functions to operate on them
 * @author	Paul Prince <paul@littlebluetech.com>
 * @date	2011
 */


#include "pcb.h"
#include "mpx_supt.h"
#include "mpx_util.h"


static pcb_queue_t	queue_ready;
static pcb_queue_t	queue_blocked;
static pcb_queue_t	queue_susp_ready;
static pcb_queue_t	queue_susp_blocked;


/*! Must be called before using any other PCB or queue functions. */
void init_pcb_queues(void)
{
	queue_ready.head		= NULL;
	queue_ready.tail		= NULL;
	queue_ready.length		= 0;

	queue_blocked.head		= NULL;
	queue_blocked.tail		= NULL;
	queue_blocked.length		= 0;

	queue_susp_ready.head		= NULL;
	queue_susp_ready.tail		= NULL;
	queue_susp_ready.length		= 0;

	queue_susp_blocked.head		= NULL;
	queue_susp_blocked.tail		= NULL;
	queue_susp_blocked.length	= 0;
}


/*! Allocates memory for a new PCB, but does not initialize it.
 *
 * This function will also allocate memory for the stack and initialize
 * the stack_top and stack_base members.
 *
 * @return	Returns a pointer to the new PCB, or NULL if an error occured.
 */
pcb_t* allocate_pcb (void)
{
	/* Pointer to the new PCB we will allocate. */
	pcb_t *new_pcb;

	/* Allocate memory for the PCB. */
	new_pcb = (pcb_t *)sys_alloc_mem(sizeof(pcb_t));
	if ( new_pcb == NULL ) {
		/* Error allocating memory for the PCB. */
		return NULL;
	}

	/* Allocate memory for the PCB's stack. */
	new_pcb->stack_base = (unsigned char *)sys_alloc_mem(STACK_SIZE);
	if ( new_pcb->stack_base == NULL ) {
		/* Error allocating memory for the PCB's stack. */
		sys_free_mem(new_pcb);
		return NULL;
	}

	/* Initialize stack_top member. */
	new_pcb->stack_top = new_pcb->stack_base + STACK_SIZE;


	return new_pcb;
}


/*! Creates a new PCB object and initializes its fields.
 *
 * @return	Returns a pointer to the new PCB, or NULL if an error occured.
 */
pcb_t* setup_pcb (
	/*! Name of the new process. Must be unique among all processes. */
	char *name,
	/*! Priority of the process. Must be between -127 and 128 (incl.) */
	int priority,
	/*! Class of the process; one of APPLICATION or SYSTEM. */
	process_class_t class
)
{
	/* Loop index. */
	int i;

	/* Pointer to the new PCB we're creating. */
	pcb_t *new_pcb;

	/* Check that arguments are valid. */
	if ( find_pcb(name) != NULL ) {
		/* Name is not unique. */
		return NULL;
	}
	if ( strlen(name) > MAX_ARG_LEN || name == NULL ) {
		/* Invalid name. */
		return NULL;
	}
	if ( priority < -127 || priority > 128 ) {
		/* Value of priority is out of range. */
		return NULL;
	}
	if ( class != APPLICATION && class != SYSTEM ) {
		/* Invalid class specified. */
		return NULL;
	}


	/* Allocate the new PCB. */
	new_pcb = allocate_pcb();
	if (new_pcb == NULL) {
		/* Allocation error. */
		return NULL;
	}

	
	/* Set the given values. */
	new_pcb->priority	= priority;
	new_pcb->class		= class;
	strcpy( new_pcb->name, name );


	/* Set other default values. */
	new_pcb->state		= READY;
	new_pcb->memory_size	= 0;
	new_pcb->load_address	= NULL;
	new_pcb->exec_address	= NULL;

	/* Initialize the stack to 0's. */
	for (i=0; i<STACK_SIZE; i++) {
		*(new_pcb->stack_base + i) = (unsigned char)0;
	}

	return new_pcb;
}


/*! Search the given queue for the named process.
 *
 * @return Returns a pointer to the PCB, or NULL if not found or error.
 *
 * @private
 */
pcb_t* find_pcb_in_queue(
	/*! The name of the process to find. */
	char *name,
	/*! The PCB queue in which to search for the process. */
	pcb_queue_t *queue
)
{
	pcb_queue_node_t *this_queue_node = queue->head;

	while (this_queue_node != NULL) {
		if ( strcmp( this_queue_node->pcb->name, name) == 0 ) {
			return this_queue_node->pcb;
		}
	}

	/* If we get here, we didn't find the process. */
	return NULL;
}


/*! Finds a process.
 *
 * Searches all process queues.
 *
 * @return Returns a pointer to the PCB, or NULL if not found or error.
 */
pcb_t* find_pcb(
	/*! The name of the process to find. */
	char *name
)
{
	/* Pointer to the requested PCB, if we find it. */
	pcb_t *found_pcb;

	/* Validate arguments. */
	if ( name == NULL || strlen(name) > MAX_ARG_LEN ) {
		/* Invalid process name. */
		return NULL;
	}

	/* Search for the PCB.  If we find it, return it. */
	if ( found_pcb = find_pcb_in_queue( name, &queue_ready ) ) {
		return found_pcb;
	}
	if ( found_pcb = find_pcb_in_queue( name, &queue_blocked ) ) {
		return found_pcb;
	}
	if ( found_pcb = find_pcb_in_queue( name, &queue_susp_ready ) ) {
		return found_pcb;
	}
	if ( found_pcb = find_pcb_in_queue( name, &queue_susp_blocked ) ) {
		return found_pcb;
	}

	/* If we get here, the process was not found. */
	return NULL;
}
