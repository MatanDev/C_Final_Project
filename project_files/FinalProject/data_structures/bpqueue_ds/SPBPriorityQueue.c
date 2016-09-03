#include "SPBPriorityQueue.h"
#include "SPList.h"
#include <stdlib.h>
#include <assert.h>
#include "../../general_utils/SPUtils.h"

#define DEFAULT_INVALID_NUMBER -1

/*
 * A structure used in order to handle the queue data type
 * maxElement - a list element representing the maximum element in the queue
 * capacity - an integer representing a size limit for the queue
 * queue - a list used to store the queue items
 */
typedef struct sp_bp_queue_t {
	SPListElement maxElement;
	int capacity;
	SPList queue;
} sp_bp_queue_t;



SPBPQueue spBPQueueCreateWrapper(int maxSize, SPBPQueue source_queue, bool createNewList) {
	SPBPQueue newQueue;
	spMinimalVerifyArgumentsRn(maxSize > 0);

	spCalloc(newQueue, sp_bp_queue_t, 1);

	newQueue->capacity = maxSize;


	if (createNewList) {
		newQueue->queue = spListCreate();
		newQueue->maxElement = NULL;
	}
	else {
		newQueue->queue = spListCopy(source_queue->queue);
		newQueue->maxElement = spListElementCopy(source_queue->maxElement);
	}

	//allocation error, or given source queue is NULL and createNewList if false
	if (newQueue->queue == NULL)
		return NULL;

	return newQueue;
}

SPBPQueue spBPQueueCreate(int maxSize) {
	spMinimalVerifyArgumentsRn(maxSize > 0);

	return spBPQueueCreateWrapper(maxSize, NULL, true);
}

SPBPQueue spBPQueueCopy(SPBPQueue source) {
	spMinimalVerifyArgumentsRn(source != NULL);

	return spBPQueueCreateWrapper(source->capacity, source , false);
}

void spBPQueueDestroy(SPBPQueue source) {
	if (source != NULL) {
		if (source->queue != NULL)
			spListDestroy(source->queue);
		spListElementDestroy(source->maxElement);
		source->maxElement = NULL;
		free(source);
		source = NULL;
	}
}

void spBPQueueClear(SPBPQueue source) {
	if (source != NULL && source->queue != NULL) {
		spListClear(source->queue);
		if (source->maxElement) {
			spListElementDestroy(source->maxElement);
			source->maxElement = NULL;
		}
	}
}

int spBPQueueSize(SPBPQueue source) {
	spMinimalVerifyArguments(source != NULL && source->queue != NULL,
			DEFAULT_INVALID_NUMBER);
	return spListGetSize(source->queue);
}

int spBPQueueGetMaxSize(SPBPQueue source) {
	spMinimalVerifyArguments(source != NULL,DEFAULT_INVALID_NUMBER);
	return source->capacity;
}


SP_BPQUEUE_MSG spBPQueueInsertIfEmpty(SPBPQueue source, SPListElement newElement) {
	SP_LIST_MSG retVal;
	retVal = spListInsertFirst(source->queue, newElement);
	if (retVal == SP_LIST_OUT_OF_MEMORY)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	source->maxElement = spListElementCopy(newElement);

	if (source->maxElement == NULL)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	return SP_BPQUEUE_SUCCESS;
}


SP_BPQUEUE_MSG spBPQueueHandleFullCapacity(SPBPQueue source) {
	int i;
	SPListElement currElemInQueue, prevElemInQueue;
	currElemInQueue = spListGetFirst(source->queue);
	prevElemInQueue = currElemInQueue;

	for (i=1;i<spBPQueueSize(source);i++) {
		prevElemInQueue = currElemInQueue;
		currElemInQueue = spListGetNext(source->queue);
	}

	// if we get here we assume we never reached the end of the list (meaning null)
	// because we assume spBPQueueSize is valid
	spListRemoveCurrent(source->queue);

	spListElementDestroy(source->maxElement);
	source->maxElement = spListElementCopy(prevElemInQueue);
	if (source->maxElement == NULL)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	return SP_BPQUEUE_SUCCESS;

}




