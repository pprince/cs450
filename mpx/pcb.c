/*!
 * @file	pcb.c
 * @brief	PCBs, process queues, and functions to operate on them
 * @author	Paul Prince <paul@littlebluetech.com>
 * @date	2011
 */


#include "pcb.h"
#include "mpx_supt.h"
#include "mpx_util.h"


static	pcb_queue_t	queue_ready;
static	pcb_queue_t	queue_blocked;
static	pcb_queue_t	queue_susp_ready;
static	pcb_queue_t	queue_susp_blocked;


/*! Extern variable that allows other files to directly access PCB queues.
 *
 * @todo  We really need to replace this with some various
 * get_queue() type functions!
 */
pcb_queue_t	*queues[4];


/*! Must be called before using any other PCB or queue functions. */
void init_pcb_queues(void)
{
	queues[0] = &queue_ready;
	queue_ready.head		= NULL;
	queue_ready.tail		= NULL;
	queue_ready.length		= 0;
	queue_ready.sort_order		= PRIORITY;

	queues[1] = &queue_blocked;
	queue_blocked.head		= NULL;
	queue_blocked.tail		= NULL;
	queue_blocked.length		= 0;
	queue_blocked.sort_order	= FIFO;

	queues[2] = &queue_susp_ready;
	queue_susp_ready.head		= NULL;
	queue_susp_ready.tail		= NULL;
	queue_susp_ready.length		= 0;
	queue_susp_ready.sort_order	= PRIORITY;

	queues[3] = &queue_susp_blocked;
	queue_susp_blocked.head		= NULL;
	queue_susp_blocked.tail		= NULL;
	queue_susp_blocked.length	= 0;
	queue_susp_blocked.sort_order	= FIFO;
}


/*! References the PCB queue appropriate for processes of a given state.
 *
 * Note that RUNNING is <em>not</em> a valid value for passing as the \c state
 * parameter, since running processes do not belong in <em>any</em> queue.
 *
 * @return	Returns either a pointer to a valid PCB queue that should hold
 * 		processes of the given state, or NULL on error.
 */
pcb_queue_t* get_queue_by_state (
	/* [in] One of the valid process states (except RUNNING). */
	process_state_t state
) {
	switch (state) {
		case READY:
			return &queue_ready;
		break;
		case BLOCKED:
			return &queue_blocked;
		break;
		case SUSP_READY:
			return &queue_susp_ready;
		break;
		case SUSP_BLOCKED:
			return &queue_susp_blocked;
		break;
		case RUNNING:
			/* RUNNING processes don't go in *any* queue. */
			return NULL;
		break;
		default:
			/* Totally Unexpected value for process state. */
			return NULL;
		break;
	}
}


/*! Allocates memory for a new PCB, but does not initialize it.
 *
 * This function will also allocate memory for the PCB's stack, and initialize
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


/*! De-allocates the memory that was used for a PCB. */
void free_pcb (pcb_t *pcb)
{
	sys_free_mem(pcb->stack_base);
	sys_free_mem(pcb);
}


/*! Creates, allocates, and initializes a new PCB object.
 *
 * This function creates a new PCB object (pcb_t), then calls allocate_pcb() to
 * do the allocation step. It then initializes the PCB's various fields
 * according to both default values and the parameters passed in.
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
		this_queue_node = this_queue_node->next;
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

	/* Loop index. */
	int i;

	/* Validate arguments. */
	if ( name == NULL || strlen(name) > MAX_ARG_LEN ) {
		/* Invalid process name. */
		return NULL;
	}

	/* Search each queue for the PCB; if we find it, return it: */
	for ( i=0; i<4; i++ ){

		/* Search this specific queue for the PCB: */
		found_pcb = find_pcb_in_queue( name, queues[i] );

		if ( found_pcb ){
			/* We found it, so return it: */
			return found_pcb;
		}
	}

	/* If we get to this point, the process was not found in any queue.
	 * ("Sorry Mario, your PCB is in another castle!") */
	return NULL;
}


/*! Removes a PCB from its queue.
 *
 * Given a pointer to a valid and en-queued PCP, this function will remove
 * that PCB from the queue that it is in.
 *
 * However, this function will <em>not</em> modify the state member of the PCB;
 * the caller is responsible for doing that, if the PCB is to be re-enqueued
 * rather than de-allocated.
 *
 * @return
 * 	Returns a pointer to the queue the PCB was removed from,
 * 	or NULL if an error occurred.
 */
