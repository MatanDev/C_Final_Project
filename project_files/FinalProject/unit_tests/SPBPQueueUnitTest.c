#include "unit_test_util.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "SPBPQueueUnitTest.h"
#include "../SPPoint.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RANDOM_INDEX_RANGE 300
#define RANDOM_VALUE_BALANCER 100
#define RANDOM_SIZE_RANGE 500
#define RANDOM_CAPACITY_RANGE 500
#define RANDOM_SORT_TEST_COUNT 100
#define DEFAULT_INVALID_NUMBER -1

//a method used to create a random queue
static SPBPQueue quickRandomQueue(int capacity, int size) {
	SP_BPQUEUE_MSG message;
	int i, index;
	double val;
	SPListElement elem, temp;
	SPBPQueue queue = spBPQueueCreate(capacity);

	for (i = 0; i < size; i++) 	{
		val = (double)rand()/((double)RAND_MAX/RANDOM_VALUE_BALANCER);
		index = (int)(rand() % RANDOM_INDEX_RANGE);

		elem = spListElementCreate(index, val);
		ASSERT_TRUE(elem != NULL);
		message = spBPQueueEnqueue(queue, elem);

		ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS || message == SP_BPQUEUE_FULL);
		spListElementDestroy(elem);
		elem = NULL;
		if (capacity > 0) {
			temp = spBPQueuePeekLast(queue);
			ASSERT_TRUE(temp != NULL);
			spListElementDestroy(temp);
			temp = NULL;
			temp = spBPQueuePeek(queue);
			ASSERT_TRUE(temp != NULL);
			spListElementDestroy(temp);
			temp = NULL;
		}
	}
	return queue;
}

//a method used to create a queue given its properties and items
static SPBPQueue quickQueue(int capacity, int size, ...) {
	int i;
	va_list items;
	SPBPQueue queue = spBPQueueCreate(capacity);
	va_start(items, size);
	for (i = 0; i < size; i++) {
		spBPQueueEnqueue(queue, va_arg(items, SPListElement));
	}
	va_end(items);
	return queue;
}

// a method used in order to remove all items from a queue, using dequeue
static void quickDequeue(SPBPQueue queue, int size) {
	int i;
	for (i = 0; i < size; i++) {
		spBPQueueDequeue(queue);
	}
}

//Test for queue create method
static bool testBPQueueCreate() {
	SPBPQueue queue = NULL, queue2 = NULL;

	//check for invalid arguments
	queue = spBPQueueCreate(DEFAULT_INVALID_NUMBER);
	ASSERT_TRUE(queue == NULL);

	queue2 = spBPQueueCreate(17);
	ASSERT_TRUE(queue2 != NULL);
	ASSERT_TRUE(spBPQueueSize(queue2) == 0);
	ASSERT_TRUE(spBPQueueGetMaxSize(queue2) == 17);
	ASSERT_TRUE(spBPQueuePeekLast(queue2) == NULL); // validate that maxElem is null

	spBPQueueDestroy(queue2);

	return true;
}

//Test for queue copy method
static bool testBPQueueCopy() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL, temp = NULL;
	SPBPQueue queue = spBPQueueCreate(1), queue2 = NULL , copy2 = NULL ,copy = NULL;

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);

	copy = spBPQueueCopy(queue);

	// test invalid arguments
	ASSERT_TRUE(spBPQueueCopy(NULL) == NULL);

	ASSERT_TRUE(copy != NULL);
	ASSERT_TRUE(0 == spBPQueueSize(copy));

	spBPQueueEnqueue(queue, e1);

	ASSERT_TRUE(0 == spBPQueueSize(copy));
	queue2 = quickQueue(17, 4, e4, e1, e3, e2);
	copy2 = spBPQueueCopy(queue2);



	ASSERT_TRUE(spBPQueueMaxValue(queue2) == spBPQueueMaxValue(copy2));
	ASSERT_TRUE(4 == spBPQueueSize(copy2));
	ASSERT_TRUE(17 == spBPQueueGetMaxSize(copy2));

	temp = spBPQueuePeek(copy2);
	ASSERT_TRUE(spListElementCompare(e1, temp) == 0);
	spListElementDestroy(temp);
	ASSERT_TRUE(spBPQueueDequeue(copy2) == SP_BPQUEUE_SUCCESS);
	ASSERT_TRUE(!spBPQueueIsFull(copy2));
	temp = spBPQueuePeek(copy2);
	ASSERT_TRUE(spListElementCompare(e2, temp) == 0);
	spListElementDestroy(temp);
	ASSERT_TRUE(spBPQueueDequeue(copy2) == SP_BPQUEUE_SUCCESS);
	temp = spBPQueuePeek(copy2);
	ASSERT_TRUE(spListElementCompare(e3, temp) == 0);
	spListElementDestroy(temp);
	ASSERT_TRUE(spBPQueueDequeue(copy2) == SP_BPQUEUE_SUCCESS);
	temp = spBPQueuePeek(copy2);
	ASSERT_TRUE(spListElementCompare(e4, temp) == 0);
	spListElementDestroy(temp);
	temp = NULL;
	ASSERT_TRUE(spBPQueueDequeue(copy2) == SP_BPQUEUE_SUCCESS);
	temp = spBPQueuePeek(copy2);
	ASSERT_TRUE(temp == NULL);
	spListElementDestroy(temp);


	spBPQueueDestroy(queue);
	spBPQueueDestroy(queue2);
	spBPQueueDestroy(copy);
	spBPQueueDestroy(copy2);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);
	return true;
}

