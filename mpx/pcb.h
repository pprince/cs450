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
typedef struct pcb_queue {

	/*! Pointer to the first element in the queue. */
	pcb_queue_node_t	*head;

	/*! Pointer to the last element in the queue. */
	pcb_queue_node_t	*tail;

	/*! Number of elements in the queue. */
	unsigned int		length;

	/*! Specifies how elements in this queue are sorted at insert-time. */
	pcb_queue_sort_order_t	sort_order;

} pcb_queue_t;


/* MACROS *
 * ------ */

/*! Provides syntactic sugar for looping over the elements of a linked list.
 *
 * This function makes it a little more readable when you want to loop over
 * elements in a linked list, starting with the head. Will work on both singly-
 * and doubly-linked lists.
 *
 * If you wish to stop processing early, before iterating through the entire
 * list, simply call  break as if you were in a \c for(;;){} or \c while()
 * loop.
 *
 * In order to use this function on your list, the following requirements must
 * be satisfied:
 *
 * <ul>
 * 	<li> You must declare the variable you pass as \c item yourself.
 * 	<li> The \c list parameter must be a pointer to a struct that has a
 *		member named \c head that is a pointer to the first item in
 *		the list.
 *	<li> In the case that the list is empty (i.e., contains zero elements),
 *		then \c list->head must point to NULL.
 *	<li> The \c item parameter <em>and</em> the \c list->head member must
 *		both be pointers to structs of the same type, and,
 *	<li> That struct must have a member named \c next that is a pointer to
 *		the next item in the list.
 *	<li> The \c next member of the last item in the list \em must point to
 *		NULL.
 * </ul>
 *
 * And also, while the following rules may not be strict requirements, it is
 * \em strongly encouraged that you adhere to them:
 *
 * <ul>
 *	<li> If, in a given execution of the loop body, you modify the list by
 *		adding, removing, moving, any list items, 
 *		you should  break out of the loop;
 *		<em>you should not,</em> having so-modified the list, continue
 *		on to the next iteration / execution of the loop body.
 *	<li> You should not modify the value of \c item inside the loop body.
 * </ul>
 *
 * Note that you're free to modify the <em>items</em>, just not the
 * <em>list</em>; so,
 * as long as you do not modify the values of any item's \c next member, you
 * are free to modify any other members.
 *
 * In particular, this function <em>is</em> compatible with the \c pcb_queue_t
 * and \c pcb_queue_node_t types.
 *
 * @param [out]	item	Iterator variable / loop index; will point to the
 * 			current item (node) just before each execution of the
 * 			loop body.
 * @param [in]	list	The singly- or doubly-linked list to iterate over.
 *
 * @return	Does <em>not</em> have a return value in the typical sense,
 * 		however the value of the ouput parameter \c item is
 * 		well-defined after the loop has terminated:
 *			<ul>
 * 	<li>If the loop terminates on its own, after iterating over the entire
 * 		list, \c item will be NULL.
 *	<li>Note that an empty list is a special case of the above, and in
 *		that case the value of \c item will be NULL after the loop
 *		has terminated, but the loop body will never have been
 *		executed.
 * 	<li>If you  break out of the loop before it terminates on its
 *		own, \c item will point to the list item that was being
 *		processed during the iteration of the loop in which \c break
 *		was called,
 *		<em>even if that item is the last item in the list.</em>
 *			</ul>
 */ 
#define foreach_listitem( item, list ) \
	for ( item = list->head; item != NULL; item = item->next )

#define foreach_listitem_rev( item, list, rev ) \
	for ( \
		(item) = (rev) ? (list)->tail : (list)->head; \
		(item) != NULL; \
		(item) = (rev) ? (item)->prev : (item)->next \
	)



/* EXTERNS *
 * ------- */
extern pcb_queue_t   *queues[];



/* FUNCTIONS
 * --
 *  These are documented in the .c file.
 * --
 */

void		init_pcb_queues		( void );
pcb_queue_t*	get_queue_by_state	( process_state_t state );
pcb_t*		setup_pcb   ( char *name, int priority, process_class_t class );
pcb_t*		find_pcb		( char *name );
pcb_queue_t*	remove_pcb		( pcb_t *pcb );
pcb_queue_t*	insert_pcb		( pcb_t *pcb );
int		block_pcb		( pcb_t *pcb );
int		unblock_pcb		( pcb_t *pcb );
int		suspend_pcb		( pcb_t *pcb );
int		resume_pcb		( pcb_t *pcb );
int		is_blocked		( pcb_t *pcb );
int		is_suspended		( pcb_t *pcb );
int		is_ready		( pcb_t *pcb );
char*		process_state_to_string	( process_state_t state );
char*		process_class_to_string	( process_class_t class );
char		process_class_to_char	( process_class_t class );


#endif
