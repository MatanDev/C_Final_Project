#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "unit_test_util.h"
#include "SPKDTreeNodeKNNUnitTest.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"
#include "../data_structures/kd_ds/SPKDTreeNodeKNN.h"
#include "SPKDArrayUnitTest.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../SPPoint.h"

#define ERROR_ALLOCATING_MEMORY 							"Error allocating memory"
#define COULD_NOT_CREATE_POINTS_ARRAY 						"Could not create points array"
#define COULD_NOT_INITIALIZE_KD_ARRAY 						"Could not initialize kd-array"
#define COULD_NOT_INITIALIZE_TREE 							"Could not initialize tree - InitKDTree returned null"
#define COULD_NOT_INITIALIZE_QUEUE 							"Could not create BPQueue"
#define COULD_NOT_INITIALIZE_QUERY_POINT					"Could not create query point"


//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  							500
#define RANDOM_TESTS_DIM_RANGE 								50
#define RANDOM_TESTS_COUNT 									50

//test case 1 data
SPKDTreeNode kNNtestCase1Tree 								= NULL;
SPBPQueue kNNtestCase1QueueKNN 								= NULL;
SPPoint kNNtestCase1QueryPoint								= NULL;
SPPoint* kNNtestCase1Points 								= NULL;
#define kNNtestCase1MaxDim           						3
#define kNNtestCase1Size 									2
#define kNNtestCase1SplitMethod 							MAX_SPREAD
#define kNNtestCase1K										1

//test case 2 data
SPKDTreeNode kNNtestCase2Tree 								= NULL;
SPBPQueue kNNtestCase2Queue 								= NULL;
SPPoint kNNtestCase2QueryPoint								= NULL;
SPPoint* kNNtestCase2Points 								= NULL;
#define kNNtestCase2MaxDim 									2
#define kNNtestCase2Size 									5
#define kNNtestCase2SplitMethod 							INCREMENTAL
#define kNNtestCase2K										3

//edge case 1 data
SPKDTreeNode kNNedgeTestCase1Tree 							= NULL;
SPBPQueue kNNedgeTestCase1Queue 							= NULL;
SPPoint kNNedgeTestCase1QueryPoint							= NULL;
SPPoint* kNNedgeTestCase1Points 							= NULL;
#define kNNedgeTestCase1MaxDim 								1
#define kNNedgeTestCase1Size 								1
#define kNNedgeTestCase1SplitMethod 						INCREMENTAL
#define kNNedgeTestCase1K									1

typedef struct distanceWithPoint {
	double distance;
	SPPoint point;
} distanceWithPoint;

 int distanceWithPointComparator(const void * firstItem, const void * secondItem) {
	distanceWithPoint* item1;
	distanceWithPoint* item2;
	double dist;
	item1 = (distanceWithPoint*)firstItem;
	item2 = (distanceWithPoint*)secondItem;

	dist = (item1)->distance - (item2)->distance;

	if (dist < 0.0)
		return -1;

	if (dist > 0.0)
		return 1;

	return spPointGetIndex((item1)->point) - spPointGetIndex((item2)->point);
}

distanceWithPoint* createAndSortDistancesArray(int size, SPPoint queryPoint, SPPoint* pointsArray){
	distanceWithPoint* distancesArray = NULL;
	int i;
	distancesArray = (distanceWithPoint*)calloc(sizeof(distanceWithPoint),size);
	if (distancesArray  == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}

	for (i=0;i<size;i++){
		distancesArray[i].point = pointsArray[i];
		distancesArray[i].distance = spPointL2SquaredDistance(queryPoint, distancesArray[i].point);
	}
	qsort(distancesArray, size, sizeof(distanceWithPoint), distanceWithPointComparator);
	return distancesArray;
}

int* getRealRsltsArray(int k,SPPoint* pointsArray,SPPoint queryPoint,int size){
	int i;
	distanceWithPoint* distancesArray;
	int *outputArray = (int*)calloc(k, sizeof(int));

	if (outputArray == NULL)
	{
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}

	distancesArray = createAndSortDistancesArray(size,queryPoint, pointsArray);

	if (distancesArray  == NULL) {
		free(outputArray);
		return NULL;
	}

	for (i = 0; i < k; i++)
		outputArray[i] = spPointGetIndex((distancesArray[i]).point);

	free(distancesArray);
	return outputArray;
}

