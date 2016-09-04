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
#include "../general_utils/SPUtils.h"

#define COULD_NOT_CREATE_POINTS_ARRAY 						"Could not create points array"
#define COULD_NOT_INITIALIZE_KD_ARRAY 						"Could not initialize kd-array"
#define COULD_NOT_INITIALIZE_TREE 							"Could not initialize tree - InitKDTree returned null"
#define COULD_NOT_INITIALIZE_QUEUE 							"Could not create BPQueue"
#define COULD_NOT_INITIALIZE_QUERY_POINT					"Could not create query point"
#define COULD_NOT_CREATE_CASE_DATA							"Could not create case data"


//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  							500
#define RANDOM_TESTS_DIM_RANGE 								50
#define RANDOM_TESTS_COUNT 									10

typedef struct knn_test_case_data {
	SPKDTreeNode tree;
	SPPoint* points;
	SPBPQueue queue;
	SPPoint queryPoint;
	int max_dim;
	int size;
	int k;
	SP_KDTREE_SPLIT_METHOD split_method;
} knn_test_case_data;

typedef struct knn_test_case_data* knnTestCaseData;

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
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
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
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
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

void destroyCaseData(SPKDTreeNode tree,SPKDArray kdArr,SPPoint* pointsArray,int size,
		SPBPQueue queue,SPPoint queryPoint){
	if (tree) spKDTreeDestroy(tree, false);
	if (kdArr) spKDArrayDestroy(kdArr);
	if (pointsArray) destroyPointsArray(pointsArray,size);
	if (queue) spBPQueueDestroy(queue);
	if (queryPoint) spPointDestroy(queryPoint);
}