pcb_queue_t* remove_pcb (
	/*! Pointer to the PCB to be de-queued. */
	pcb_t *pcb
)
{
	/* Loop index / iterator. */
	pcb_queue_node_t* this_node;

	/* The queue we will soon try to remove the given PCB from. */
	pcb_queue_t* queue = NULL;

	/* Validate argument. */
	if ( pcb == NULL ){
		/* ERROR: Got NULL pointer for argument. */
		return NULL;
	}

	/* Fetch the queue that we will be removing this process from. */
	queue = get_queue_by_state( pcb->state );

	/* Validate queue. */
	if ( queue == NULL ){
		/* ERROR: PCB seems to have invalid state assigned... */
		return NULL;
	}

	foreach_listitem( this_node, queue ){
		if ( this_node->pcb == pcb ){

			/* We've found our queue node.  Remove it:
			 * --------------------------------------- */

			/* Fix forward links and head: */
			if ( queue->head == this_node ){
				queue->head = this_node->next;
			} else {
				this_node->prev->next = this_node->next;
			}

			/* Fix backward links and tail: */
			if ( queue->tail == this_node ){
				queue->tail = this_node->prev;
			} else {
				this_node->next->prev = this_node->prev;
			}

			/* Adjust queue's node count: */
			queue->length--;

			/* And, de-allocate the queue descriptor (aka node):
			 * (with check for error.) */
			if ( sys_free_mem(this_node) != 0 ){
				/* ERROR: failure freeing memory...
				 *   Maybe we should just let this one slide,
				 *   (as failure to free memory is not an
				 *   immediately-fatal condition...),
				 *   But for now, err on the side of caution. */
				return NULL;
			}

			return queue;
		}
	}

	/* If, at this point, this_node is NULL, it means we didn't
	 * find the PCB in the queue where it should have been... so,
	 * ERROR: PCB wasn't found in the queue where it was expected. */
	return NULL;
}


/*! Inserts a PCB into the appropriate queue.
 *
 * Inspects the PCB's state member to determine which queue to insert into.
 *
 * Inspects the queue's sort_order member to determine whether to insert in
 * order of priority, or to simply insert the PCB at the end of of the queue.
 *
 * @return
 * 	Returns a pointer to the queue the PCB was inserted into,
 * 	or NULL if an error occurred.
 */
pcb_queue_t* insert_pcb (
	/*! Pointer to the PCB to be enqueued. */
	pcb_t *pcb
)
{
	/* Pointer to the queue we will insert into. */
	pcb_queue_t		*queue;
	/* Pointer to the new queue node descriptor we must make. */
	pcb_queue_node_t	*new_queue_node;
	/* For use in loops that iterating through the queue. */
	pcb_queue_node_t	*iter_node;

	/* Validate argument */
	if (pcb == NULL) {
		/* PCB to insert cannot be null... come on :) */
		return NULL;
	}

	/* Determine which queue we will insert this PCB into. */
	switch (pcb->state) {
		case READY:
			queue = &queue_ready;
		break;
		case BLOCKED:
			queue = &queue_blocked;
		break;
		case SUSP_READY:
			queue = &queue_susp_ready;
		break;
		case SUSP_BLOCKED:
			queue = &queue_susp_blocked;
		break;
		default:
			/* Unexpected value for PCB state (maybe Running?) */
			return NULL;
		break;
	}

	/* Allocate the new queue descriptor. */
	new_queue_node =
		(pcb_queue_node_t *)sys_alloc_mem(sizeof(pcb_queue_node_t));
	if ( new_queue_node == NULL ){
		/* Error allocating memory. */
		return NULL;
	}


	/* Do the insert ... */
	/* ----------------- */
	
	new_queue_node->pcb = pcb;

	/* Case one: queue is empty. */
	if ( queue->length == 0 ){
		new_queue_node->next	= NULL;
		new_queue_node->prev	= NULL;
		queue->head		= new_queue_node;
		queue->tail		= new_queue_node;
		queue->length		= 1;
		return queue;
	}

	/* Case two: FIFO queue; we only need to insert at end. */
	if ( queue->sort_order == FIFO ){
		goto INSERT_AT_END;
	}

	/* The hard case: insert in priority-order. */
	iter_node = queue->head;
	while (iter_node != NULL) {
		if ( iter_node->pcb->priority < pcb->priority ){
			/* Insert before iter_node */
			new_queue_node->prev = iter_node->prev;
			iter_node->prev->next = new_queue_node;
			iter_node->prev = new_queue_node;
			new_queue_node->next = iter_node;
			if ( queue->head == iter_node ){
				queue->head = new_queue_node;
			}
			queue->length++;
			return queue;
		}
		iter_node = iter_node->next;
	}
	/* If we got this far, we need to do an insert-at-the-end. */


	INSERT_AT_END:
		new_queue_node->next	= NULL;
		new_queue_node->prev	= queue->tail;
		queue->tail->next	= new_queue_node;
		queue->tail		= new_queue_node;
		queue->length++;
		return queue;
}