//general test
bool verifyKNN(SPBPQueue rsltQueue, int k,SPPoint* pointsArray,SPPoint queryPoint, int numOfPoints){
	SPBPQueue workingQueue = NULL;
	SPListElement tempElement = NULL;
	SP_BPQUEUE_MSG msg;
	bool emptyFlag;
	int i, queueSize;
	int* rsltsArray;

	if (rsltQueue == NULL || pointsArray == NULL || queryPoint == NULL)
		return false;

	workingQueue = spBPQueueCopy(rsltQueue);

	ASSERT_TRUE(workingQueue != NULL);


	rsltsArray = getRealRsltsArray(k,pointsArray,queryPoint,numOfPoints);

	if (rsltsArray == NULL){
		spBPQueueDestroy(workingQueue);
		return false;
	}

	queueSize = spBPQueueSize(workingQueue);
	for (i = 0; i< queueSize ; i++){
		tempElement = spBPQueuePeek(workingQueue);

		if (spListElementGetIndex(tempElement) != rsltsArray[i]){
			free(rsltsArray);
			spBPQueueDestroy(workingQueue);
			return false;
		}
		spListElementDestroy(tempElement);

		msg = spBPQueueDequeue(workingQueue);
		if (msg != SP_BPQUEUE_SUCCESS){
			free(rsltsArray);
			spBPQueueDestroy(workingQueue);
			return false;
		}
	}
	emptyFlag = spBPQueueIsEmpty(workingQueue);
	spBPQueueDestroy(workingQueue);
	free(rsltsArray);
	return emptyFlag;
}

void destroyCaseData(SPKDTreeNode tree,SPKDArray kdArr,SPPoint* pointsArray,int size,SPBPQueue queue,SPPoint queryPoint){
	if (tree) spKDTreeDestroy(tree);
	if (kdArr) spKDArrayDestroy(kdArr);
	if (pointsArray) destroyPointsArray(pointsArray,size);
	if (queue) spBPQueueDestroy(queue);
	if (queryPoint) spPointDestroy(queryPoint);
}


//null test
bool runKnnNullTests(){
	SPBPQueue tempQueue = NULL;
	SPPoint point = generateRandomPoint(4,1);

	tempQueue = spBPQueueCreate(4);

	kNearestNeighbors(NULL,tempQueue, point);
	ASSERT_TRUE(spBPQueueIsEmpty(tempQueue));

	spPointDestroy(point);
	spBPQueueDestroy(tempQueue);
	return true;
}

//specific case tests