//Test for queue size
static bool testBPQueueSize() {
	SPListElement temp = NULL, e1 = NULL;
	SPBPQueue queue = spBPQueueCreate(2);

	ASSERT_TRUE(0 == spBPQueueSize(queue));
	e1 = spListElementCreate(1, 1.0);
	spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(1 == spBPQueueSize(queue));
	spListElementSetIndex(e1, 2);
	spListElementSetValue(e1, 2.0);
	spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(2 == spBPQueueSize(queue));
	temp = spBPQueuePeek(queue);
	spListElementDestroy(temp);
	spBPQueueDequeue(queue);
	ASSERT_TRUE(1 == spBPQueueSize(queue));

	//test invalid argument
	ASSERT_TRUE(spBPQueueSize(NULL) ==  DEFAULT_INVALID_NUMBER);

	spBPQueueDestroy(queue);
	spListElementDestroy(e1);
	return true;
}

//Test for queue max size limit
static bool testBPQueueMaxSize() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL;
	SPBPQueue queue = NULL;

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);
	queue = quickQueue(5, 4, e2, e3, e1, e4);

	ASSERT_TRUE(4 == spBPQueueSize(queue));
	ASSERT_TRUE(5 == spBPQueueGetMaxSize(queue));
	ASSERT_TRUE(spBPQueueSize(queue) < spBPQueueGetMaxSize(queue));

	//test invalid argument
	ASSERT_TRUE(spBPQueueGetMaxSize(NULL) ==  DEFAULT_INVALID_NUMBER);

	spBPQueueDestroy(queue);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);

	return true;
}


//Test for peek and peek last methods
static bool testBPQueuePeekFirstandLast() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL, first = NULL, last = NULL, temp = NULL;
	SPBPQueue queue = NULL, queue2 = NULL;

	queue = spBPQueueCreate(0);
	ASSERT_TRUE(spBPQueuePeek(queue) == NULL);
	ASSERT_TRUE(spBPQueuePeekLast(queue) == NULL);

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);

	queue2 = quickQueue(5, 4, e2, e3, e1, e4);

	first = spBPQueuePeek(queue2);
	ASSERT_TRUE(spListElementCompare(e1, first) == 0);

	temp = spBPQueuePeek(queue2);
	ASSERT_TRUE(spListElementCompare(first, temp) == 0);

	spListElementDestroy(temp);
	spListElementDestroy(first);

	last = spBPQueuePeekLast(queue2);
	ASSERT_TRUE(spListElementCompare(e4, last) == 0);

	temp = spBPQueuePeekLast(queue2);
	ASSERT_TRUE(spListElementCompare(last, temp) == 0);

	//test invalid arguments - peek
	ASSERT_TRUE(spBPQueuePeek(NULL) == NULL);
	//test invalid arguments - peek last
	ASSERT_TRUE(spBPQueuePeekLast(NULL) == NULL);

	spListElementDestroy(last);
	spListElementDestroy(temp);
	spBPQueueDestroy(queue);
	spBPQueueDestroy(queue2);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);

	return true;
}

//Test for minimum and maximum values
static bool testBPQueueMinMaxValue() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL;
	SPBPQueue queue = NULL, queue2 = NULL;

	queue = spBPQueueCreate(0);
	ASSERT_TRUE(spBPQueueMinValue(queue) == DEFAULT_INVALID_NUMBER);
	ASSERT_TRUE(spBPQueueMaxValue(queue) == DEFAULT_INVALID_NUMBER);

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);

	queue2 = quickQueue(5, 4, e2, e3, e1, e4);

	ASSERT_TRUE(spBPQueueMinValue(queue2) == 1);
	ASSERT_TRUE(spBPQueueMaxValue(queue2) == 4);


	//test invalid arguments
	ASSERT_TRUE(spBPQueueMinValue(NULL) == DEFAULT_INVALID_NUMBER);
	ASSERT_TRUE(spBPQueueMaxValue(NULL) == DEFAULT_INVALID_NUMBER);

	spBPQueueDestroy(queue);
	spBPQueueDestroy(queue2);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);

	return true;
}

