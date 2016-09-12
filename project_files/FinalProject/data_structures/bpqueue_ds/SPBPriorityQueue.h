#ifndef SPBPRIORITYQUEUE_H_
#define SPBPRIORITYQUEUE_H_
#include "SPListElement.h"
#include <stdbool.h>
/**
 * SP Bounded Priority Queue summary
 *
 * Implements a Priority Queue type.
 * The queue size is limited by an integer called capacity.
 * The elements of the queue are stored in a SPList type, while keeping
 * the invariant of - the list is ordered (first item is smallest)
 * The queue supports storing similar items, and as the
 * enqueue action copy's the content of the given item, the internal order of identical items is not relevant
 * The type also stores a pointer to the last element in the queue, which is also the largest one
 * if the queue is empty, the pointer is NULL
 *
 * The following functions are available:
 *
 *   spBPQueueCreate            - Creates a new empty queue, limited to the given maximum size
 *   spBPQueueDestroy           - Deletes an existing queue and frees all resources
 *   spBPQueueCopy              - Copies an existing queue, with the same capacity
 *   spBPQueueClear             - Removes all the elements from the queue
 *   spBPQueueSize              - Returns the size of a given queue
 *   spBPQueueGetMaxSize        - Returns the maximum size limit (capacity) of the given queue
 *   spBPQueueEnqueue           - Inserts a new item to the queue, the inserted item is a copy of the given one,
 *                                the item would not be inserted if it is larger than the maximum
 *                                item of the queue, and the queue is at full capacity
 *   spBPQueueDequeue           - Removes the minimal item from the queue
 *   spBPQueuePeek              - Returns a copy of the minimal item in the queue
 *   spBPQueuePeekLast          - Returns a copy of the maximal item in the queue
 *   spBPQueueMinValue          - Returns the value of the minimal item in the queue
 *   spBPQueueMaxValue          - Returns the value of the maximal item in the queue
 *   spBPQueueIsEmpty           - Returns true if and only if the queue is empty
 *   spBPQueueIsFull            - Returns true if and only if the queue is full
 */


/** type used to define Bounded priority queue **/
typedef struct sp_bp_queue_t* SPBPQueue;

/** type for error reporting **/
typedef enum sp_bp_queue_msg_t {
	SP_BPQUEUE_OUT_OF_MEMORY,
	SP_BPQUEUE_FULL,
	SP_BPQUEUE_EMPTY,
	SP_BPQUEUE_INVALID_ARGUMENT,
	SP_BPQUEUE_SUCCESS
} SP_BPQUEUE_MSG;

/**
 * Allocates a new queue.
 * This function creates a new empty queue.
 * @param maxSize - a limit for the size of the queue
 * @return
 * 	NULL - If allocations failed or maxSize <= 0
 * 	A new queue in case of success.
 *
 * @logger - the method logs allocation and arguments errors if needed
 */
SPBPQueue spBPQueueCreate(int maxSize);

/**
 * Creates a copy of target queue.
 *
 * The new copy will contains the same capacity limit and a copy of the queue items
 *
 * @param source The target source to copy
 * @return
 * NULL if a NULL was sent or a memory allocation failed.
 * A queue containing the same capacity and the same queue items.
 *
 * @logger - the method logs allocation and arguments errors if needed
 */
SPBPQueue spBPQueueCopy(SPBPQueue source);

/**
 * Deallocates an existing queue. Clears all elements by using the
 * stored free function.
 *
 * @param source - queue to be deallocated. If list is NULL nothing will be
 * done
 */
void spBPQueueDestroy(SPBPQueue source);

/**
 * Removes all elements from the queue.
 *
 * The elements are deallocated using the list clear items
 * @param source -  Target queue to remove all element from
 * does nothing if source is NULL or the source internal list is NULL
 */
void spBPQueueClear(SPBPQueue source);