void destroyCaseDataByWrapperTestCase(knnTestCaseData caseData){
	if (caseData){
		destroyCaseData(caseData->tree, NULL, caseData->points, caseData->size,
				caseData->queue, caseData->queryPoint);
		free(caseData);
	}
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
knnTestCaseData initializeKnnTestCase1(){
	//test case 1 data
	SPKDTreeNode kNNtestCase1Tree 	= NULL;
	SPBPQueue kNNtestCase1QueueKNN = NULL;
	SPPoint kNNtestCase1QueryPoint	= NULL;
	SPPoint* kNNtestCase1Points = NULL;
	int kNNtestCase1MaxDim = 3;
	int kNNtestCase1Size = 2;
	SP_KDTREE_SPLIT_METHOD kNNtestCase1SplitMethod = MAX_SPREAD;
	int kNNtestCase1K = 1;
	knnTestCaseData caseData = NULL;
	double data1[3] = {1,2,3}, data2[3] = {5,-7,13}, queryData[3] = {0,0,0};
	SPPoint p1,p2;
	SPKDArray kdArr = NULL;

	spCalloc(caseData, knn_test_case_data, 1);

	kNNtestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),kNNtestCase1Size);
	if (kNNtestCase1Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = kNNtestCase1Points;
	caseData->size = kNNtestCase1Size;

	p1 = spPointCreate(data1,kNNtestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p2 = spPointCreate(data2,kNNtestCase1MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}

	kNNtestCase1Points[0] = p1;
	kNNtestCase1Points[1] = p2;

	kNNtestCase1QueryPoint = spPointCreate(queryData, kNNtestCase1MaxDim, 3);
	if (kNNtestCase1QueryPoint == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queryPoint = kNNtestCase1QueryPoint;


	kNNtestCase1QueueKNN = spBPQueueCreate(kNNtestCase1K);
	if (kNNtestCase1QueueKNN == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queue = kNNtestCase1QueueKNN;

	kdArr = Init(kNNtestCase1Points,kNNtestCase1Size);
	kNNtestCase1Tree = InitKDTree(kdArr, kNNtestCase1SplitMethod);
	spKDArrayDestroy(kdArr);

	caseData->k = kNNtestCase1K;
	caseData->split_method = kNNtestCase1SplitMethod;
	caseData->tree = kNNtestCase1Tree;

	return caseData;
}

bool runKnnTestCase1(knnTestCaseData caseData){
	SPListElement curr_elem;
	bool successFlag;
	if (caseData == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_CASE_DATA,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_CASE_DATA);
		return false;
	}
	if (caseData->tree == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (caseData->points == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queue == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queryPoint == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(caseData->tree, caseData->queue, caseData->queryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(caseData->queue, caseData->k, caseData->points, caseData->queryPoint,caseData->size));


	//elements should be 1
	curr_elem = spBPQueuePeek(caseData->queue);
	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	return true;
}

//test case 2
knnTestCaseData initializeKnnTestCase2(){
	//test case 2 data
	SPKDTreeNode kNNtestCase2Tree = NULL;
	SPBPQueue kNNtestCase2Queue = NULL;
	SPPoint kNNtestCase2QueryPoint = NULL;
	SPPoint* kNNtestCase2Points = NULL;
	int kNNtestCase2MaxDim = 2;
	int kNNtestCase2Size = 5;
	SP_KDTREE_SPLIT_METHOD kNNtestCase2SplitMethod = INCREMENTAL;
	int kNNtestCase2K = 3;
	knnTestCaseData caseData = NULL;
	double data1[2] = {1,2},data2[2] = {123,70},data3[2] = {2,7},data4[2] = {9,11},
			data5[2] = {3,4}, queryData[2] = {0,0};
	SPPoint p1,p2,p3,p4,p5;
	SPKDArray kdArr = NULL;

	spCalloc(caseData, knn_test_case_data, 1);

	kNNtestCase2Points = (SPPoint*)calloc(sizeof(SPPoint),kNNtestCase2Size);
	if (kNNtestCase2Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = kNNtestCase2Points;
	caseData->size = kNNtestCase2Size;

	p1 = spPointCreate(data1,kNNtestCase2MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p2 = spPointCreate(data2,kNNtestCase2MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p3 = spPointCreate(data3,kNNtestCase2MaxDim,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p4 = spPointCreate(data4,kNNtestCase2MaxDim,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p5 = spPointCreate(data5,kNNtestCase2MaxDim,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
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
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queryPoint = kNNtestCase2QueryPoint;

	kNNtestCase2Queue = spBPQueueCreate(kNNtestCase2K);
	if (kNNtestCase2Queue == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spPointDestroy(p5);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queue = kNNtestCase2Queue;

	kdArr = Init(kNNtestCase2Points,kNNtestCase2Size);
	kNNtestCase2Tree = InitKDTree(kdArr, kNNtestCase2SplitMethod);
	spKDArrayDestroy(kdArr);

	caseData->k = kNNtestCase2K;
	caseData->split_method = kNNtestCase2SplitMethod;
	caseData->tree = kNNtestCase2Tree;

	return caseData;
}

bool runKnnTestCase2(knnTestCaseData caseData){
	SPListElement curr_elem;
	SP_BPQUEUE_MSG msg;
	bool successFlag;
	if (caseData == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_CASE_DATA,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_CASE_DATA);
		return false;
	}
	if (caseData->tree == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (caseData->points == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queue == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queryPoint == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(caseData->tree, caseData->queue, caseData->queryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(caseData->queue, caseData->k, caseData->points, caseData->queryPoint,caseData->size));


	//first elem should be 1
	curr_elem = spBPQueuePeek(caseData->queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	msg = spBPQueueDequeue(caseData->queue);
	ASSERT_TRUE(msg == SP_BPQUEUE_SUCCESS);

	//second element should be 5
	curr_elem = spBPQueuePeek(caseData->queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 5);
	spListElementDestroy(curr_elem);

	msg = spBPQueueDequeue(caseData->queue);
	ASSERT_TRUE(msg == SP_BPQUEUE_SUCCESS);

	//third element should be 3
	curr_elem = spBPQueuePeek(caseData->queue);

	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 3);
	spListElementDestroy(curr_elem);
	return true;
}

//edge test case 1
knnTestCaseData initializeKnnEdgeTestCase1(){
	//edge case 1 data
	SPKDTreeNode kNNedgeTestCase1Tree = NULL;
	SPBPQueue kNNedgeTestCase1Queue = NULL;
	SPPoint kNNedgeTestCase1QueryPoint = NULL;
	SPPoint* kNNedgeTestCase1Points = NULL;
	int kNNedgeTestCase1MaxDim = 1;
	int kNNedgeTestCase1Size = 1;
	SP_KDTREE_SPLIT_METHOD kNNedgeTestCase1SplitMethod 	= INCREMENTAL;
	int kNNedgeTestCase1K = 1;
	knnTestCaseData caseData = NULL;
	double data1[1] = {2}, queryData[1] = {0};
	SPPoint p1;
	SPKDArray kdArr = NULL;
	spCalloc(caseData, knn_test_case_data, 1);

	kNNedgeTestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),kNNedgeTestCase1Size);
	if (kNNedgeTestCase1Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = kNNedgeTestCase1Points;
	caseData->size = kNNedgeTestCase1Size;

	p1 = spPointCreate(data1,kNNedgeTestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	kNNedgeTestCase1Points[0] = p1;


	kNNedgeTestCase1QueryPoint = spPointCreate(queryData,kNNedgeTestCase1MaxDim,2);
	if (kNNedgeTestCase1QueryPoint == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queryPoint = kNNedgeTestCase1QueryPoint;

	kNNedgeTestCase1Queue = spBPQueueCreate(kNNedgeTestCase1K);
	if (kNNedgeTestCase1Queue == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->queue = kNNedgeTestCase1Queue;


	kdArr = Init(kNNedgeTestCase1Points,kNNedgeTestCase1Size);
	kNNedgeTestCase1Tree = InitKDTree(kdArr, kNNedgeTestCase1SplitMethod);
	spKDArrayDestroy(kdArr);

	caseData->k = kNNedgeTestCase1K;
	caseData->split_method = kNNedgeTestCase1SplitMethod;
	caseData->tree = kNNedgeTestCase1Tree;

	return caseData;
}

bool runKnnEdgeTestCase1(knnTestCaseData caseData){
	SPListElement curr_elem;
	bool successFlag;
	if (caseData == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_CASE_DATA,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_CASE_DATA);
		return false;
	}
	if (caseData->tree == NULL){
		spLoggerSafePrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (caseData->points == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queue == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUEUE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}
	if (caseData->queryPoint == NULL){
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	successFlag = kNearestNeighbors(caseData->tree, caseData->queue, caseData->queryPoint);

	//general success
	ASSERT_TRUE(successFlag);
	ASSERT_TRUE(verifyKNN(caseData->queue, caseData->k, caseData->points, caseData->queryPoint,caseData->size));


	//elements should be 1
	curr_elem = spBPQueuePeek(caseData->queue);
	ASSERT_TRUE(spListElementGetIndex(curr_elem) == 1);
	spListElementDestroy(curr_elem);

	return true;
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

		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_QUERY_POINT);
		return false;
	}

	if (queryPoint == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);

		spLoggerSafePrintError(COULD_NOT_INITIALIZE_QUERY_POINT,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_QUERY_POINT);
		return false;
	}

	if (pointsArray == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerSafePrintError(COULD_NOT_CREATE_POINTS_ARRAY,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}

	kdArr = Init(pointsArray,size);

	if (kdArr == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_KD_ARRAY,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_KD_ARRAY);
		return false;
	}

	tree = InitKDTree(kdArr, splitMethod);

	if (tree == NULL){
		destroyCaseData(tree,kdArr,pointsArray,size,queue,queryPoint);
		spLoggerSafePrintError(
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
	knnTestCaseData case1Data = NULL, case2Data = NULL, edgeCase1Data = NULL;
	int seed = time(NULL);
	srand(seed);

	//null case
	RUN_TEST(runKnnNullTests);

	//case 1
	case1Data = initializeKnnTestCase1();
	RUN_TEST_WITH_PARAM(runKnnTestCase1, case1Data);
	destroyCaseDataByWrapperTestCase(case1Data);


	//case 2
	case2Data = initializeKnnTestCase2();
	RUN_TEST_WITH_PARAM(runKnnTestCase2, case2Data);
	destroyCaseDataByWrapperTestCase(case2Data);


	//edge case 1
	edgeCase1Data = initializeKnnEdgeTestCase1();
	RUN_TEST_WITH_PARAM(runKnnEdgeTestCase1, edgeCase1Data);
	destroyCaseDataByWrapperTestCase(edgeCase1Data);


	//random tests
	for (i = 0 ; i< RANDOM_TESTS_COUNT;i++)
		RUN_TEST(runRandomKnnTest);
}