//test case 1
void initializeKnnTestCase1(){
	double data1[3] = {1,2,3}, data2[3] = {5,-7,13}, queryData[3] = {0,0,0};
	SPPoint p1,p2;
	SPKDArray kdArr = NULL;

	kNNtestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),kNNtestCase1Size);
	if (kNNtestCase1Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,kNNtestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,kNNtestCase1MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kNNtestCase1QueryPoint = spPointCreate(queryData, kNNtestCase1MaxDim, 3);
	if (kNNtestCase1QueryPoint == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kNNtestCase1Points[0] = p1;
	kNNtestCase1Points[1] = p2;

	kNNtestCase1QueueKNN = spBPQueueCreate(kNNtestCase1K);
	if (kNNtestCase1QueueKNN == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kdArr = Init(kNNtestCase1Points,kNNtestCase1Size);
	kNNtestCase1Tree = InitKDTree(kdArr, kNNtestCase1SplitMethod);
	spKDArrayDestroy(kdArr);
}

bool runKnnTestCase1(){
	SPListElement curr_elem;
	bool successFlag;
	if (kNNtestCase1Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (kNNtestCase1Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNtestCase1QueueKNN == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNtestCase1QueryPoint == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(kNNtestCase1Tree, kNNtestCase1QueueKNN, kNNtestCase1QueryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(kNNtestCase1QueueKNN, kNNtestCase1K, kNNtestCase1Points, kNNtestCase1QueryPoint,kNNtestCase1Size));


	//elements should be 1
	curr_elem = spBPQueuePeek(kNNtestCase1QueueKNN);
	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	return true;
}

void destroyKnnTestCase1(){
	destroyCaseData(kNNtestCase1Tree, NULL, kNNtestCase1Points, kNNtestCase1Size, kNNtestCase1QueueKNN, kNNtestCase1QueryPoint);

	kNNtestCase1Points = NULL;
	kNNtestCase1Tree = NULL;
	kNNtestCase1QueueKNN = NULL;
	kNNtestCase1QueryPoint = NULL;
}

//test case 2
void initializeKnnTestCase2(){
	double data1[2] = {1,2},data2[2] = {123,70},data3[2] = {2,7},data4[2] = {9,11},
			data5[2] = {3,4}, queryData[2] = {0,0};
	SPPoint p1,p2,p3,p4,p5;
	SPKDArray kdArr = NULL;
	kNNtestCase2Points = (SPPoint*)calloc(sizeof(SPPoint),kNNtestCase2Size);
	if (kNNtestCase2Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,kNNtestCase2MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,kNNtestCase2MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p3 = spPointCreate(data3,kNNtestCase2MaxDim,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p4 = spPointCreate(data4,kNNtestCase2MaxDim,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p5 = spPointCreate(data5,kNNtestCase2MaxDim,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	kNNtestCase2Points[0] = p1;
	kNNtestCase2Points[1] = p2;
	kNNtestCase2Points[2] = p3;
	kNNtestCase2Points[3] = p4;
	kNNtestCase2Points[4] = p5;

	kNNtestCase2QueryPoint= spPointCreate(queryData,kNNtestCase2MaxDim,6);
	if (kNNtestCase2QueryPoint == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spPointDestroy(p5);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kNNtestCase2Queue = spBPQueueCreate(kNNtestCase2K);
	if (kNNtestCase2Queue == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spPointDestroy(p5);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kdArr = Init(kNNtestCase2Points,kNNtestCase2Size);
	kNNtestCase2Tree = InitKDTree(kdArr, kNNtestCase2SplitMethod);
	spKDArrayDestroy(kdArr);
}

bool runKnnTestCase2(){
	SPListElement curr_elem;
	SP_BPQUEUE_MSG msg;
	bool successFlag;
	if (kNNtestCase2Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (kNNtestCase2Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNtestCase2Queue == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNtestCase2QueryPoint == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(kNNtestCase2Tree, kNNtestCase2Queue, kNNtestCase2QueryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(kNNtestCase2Queue, kNNtestCase2K, kNNtestCase2Points, kNNtestCase2QueryPoint, kNNtestCase2Size));


	//first elem should be 1
	curr_elem = spBPQueuePeek(kNNtestCase2Queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	msg = spBPQueueDequeue(kNNtestCase2Queue);
	ASSERT_TRUE(msg == SP_BPQUEUE_SUCCESS);

	//second element should be 5
	curr_elem = spBPQueuePeek(kNNtestCase2Queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 5);
	spListElementDestroy(curr_elem);

	msg = spBPQueueDequeue(kNNtestCase2Queue);
	ASSERT_TRUE(msg == SP_BPQUEUE_SUCCESS);

	//third element should be 3
	curr_elem = spBPQueuePeek(kNNtestCase2Queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 3);
	spListElementDestroy(curr_elem);
	return true;
}

void destroyKnnTestCase2(){
	destroyCaseData(kNNtestCase2Tree, NULL, kNNtestCase2Points, kNNtestCase2Size, kNNtestCase2Queue, kNNtestCase2QueryPoint);

	kNNtestCase2Points = NULL;
	kNNtestCase2Tree = NULL;
	kNNtestCase2Queue = NULL;
	kNNtestCase2QueryPoint = NULL;
}

//edge test case 1
void initializeKnnEdgeTestCase1(){
	double data1[1] = {2}, queryData[1] = {0};
	SPPoint p1;
	SPKDArray kdArr = NULL;

	kNNedgeTestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),kNNedgeTestCase1Size);
	if (kNNedgeTestCase1Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,kNNedgeTestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}


	kNNedgeTestCase1Points[0] = p1;


	kNNedgeTestCase1QueryPoint = spPointCreate(queryData,kNNedgeTestCase1MaxDim,2);
	if (kNNedgeTestCase1QueryPoint == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	kNNedgeTestCase1Queue = spBPQueueCreate(kNNedgeTestCase1K);
	if (kNNedgeTestCase1Queue == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}


	kdArr = Init(kNNedgeTestCase1Points,kNNedgeTestCase1Size);
	kNNedgeTestCase1Tree = InitKDTree(kdArr, kNNedgeTestCase1SplitMethod);
	spKDArrayDestroy(kdArr);
}

bool runKnnEdgeTestCase1(){
	SPListElement curr_elem;
	bool successFlag;
	if (kNNedgeTestCase1Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (kNNedgeTestCase1Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNedgeTestCase1Queue == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (kNNedgeTestCase1QueryPoint == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(kNNedgeTestCase1Tree, kNNedgeTestCase1Queue, kNNedgeTestCase1QueryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(kNNedgeTestCase1Queue, kNNedgeTestCase1K, kNNedgeTestCase1Points, kNNedgeTestCase1QueryPoint,kNNedgeTestCase1Size));


	//elements should be 1
	curr_elem = spBPQueuePeek(kNNedgeTestCase1Queue);
	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	return true;
}

void destroyKnnEdgeTestCase1(){
	destroyCaseData(kNNedgeTestCase1Tree, NULL, kNNedgeTestCase1Points, kNNedgeTestCase1Size, kNNedgeTestCase1Queue, kNNedgeTestCase1QueryPoint);

	kNNedgeTestCase1Points = NULL;
	kNNedgeTestCase1Tree = NULL;
	kNNedgeTestCase1Queue = NULL;
	kNNedgeTestCase1QueryPoint = NULL;
}


//random tests
bool runRandomKnnTest(){
	int maxDim, size, k;
	bool successFlag = true;
	SPPoint* pointsArray = NULL;
	SPPoint queryPoint = NULL;
	SPBPQueue queue = NULL;
	SPKDArray kdArr = NULL;
	SPKDTreeNode tree = NULL;
	SP_KDTREE_SPLIT_METHOD splitMethod;

	maxDim = 1 + (int)(rand() % RANDOM_TESTS_DIM_RANGE);
	size = 2 + (int)(rand() % RANDOM_TESTS_SIZE_RANGE); //size = 1 is tested at a the edge cases
	k = (int)(rand() % size);
	splitMethod = (int)(rand()%3);

	pointsArray = generateRandomPointsArray(maxDim,size);
	queryPoint = generateRandomPoint(maxDim, size+1);

	queue = spBPQueueCreate(k);

	if (queue == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);

		spLoggerPrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_QUERY_POINT);
		return false;
	}

	if (queryPoint == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);

		spLoggerPrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_QUERY_POINT);
		return false;
	}

	if (pointsArray == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}

	kdArr = Init(pointsArray,size);

	if (kdArr == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerPrintError(COULD_NOT_INITIALIZE_KD_ARRAY,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_KD_ARRAY);
		return false;
	}

	tree = InitKDTree(kdArr, splitMethod);

	if (tree == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerPrintError(
				COULD_NOT_INITIALIZE_TREE,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}


	successFlag = kNearestNeighbors(tree, queue, queryPoint);

	if (!successFlag){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		return false;
	}

	successFlag = verifyKNN(queue, k, pointsArray,queryPoint,size);
	destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);

	return successFlag;
}



void runKDTreeNodeKNNTests(){
	int i;
	int seed = time(NULL);
	srand(seed);

	//null case
	RUN_TEST(runKnnNullTests);

	//case 1
	initializeKnnTestCase1();
	RUN_TEST(runKnnTestCase1);
	destroyKnnTestCase1();

	//case 2
	initializeKnnTestCase2();
	RUN_TEST(runKnnTestCase2);
	destroyKnnTestCase2();

	//edge case 1
	initializeKnnEdgeTestCase1();
	RUN_TEST(runKnnEdgeTestCase1);
	destroyKnnEdgeTestCase1();

	//random tests
	for (i = 0 ; i< RANDOM_TESTS_COUNT;i++)
		RUN_TEST(runRandomKnnTest);
}