//Test for is empty and is full methods
static bool testBPQueueIsEmptyFull() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL, e5 = NULL;
	SPBPQueue queue = NULL, queue2 = NULL;

	queue = spBPQueueCreate(1);

	ASSERT_TRUE(spBPQueueIsEmpty(queue));
	//ASSERT_TRUE(spBPQueueIsFull(queue));

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);
	e5 = spListElementCreate(5, 5.0);

	queue2 = quickQueue(5, 4, e2, e3, e1, e4);
	ASSERT_TRUE(!spBPQueueIsFull(queue2));
	ASSERT_TRUE(!spBPQueueIsEmpty(queue2));

	spBPQueueEnqueue(queue2, e5);
	ASSERT_TRUE(spBPQueueIsFull(queue2));
	ASSERT_TRUE(!spBPQueueIsEmpty(queue2));

	quickDequeue(queue2, 4);
	ASSERT_TRUE(!spBPQueueIsEmpty(queue2));
	ASSERT_TRUE(!spBPQueueIsFull(queue2));

	spBPQueueDequeue(queue2);
	ASSERT_TRUE(spBPQueueIsEmpty(queue2));
	ASSERT_TRUE(!spBPQueueIsFull(queue2));

	spBPQueueDestroy(queue);
	spBPQueueDestroy(queue2);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);
	spListElementDestroy(e5);
	return true;
}

//Test for Enqueue
static bool testBPQueueEnqueue() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL,
			e4 = NULL, e5 = NULL, temp = NULL;
	SPBPQueue queue = NULL;
	SP_BPQUEUE_MSG message;

	e1 = spListElementCreate(1, 1.1);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);
	e5 = spListElementCreate(5, 5.82);
	queue = quickQueue(4, 4, e1, e2, e4, e5);

	temp = spBPQueuePeekLast(queue);
	ASSERT_TRUE(isEqual(spListElementCompare(temp, e5), 0.0));
	spListElementDestroy(temp);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(isEqual(spListElementCompare(temp, e1) , 0.0));
	spListElementDestroy(temp);

	ASSERT_TRUE(isEqual(spBPQueueMaxValue(queue) , 5.82));
	ASSERT_TRUE(isEqual(spBPQueueMinValue(queue) , 1.1));

	message = spBPQueueEnqueue(queue, e3);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	temp = spBPQueuePeekLast(queue);
	ASSERT_TRUE(isEqual(spListElementCompare(temp, e4) ,0.0));
	spListElementDestroy(temp);

	message = spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);
	message = spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);
	message = spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);
	message = spBPQueueEnqueue(queue, e1);
	ASSERT_TRUE(message == SP_BPQUEUE_FULL);

	temp = spBPQueuePeekLast(queue);
	ASSERT_TRUE(isEqual(spListElementCompare(temp, e1), 0.0));
	spListElementDestroy(temp);

	//test invalid arguments
	message = spBPQueueEnqueue(NULL, NULL);
	ASSERT_TRUE(message == SP_BPQUEUE_INVALID_ARGUMENT);

	message = spBPQueueEnqueue(queue, NULL);
	ASSERT_TRUE(message == SP_BPQUEUE_INVALID_ARGUMENT);

	message = spBPQueueEnqueue(NULL, e1);
	ASSERT_TRUE(message == SP_BPQUEUE_INVALID_ARGUMENT);

	spBPQueueDestroy(queue);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);
	spListElementDestroy(e5);
	return true;
}

//Test for Dequeue
static bool testBPQueueDequeue() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL, e5 = NULL, temp = NULL;
	SPBPQueue queue = NULL;
	SP_BPQUEUE_MSG message;

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 2.0);
	e4 = spListElementCreate(4, 7.0);
	e5 = spListElementCreate(5, 5.0);
	queue = quickQueue(5, 5, e1, e2, e3, e4, e5);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(spListElementCompare(temp,e1) == 0);
	spListElementDestroy(temp);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(spListElementCompare(temp,e2) == 0);
	spListElementDestroy(temp);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(spListElementCompare(temp,e3) == 0);
	spListElementDestroy(temp);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(spListElementCompare(temp,e5) == 0);
	spListElementDestroy(temp);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	temp = spBPQueuePeek(queue);
	ASSERT_TRUE(spListElementCompare(temp,e4) == 0);
	spListElementDestroy(temp);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);

	message = spBPQueueDequeue(queue);
	ASSERT_TRUE(message == SP_BPQUEUE_EMPTY);

	//test invalid argument
	message  = spBPQueueDequeue(NULL);
	ASSERT_TRUE(message == SP_BPQUEUE_INVALID_ARGUMENT);

	spBPQueueDestroy(queue);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);
	spListElementDestroy(e5);
	return true;
}