SP_BPQUEUE_MSG spBPQueueInsertNotEmptyNotLast(SPBPQueue source,
		SPListElement element) {
	SP_LIST_MSG retVal;
	retVal = spListInsertBeforeCurrent(source->queue, element);

	if (retVal == SP_LIST_OUT_OF_MEMORY)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	if (spBPQueueSize(source) > spBPQueueGetMaxSize(source))
		return spBPQueueHandleFullCapacity(source);

	return SP_BPQUEUE_SUCCESS;
}


SP_BPQUEUE_MSG spBPQueueInsertNotEmptyButLast(SPBPQueue source, SPListElement element) {
	SP_LIST_MSG retVal;
	retVal = spListInsertLast(source->queue, element);

	if (retVal == SP_LIST_OUT_OF_MEMORY)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	spListElementDestroy(source->maxElement);
	source->maxElement = spListElementCopy(element);

	if (source->maxElement == NULL)
		return SP_BPQUEUE_OUT_OF_MEMORY;

	return SP_BPQUEUE_SUCCESS;
}


SP_BPQUEUE_MSG spBPQueueInsertNotEmpty(SPBPQueue source, SPListElement newElement) {
	SPListElement currElemInQueue = spListGetFirst(source->queue);

	while (currElemInQueue != NULL && spListElementCompare(currElemInQueue, newElement) <= 0) {
		currElemInQueue = spListGetNext(source->queue);
	}

	// currElemInQueue > newElement
	if (currElemInQueue != NULL) {
		currElemInQueue = NULL;
		return spBPQueueInsertNotEmptyNotLast(source, newElement);
	}

	// we are not at full capacity because we took care of it before
	return spBPQueueInsertNotEmptyButLast(source, newElement);
}

SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element) {
	spMinimalVerifyArguments(source != NULL && source->queue != NULL && element != NULL,
			SP_BPQUEUE_INVALID_ARGUMENT);

	if (spBPQueueGetMaxSize(source) == 0)
		return SP_BPQUEUE_FULL;


	// the list is full and the element is greater than all the current items
	if (spBPQueueIsFull(source) && spListElementCompare(element, source->maxElement) >= 0) {
		return SP_BPQUEUE_FULL;
	}

	if (spBPQueueIsEmpty(source))
		return spBPQueueInsertIfEmpty(source,element);

	//insert to a non empty queue
	return spBPQueueInsertNotEmpty(source, element);
}

SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source) {
	SPListElement first;
	SP_LIST_MSG actionStatus;

	spMinimalVerifyArguments(source != NULL, SP_BPQUEUE_INVALID_ARGUMENT);

	if (spBPQueueIsEmpty(source))
		return SP_BPQUEUE_EMPTY;

	first = spListGetFirst(source->queue);
	if (first == NULL)
		return SP_BPQUEUE_EMPTY;

	// if we have 1 items -> last is first -> we should free its pointer
	if (spBPQueueSize(source) == 1) {
		spListElementDestroy(source->maxElement);
		source->maxElement = NULL;
	}

	actionStatus = spListRemoveCurrent(source->queue);

	if (actionStatus != SP_LIST_SUCCESS)
		return SP_BPQUEUE_EMPTY;

	return SP_BPQUEUE_SUCCESS;
}

SPListElement spBPQueuePeek(SPBPQueue source) {
	SPListElement first;
	spMinimalVerifyArgumentsRn(source != NULL);

	first = spListGetFirst(source->queue);

	return spListElementCopy(first);
}

SPListElement spBPQueuePeekLast(SPBPQueue source) {
	spMinimalVerifyArgumentsRn(source != NULL);
	return spListElementCopy(source->maxElement);
}


double returnValueFrom(SPBPQueue source, SPListElement (*func)(SPBPQueue)) {
	SPListElement item;
	double returnValue;
	spMinimalVerifyArguments(source != NULL, DEFAULT_INVALID_NUMBER);

	item = (*func)(source);
	returnValue = spListElementGetValue(item);
	spListElementDestroy(item);

	return returnValue;
}

double spBPQueueMinValue(SPBPQueue source) {
	return returnValueFrom(source, &spBPQueuePeek);
}

double spBPQueueMaxValue(SPBPQueue source) {
	return returnValueFrom(source, &spBPQueuePeekLast);
}

bool spBPQueueIsEmpty(SPBPQueue source) {
	assert(source);
	return (spBPQueueSize(source) == 0);
}

bool spBPQueueIsFull(SPBPQueue source) {
	assert(source);
	return (spBPQueueSize(source) == spBPQueueGetMaxSize(source));
}