/**
 * Returns the number of elements in the queue.
 *
 * @param source - The target which size is requested.
 * @return
 * -1 if a NULL pointer was sent or in case of internal NULL pointer
 * Otherwise the number of elements in the queue.
 *
 * @logger - the method logs arguments errors if needed
 */
int spBPQueueSize(SPBPQueue source);

/**
 * Returns the maximum limit (capacity) of elements in the queue.
 *
 * @param source - The target which max size is requested.
 * @return
 * -1 if a NULL pointer was sent
 * Otherwise the capacity of the queue.
 *
 * @logger - the method logs arguments errors if needed
 */
int spBPQueueGetMaxSize(SPBPQueue source);

/**
 * Insert a new item to the queue
 * at the suitable place, while keeping the internal list sorted
 * without violating the capacity limit
 *s
 * @param source - The target which the enqueue is requested on.
 * @param element - the element to insert to the queue
 *
 * @return
 * 	SP_BPQUEUE_OUT_OF_MEMORY - in case of memory allocation error
 *	SP_BPQUEUE_FULL - in case the queue is at full capacity and the requested
 *					  element is greater than all the elements in the queue or equal
 *					  to maximal element in the queue.
 *	SP_BPQUEUE_INVALID_ARGUMENT - in case source is NULL or element is NULL
 *	SP_BPQUEUE_SUCCESS - in case the element was successfully inserted to the queue
 *
 *	@logger - the method logs allocation and arguments errors if needed
 */
SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element);

/**
 * Removes the minimal item from the queue
 *
 * @param source - The target which the dequeue is requested on.
 *
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if source is NULL
 * SP_BPQUEUE_EMPTY if the queue if empty
 * SP_BPQUEUE_SUCCESS the element has been removed successfully
 *
 * @logger - the method logs allocation and arguments errors if needed
 */
SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source);

/**
 * The method is used to get a copy of the first (minimum) item in the queue.
 * @param source - The target which the check is requested on.
 * @return
 * NULL if source is NULL or queue is empty, otherwise returns the
 * a hard copy of the first (minimum) item in the queue.
 */
SPListElement spBPQueuePeek(SPBPQueue source);

/**
 * The method is used to get a copy of the last (maximum) item in the queue.
 * @param source - The target which the check is requested on.
 * @return
 * NULL if source is NULL or queue is empty, otherwise returns the
 * a hard copy of the last (maximum) item in the queue.
 *
 * @logger - the method logs arguments errors if needed
 */
SPListElement spBPQueuePeekLast(SPBPQueue source);

/**
 * The method is used to get the minimum value of the items in the queue.
 * @param source - The target which the check is requested on.
 * @return
 * -1 if source is NULL or queue is empty, otherwise returns the
 * minimum value of the items in the queue.
 *
 * @logger - the method logs arguments errors if needed
 */
double spBPQueueMinValue(SPBPQueue source);

/**
 * The method is used to get the maximum value of the items in the queue.
 * @param source - The target which the check is requested on.
 * @return
 * -1 if source is NULL or queue is empty, otherwise returns the
 * maximum value of the items in the queue.
 *
 * @logger - the method logs arguments errors if needed
 */
double spBPQueueMaxValue(SPBPQueue source);

/**
 * Check if the queue is empty.
 * Assert source != NULL
 * @param source - The target which the check is requested on.
 * @return
 * true iff the queue is empty
 *
 */
bool spBPQueueIsEmpty(SPBPQueue source);

/**
 * Check if the queue is at full capacity.
 * Assert source != NULL
 * @param source - The target which the check is requested on.
 * @return
 * true iff the queue is at full capacity
 */
bool spBPQueueIsFull(SPBPQueue source);


/**
 * Allocates a new queue.
 * This function creates a new empty queue,
 * Using a flag regarding the creation of the internal data list, if the flag is on
 * the method will create a new list, otherwise it will use a copy of the given source queue
 * @param maxSize - a limit for the size of the queue
 * @param source_queue - the given queue, this parameter should be NULL if the createNewList flag is on
 * @param createNewList - a flag used to indicate whether to create the internal list
 * @return
 * 	NULL - If allocations failed or maxSize <= 0 or createNewList flag is off and source_queue is NULL
 * 	A new queue in case of success, with respect to the createNewList flag
 *
 * 	@logger - the method logs allocation and arguments errors if needed
 */