//Test for the 'clear' method
static bool testBPQueueClear() {
	SPListElement e1 = NULL, e2 = NULL, e3 = NULL , e4 = NULL;
	SPBPQueue queue = NULL, queue2 = NULL;

	e1 = spListElementCreate(1, 1.0);
	e2 = spListElementCreate(2, 2.0);
	e3 = spListElementCreate(3, 3.0);
	e4 = spListElementCreate(4, 4.0);
	queue = quickQueue(7, 4, e3, e4, e2, e1);

	spBPQueueClear(queue);

	ASSERT_TRUE(0 == spBPQueueSize(queue));
	ASSERT_TRUE(7 == spBPQueueGetMaxSize(queue));

	ASSERT_TRUE(spBPQueuePeek(queue) == NULL);
	ASSERT_TRUE(spBPQueuePeekLast(queue) == NULL); // validate that maxElement is null

	queue2 = spBPQueueCreate(19);

	spBPQueueClear(queue2);
	ASSERT_TRUE(0 == spBPQueueSize(queue2));

	//verify invalid argument not crashing
	spBPQueueClear(NULL);

	spBPQueueDestroy(queue);
	spBPQueueDestroy(queue2);
	spListElementDestroy(e1);
	spListElementDestroy(e2);
	spListElementDestroy(e3);
	spListElementDestroy(e4);
	return true;
}

//Test for Destroy method
static bool testBPQueueDestroy() {
	spBPQueueDestroy(NULL);
	return true;
}

//Test for the queue ordered invariant, also tests the enqueue and dequeue
static bool testBPQueueSorted() {
	SPListElement prevElement = NULL, currentElement = NULL;
	SP_BPQUEUE_MSG message;
	int size,max_size, counter,i, numOfTests = RANDOM_SORT_TEST_COUNT;
	SPBPQueue queue = NULL;

	for (i = 0 ; i< numOfTests ; i++) {
		size = (int)(rand() % RANDOM_SIZE_RANGE);
		max_size = 1+(int)(rand() % RANDOM_CAPACITY_RANGE);

		counter = 0;
		ASSERT_TRUE(queue == NULL);
		queue = quickRandomQueue(max_size, size);
		ASSERT_TRUE(queue != NULL);

		if (!spBPQueueIsEmpty(queue)) {
			currentElement = spBPQueuePeek(queue);
			message	= spBPQueueDequeue(queue);
			ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);
			counter++;
		}
		while (!spBPQueueIsEmpty(queue)) {
			spListElementDestroy(prevElement);
			prevElement = NULL;
			prevElement = spListElementCopy(currentElement);
			spListElementDestroy(currentElement);
			currentElement = NULL;
			currentElement = spBPQueuePeek(queue);
			message	= spBPQueueDequeue(queue);
			ASSERT_TRUE(prevElement != NULL);
			ASSERT_TRUE(currentElement != NULL);
			ASSERT_TRUE(message == SP_BPQUEUE_SUCCESS);
			ASSERT_TRUE(spListElementCompare(prevElement,currentElement)<=0);
			counter++;
		}
		ASSERT_TRUE(counter == (size < max_size ? size : max_size) );
		ASSERT_TRUE(spBPQueueIsEmpty(queue));

		spListElementDestroy(currentElement);
		currentElement = NULL;
		spListElementDestroy(prevElement);
		prevElement = NULL;
		spBPQueueDestroy(queue);
		queue = NULL;
	}
	return true;
}


void runBPQueueTests() {
	srand(time(NULL));
	RUN_TEST(testBPQueueCreate);
	RUN_TEST(testBPQueueCopy);
	RUN_TEST(testBPQueueSize);
	RUN_TEST(testBPQueueMaxSize);
	RUN_TEST(testBPQueuePeekFirstandLast);
	RUN_TEST(testBPQueueMinMaxValue);
	RUN_TEST(testBPQueueClear);
	RUN_TEST(testBPQueueDestroy);
	RUN_TEST(testBPQueueSorted);
	RUN_TEST(testBPQueueIsEmptyFull);
	RUN_TEST(testBPQueueEnqueue);
	RUN_TEST(testBPQueueDequeue);
}