SPBPQueue spBPQueueCreateWrapper(int maxSize, SPBPQueue source_queue, bool createNewList);

/*
 * The method gets a queue and an element and inserts the element to the queue
 * Pre assumptions:
 *  source is empty, source != NULL , newElement != NULL
 * @param source - the given queue to work on
 * @param newElement - the given element to insert
 * @return
 * SP_BPQUEUE_OUT_OF_MEMORY - in case of allocation error
 * SP_BPQUEUE_SUCCESS - in case the new is inserted correctly
 */
SP_BPQUEUE_MSG spBPQueueInsertIfEmpty(SPBPQueue source, SPListElement newElement);

/*
 * The method removes the last element in the queue (to prevent capacity overflow)
 * The method is given a queue
 * it iterate over the list items, removes the last one (the maximum) and update the current maximum
 * Pre-assumptions
 *  	source != NULL, list is at full capacity
 * @param source - the given queue from which we should remove the last item
 * @return
 * 	SP_BPQUEUE_OUT_OF_MEMORY - in case of memory error
 *  SP_BPQUEUE_SUCCESS - in case the item was successfully inserted to the queue
 */
SP_BPQUEUE_MSG spBPQueueHandleFullCapacity(SPBPQueue source);

/*
 * The method inserts an item to the queue before a given item,
 * assuming the insertion is not at the end of the queue and the queue is not empty
 * Pre-assumptions:
 * source != NULL and source->list not empty and element != NULL
 * and the current iterator of the internal list in the queue is not null
 *  @param source - the given queue from which we should insert the item
 *  @param element - the element we should insert to the queue
 *  @return
 *  SP_BPQUEUE_OUT_OF_MEMORY - in case of memory error
 *  SP_BPQUEUE_SUCCESS - in case the item was successfully inserted to the queue
 */
SP_BPQUEUE_MSG spBPQueueInsertNotEmptyNotLast(SPBPQueue source,
		SPListElement element);

/*
 * The method inserts an item to the queue at its end,
 * assuming the queue is not empty
 * Pre-assumptions:
 * - source != NULL and source->list not empty and not at (full capacity - 1) and element != NULL
 *  @param source - the given queue to which we should insert the item
 *  @param element - the element we should insert to the queue
 *  @return
 *  SP_BPQUEUE_OUT_OF_MEMORY - in case of memory error
 *  SP_BPQUEUE_SUCCESS - in case the item was successfully inserted to the queue
 */
SP_BPQUEUE_MSG spBPQueueInsertNotEmptyButLast(SPBPQueue source, SPListElement element);

/*
 * The method inserts an item to the queue,
 * assuming the queue is not empty
 * Pre-assumptions:
 * - source != NULL and source->list not empty and newElement != NULL
 *  @param source - the given queue to which we should insert the item
 *  @param newElement - the element we should insert to the queue
 *  @return
 *  SP_BPQUEUE_OUT_OF_MEMORY - in case of memory error
 *  SP_BPQUEUE_SUCCESS - in case the item was successfully inserted to the queue
 */
SP_BPQUEUE_MSG spBPQueueInsertNotEmpty(SPBPQueue source, SPListElement newElement);

/*
 * The method returns an element value, given a queue and a
 * function pointer that extract some element from it
 * Pre Assumptions - if the source is not NULL than func(source) != NULL
 * @param source - the given queue to extract the data from
 * @param func - a function pointer that given a queue, extract some element from it
 * @return
 * DEFAULT_INVALID_DOUBLE if source is NULL, otherwise the func(source) value.
 *
 * @logger - the method logs allocation and arguments errors if needed
 */
double returnValueFrom(SPBPQueue source, SPListElement (*func)(SPBPQueue));
#endif
