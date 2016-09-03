#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "unit_test_util.h"
#include "../SPPoint.h"
#include "SPKDArrayUnitTest.h"
#include "../data_structures/kd_ds/SPKDArray.h"
#include "../SPLogger.h"
#include "../image_parsing/SPImagesParser.h"

#define SPKDARRAY_TESTS_ALLOCATION_ERROR 				"Error allocating memory at kd-array test unit"
#define SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR 		"Error initializing point, at kd-array test unit"

//test case 1 macros
#define CASE1_NUM_OF_POINTS                             2
#define CASE1_DIM                                       3

//test case 2 macros
#define CASE2_NUM_OF_POINTS                             5
#define CASE2_DIM                                       2

//edge case 1 macros
#define EDGE_CASE1_NUM_OF_POINTS                        1
#define EDGE_CASE1_DIM                                  1

//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  						200
#define RANDOM_TESTS_DIM_RANGE 							50
#define RANDOM_TESTS_COUNT 								20

//random test case logging macros
#define enableLoggingOnRandomTests 						false
#define MSG_MAX_SIZE 									2048
#define DEBUG_LOG_FORMAT_RANDOM_TEST_SETTINGS 			"Settings :\n dim %d\n size %d\n selected splitting dim %d\n"
#define DEBUG_LOG_FORMAT_RANDOM_TEST_START 				"Start random case #%d"
#define DEBUG_LOG_FORMAT_RANDOM_TEST_END 				"Success. end random test case."




//global variables
int randomTestsIndex = 0;
SPPoint* case1PointsArray = NULL; //also used in NULL cases tests
SPPoint* case2PointsArray = NULL;
SPPoint* edgeCase1PointsArray = NULL;


void initializePointsArrayCase1(){
	double data1[3] = {1,2,3},data2[3] = {5,-7,13};
	SPPoint p1,p2;
	case1PointsArray = (SPPoint*)calloc(sizeof(SPPoint),CASE1_NUM_OF_POINTS);
	if (case1PointsArray == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,CASE1_DIM,1);
	if (p1 == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,CASE1_DIM,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	case1PointsArray[0] = p1;
	case1PointsArray[1] = p2;
}

void initializePointsArrayCase2(){
	double data1[2] = {1,2},data2[2] = {123,70},data3[2] = {2,7},data4[2] = {9,11}, data5[2] = {3,4};
	SPPoint p1,p2,p3,p4,p5;
	case2PointsArray = (SPPoint*)calloc(sizeof(SPPoint),CASE2_NUM_OF_POINTS);
	if (case2PointsArray == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,CASE2_DIM,1);
	if (p1 == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,CASE2_DIM,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p3 = spPointCreate(data3,CASE2_DIM,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p4 = spPointCreate(data4,CASE2_DIM,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p5 = spPointCreate(data5,CASE2_DIM,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	case2PointsArray[0] = p1;
	case2PointsArray[1] = p2;
	case2PointsArray[2] = p3;
	case2PointsArray[3] = p4;
	case2PointsArray[4] = p5;
}

void initializePointsArrayEdgeCase1(){
	double data1[1] = {2};
	SPPoint p1;
	edgeCase1PointsArray = (SPPoint*)calloc(sizeof(SPPoint),EDGE_CASE1_NUM_OF_POINTS);
	if (edgeCase1PointsArray == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,EDGE_CASE1_DIM,1);
	if (p1 == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}


	edgeCase1PointsArray[0] = p1;
}

void destroyPointsArray(SPPoint* array, int numOfItems){
	int i;
	if (array != NULL){
		for (i=0;i<numOfItems;i++ ){
			spPointDestroy(array[i]);
		}
		free(array);
	}
}

SPPoint generateRandomPoint(int dim, int index) {
	int i;
	SPPoint p;
	double* data = (double*)calloc(dim, sizeof(double));
	if (data == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	for (i = 0; i < dim; i++) {
		data[i] = -20  + ((double)rand() / ((double)RAND_MAX / 100));
	}
	p = spPointCreate(data, dim, index);
	free(data);
	return p;
}

SPPoint* generateRandomPointsArray(int dim, int size){
	SPPoint* arr = NULL;
	int i,j;
	arr = (SPPoint*)calloc(sizeof(SPPoint),size);

	if (arr == NULL){
		spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	for (i = 0 ; i< size ; i++){
		arr[i] = generateRandomPoint(dim, i);
		if (arr[i] == NULL){
			//allocation error - roll back
			for (j=0;j<i;j++){
				spPointDestroy(arr[j]);
			}
			free(arr);
			spLoggerSafePrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
			return NULL;
		}
	}
	return arr;
}

bool isIndexesMatrixSorted(SPKDArray kdArr){
	int i,j;
	if (kdArr == NULL) // when splitting size 1
		return true;
	for (i = 0;i<kdArr->dim; i++){
		for (j = 0 ; j < kdArr->size - 1; j++){
			if (spPointGetAxisCoor(kdArr->pointsArray[kdArr->indicesMatrix[i][j]], i)>
						spPointGetAxisCoor(kdArr->pointsArray[kdArr->indicesMatrix[i][j+1]], i)){
				return false;
			}
		}
	}
	return true;
}

bool isSimilarIndexes(SPPoint* array, int size){
	int i,j;
	for (i = 0;i<size;i++){
		for ( j = i+1 ; j< size ; j++){
			if (spPointGetIndex(array[i]) == spPointGetIndex(array[j])){
				return false;
			}
		}
	}
	return true;
}

bool verifyArraySizes(int father_size, int left_size, int right_size){
	if (father_size != left_size + right_size){
		return false;
	}
	if (right_size > left_size){
		return false;
	}
	if (right_size - left_size > 1){
		return false;
	}
	return true;
}

bool searchIndex(SPPoint* array, int size, int index){
	int i;
	for (i=0;i<size;i++){
		if (spPointGetIndex(array[i]) == index){
			return true;
		}
	}
	return false;
}

bool isComplete(SPPoint* arr, SPPoint* left, SPPoint* right, int size, int left_size, int right_size){
	int i, index;
	bool foundItem;
	for (i=0;i<size;i++){
		index = spPointGetIndex(arr[i]);
		foundItem = (searchIndex(left, left_size,index) || searchIndex(right, right_size,index));
		if (!foundItem){
			return false;
		}
	}
	return true;
}

static bool testInitNullCases(){
	SPKDArray rsltArray = NULL;

	rsltArray = Init(NULL,4);
	ASSERT_TRUE(rsltArray == NULL);
	if (rsltArray != NULL){
		spKDArrayDestroy(rsltArray);
	}

	rsltArray = Init(case1PointsArray,0);
	ASSERT_TRUE(rsltArray == NULL);
	if (rsltArray != NULL){
		spKDArrayDestroy(rsltArray);
		return false;
	}

	rsltArray = Init(case1PointsArray,-1);
	ASSERT_TRUE(rsltArray == NULL);
	if (rsltArray != NULL){
		spKDArrayDestroy(rsltArray);
		return false;
	}

	return true;
}

static bool testInitEdgeCase1(){
	SPKDArray kdArr = NULL;
	kdArr = Init(edgeCase1PointsArray,EDGE_CASE1_NUM_OF_POINTS);

	ASSERT_TRUE(kdArr!=NULL);

	ASSERT_TRUE(kdArr->dim ==EDGE_CASE1_DIM);
	ASSERT_TRUE(kdArr->size ==EDGE_CASE1_NUM_OF_POINTS);
	ASSERT_TRUE(kdArr->indicesMatrix != NULL);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE(kdArr->pointsArray != NULL);
	ASSERT_TRUE(spPointGetDimension((kdArr->pointsArray)[0]) == EDGE_CASE1_DIM);
	ASSERT_TRUE(spPointGetIndex((kdArr->pointsArray)[0]) == 1);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,0), 2));

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));

	spKDArrayDestroy(kdArr);

	return true;
}

static bool testInitCase1(){
	SPKDArray kdArr = NULL;
	kdArr = Init(case1PointsArray,CASE1_NUM_OF_POINTS);

	ASSERT_TRUE(kdArr!=NULL);

	ASSERT_TRUE(kdArr->dim == CASE1_DIM);
	ASSERT_TRUE(kdArr->size == CASE1_NUM_OF_POINTS);

	ASSERT_TRUE(kdArr->pointsArray != NULL);

	ASSERT_TRUE(spPointGetDimension((kdArr->pointsArray)[0]) == CASE1_DIM);
	ASSERT_TRUE(spPointGetDimension((kdArr->pointsArray)[1]) == CASE1_DIM);

	ASSERT_TRUE(spPointGetIndex((kdArr->pointsArray)[0]) == 1);
	ASSERT_TRUE(spPointGetIndex((kdArr->pointsArray)[1]) == 2);

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,0), 1));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,1), 2));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,2), 3));

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[1] ,0), 5));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[1] ,1), -7));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[1] ,2), 13));

	ASSERT_TRUE(kdArr->indicesMatrix != NULL);

	ASSERT_TRUE((kdArr->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][1] == 1);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][0] == 1);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][1] == 0);
	ASSERT_TRUE((kdArr->indicesMatrix)[2][0] == 0);
	ASSERT_TRUE((kdArr->indicesMatrix)[2][1] == 1);

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));
	spKDArrayDestroy(kdArr);

	return true;
}

static bool testInitCase2(){
	SPKDArray kdArr = NULL;
	kdArr = Init(case2PointsArray,CASE2_NUM_OF_POINTS);
	int i;

	ASSERT_TRUE(kdArr!=NULL);

	ASSERT_TRUE(kdArr->dim == CASE2_DIM);
	ASSERT_TRUE(kdArr->size == CASE2_NUM_OF_POINTS);

	ASSERT_TRUE(kdArr->pointsArray != NULL);

	for (i=0;i<CASE2_NUM_OF_POINTS;i++){
		ASSERT_TRUE(spPointGetDimension((kdArr->pointsArray)[i]) == CASE2_DIM);
		ASSERT_TRUE(spPointGetIndex((kdArr->pointsArray)[i]) == (i+1));
	}

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,0), 1));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[0] ,1), 2));

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[1] ,0), 123));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[1] ,1), 70));

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[2] ,0), 2));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[2] ,1), 7));

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[3] ,0), 9));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[3] ,1), 11));

	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[4] ,0), 3));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor((kdArr->pointsArray)[4] ,1), 4));


	ASSERT_TRUE(kdArr->indicesMatrix != NULL);

	ASSERT_TRUE((kdArr->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][1] == 2);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][2] == 4);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][3] == 3);
	ASSERT_TRUE((kdArr->indicesMatrix)[0][4] == 1);

	ASSERT_TRUE((kdArr->indicesMatrix)[1][0] == 0);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][1] == 4);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][2] == 2);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][3] == 3);
	ASSERT_TRUE((kdArr->indicesMatrix)[1][4] == 1);

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));
	spKDArrayDestroy(kdArr);

	return true;
}

static bool testSplitNullCases(){
	SPKDArrayPair rsltPair = NULL;
	SPKDArray kdArr = NULL;

	rsltPair = Split(NULL,4);
	ASSERT_TRUE(rsltPair == NULL);
	if (rsltPair != NULL){
		spKDArrayPairDestroy(rsltPair);
	}

	kdArr = Init(case1PointsArray, CASE1_NUM_OF_POINTS);
	ASSERT_TRUE(kdArr != NULL);

	rsltPair = Split(kdArr,-1);
	ASSERT_TRUE(rsltPair == NULL);

	if (rsltPair != NULL){
		spKDArrayPairDestroy(rsltPair);
		spKDArrayDestroy(kdArr);
		return false;
	}

	spKDArrayDestroy(kdArr);
	return true;
}

static bool testSplitEdgeCase1(){
	SPKDArray kdArr = NULL;
	SPKDArrayPair kdArrPair = NULL;

	kdArr = Init(edgeCase1PointsArray,EDGE_CASE1_NUM_OF_POINTS);
	ASSERT_TRUE(kdArr != NULL);

	kdArrPair = Split(kdArr,0);
	ASSERT_TRUE(kdArrPair != NULL);

	//left is 1x1 with value '2.0'
	ASSERT_TRUE((kdArrPair->kdLeft)!=NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->dim ==EDGE_CASE1_DIM);
	ASSERT_TRUE((kdArrPair->kdLeft)->size ==1);
	ASSERT_TRUE((kdArrPair->kdLeft)->indicesMatrix != NULL);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 1);
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == EDGE_CASE1_DIM);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 2));

	//right should be empty, thus null
	ASSERT_TRUE(kdArrPair->kdRight == NULL);

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdLeft));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdRight));

	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(kdArrPair);
	kdArr = NULL;
	kdArrPair = NULL;
	return true;
}

static bool testSplitCase1(){
	SPKDArray kdArr = NULL;
	SPKDArrayPair kdArrPair;

	kdArr = Init(case1PointsArray,CASE1_NUM_OF_POINTS);
	ASSERT_TRUE(kdArr != NULL);

	kdArrPair = Split(kdArr,1);
	ASSERT_TRUE(kdArrPair != NULL);

	//left is 1x3 with point {5,-7,13} and index 2
	ASSERT_TRUE((kdArrPair->kdLeft)!=NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->dim == CASE1_DIM);
	ASSERT_TRUE((kdArrPair->kdLeft)->size == 1);
	ASSERT_TRUE((kdArrPair->kdLeft)->indicesMatrix != NULL);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[2][0] == 0);

	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == CASE1_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 2);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 5));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,1), -7));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,2), 13));

	//right is 1x3 with point {1,2,3} and index 1
	ASSERT_TRUE((kdArrPair->kdRight)!=NULL);
	ASSERT_TRUE((kdArrPair->kdRight)->dim == CASE1_DIM);
	ASSERT_TRUE((kdArrPair->kdRight)->size == 1);
	ASSERT_TRUE((kdArrPair->kdRight)->indicesMatrix != NULL);

	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[1][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[2][0] == 0);

	ASSERT_TRUE((kdArrPair->kdRight)->pointsArray != NULL);
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdRight)->pointsArray)[0]) == CASE1_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdRight)->pointsArray)[0]) == 1);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[0] ,0), 1));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[0] ,1), 2));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[0] ,2), 3));

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdLeft));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdRight));

	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(kdArrPair);
	return true;
}

static bool testSplitCase2(){
	SPKDArray kdArr = NULL;
	SPKDArrayPair kdArrPair;

	kdArr = Init(case2PointsArray,CASE2_NUM_OF_POINTS);
	ASSERT_TRUE(kdArr != NULL);

	kdArrPair = Split(kdArr,0);
	ASSERT_TRUE(kdArrPair != NULL);

	//left
	ASSERT_TRUE((kdArrPair->kdLeft)!=NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->dim == CASE2_DIM);
	ASSERT_TRUE((kdArrPair->kdLeft)->size == 3);
	ASSERT_TRUE((kdArrPair->kdLeft)->indicesMatrix != NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);

	// points array should be : {1,2}[1] , {2,7}[3] , {3,4}[5]
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 1);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 1));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,1), 2));

	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[1]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[1]) == 3);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[1] ,0), 2));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[1] ,1), 7));

	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[2]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[2]) == 5);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[2] ,0), 3));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[2] ,1), 4));

	// matrix should be {{0,1,2},{0,2,1}}
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][1] == 1);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][2] == 2);

	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][1] == 2);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][2] == 1);

	//right
	ASSERT_TRUE((kdArrPair->kdRight)!=NULL);
	ASSERT_TRUE((kdArrPair->kdRight)->dim == CASE2_DIM);
	ASSERT_TRUE((kdArrPair->kdRight)->size == 2);
	ASSERT_TRUE((kdArrPair->kdRight)->indicesMatrix != NULL);
	ASSERT_TRUE((kdArrPair->kdRight)->pointsArray != NULL);

	// points array should be : {123,70}[2] , {9,11}[4]
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdRight)->pointsArray)[0]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdRight)->pointsArray)[0]) == 2);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[0] ,0), 123));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[0] ,1), 70));

	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdRight)->pointsArray)[1]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdRight)->pointsArray)[1]) == 4);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[1] ,0), 9));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdRight)->pointsArray)[1] ,1), 11));


	// matrix should be {{1,0},{1,0}}
	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[0][0] == 1);
	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[0][1] == 0);

	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[1][0] == 1);
	ASSERT_TRUE(((kdArrPair->kdRight)->indicesMatrix)[1][1] == 0);

	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdLeft));
	ASSERT_TRUE(isIndexesMatrixSorted(kdArrPair->kdRight));
	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(kdArrPair);
	return true;
}

void logRandomTestSettings(int dim,int size,int splitting_dim){
	char message[MSG_MAX_SIZE];
	sprintf(message , DEBUG_LOG_FORMAT_RANDOM_TEST_SETTINGS, dim,size,splitting_dim);
	spLoggerSafePrintMsg(message);
}

char* pointsArrayToString(SPPoint* points,int size){
	int i;
	SP_DP_MESSAGES msg;
	char* message;
	char tempMsg[MSG_MAX_SIZE*RANDOM_TESTS_DIM_RANGE] ;
	char* tempPointString;

	message = (char*)calloc(sizeof(char),MSG_MAX_SIZE*RANDOM_TESTS_SIZE_RANGE*RANDOM_TESTS_DIM_RANGE);

	for (i=0;i<size;i++){
		tempPointString = pointToString(points[i], &msg);
		sprintf(tempMsg,"[%d] : %s",spPointGetIndex(points[i]),tempPointString);
		strcat(message,tempMsg);
	}

	return message;
}

void logPointsArray(SPPoint* points, int size){
	char message[MSG_MAX_SIZE*RANDOM_TESTS_SIZE_RANGE*RANDOM_TESTS_DIM_RANGE], *tempMsg;
	tempMsg = pointsArrayToString(points,size);
	sprintf(message, "Generated Points are :\n %s",tempMsg);
	spLoggerSafePrintMsg(message);
	free(tempMsg);
}

char* indicesMatrixToString(SPKDArray kdArr) {
	char* ret = (char *)malloc(4096);
	char* curr = ret;
	char buf[4096];
	int j, i;
	for (j = 0; j < kdArr->dim; j++) {
		for (i = 0; i < kdArr->size; i++) {
			sprintf(buf, "%d ", kdArr->indicesMatrix[j][i]);
			memcpy(curr, buf, strlen(buf));
			curr += strlen(buf);
		}
		curr[0] = '\n';
		curr++;
	}
	curr[0] = '\0';
	return ret;
}

void logKDArray(SPKDArray kdArr) {
	char buf[4096];
	char* tempString;
	if (kdArr != NULL) {
		sprintf(buf, "kdArr pointer: %p", (void *)kdArr);
		spLoggerSafePrintMsg(buf);
		sprintf(buf, "dim: %d", kdArr->dim);
		spLoggerSafePrintMsg(buf);
		sprintf(buf, "size: %d", kdArr->size);
		spLoggerSafePrintMsg(buf);
		if (kdArr->pointsArray != NULL) {
			tempString = pointsArrayToString(kdArr->pointsArray,kdArr->size);
			sprintf(buf, "points array:\n %s",tempString);
			spLoggerSafePrintMsg(buf);
			free(tempString);
		}
		else
			spLoggerSafePrintMsg("points array: NULL");
		if (kdArr->indicesMatrix != NULL) {
			sprintf(buf, "indices matrix:\n %s",
					indicesMatrixToString(kdArr));
			spLoggerSafePrintMsg(buf);
		}
		else
			spLoggerSafePrintMsg("indices matrix: NULL");
	}
	else {
		spLoggerSafePrintMsg("kd array is null");
	}
}

void logKDPair(SPKDArrayPair kdArrPair) {
	char buf[1024];
	if (kdArrPair != NULL) {
		sprintf(buf, "kdArrPair pointer: %p", (void *)kdArrPair);
		spLoggerSafePrintMsg(buf);
		if (kdArrPair->kdLeft != NULL){
			spLoggerSafePrintMsg("Left kd array : \n");
			logKDArray(kdArrPair->kdLeft);
		}
		else
			spLoggerSafePrintMsg("kdArrPair->kdLeft: NULL");
		if (kdArrPair->kdRight != NULL){
			spLoggerSafePrintMsg("Right kd array : \n");
			logKDArray(kdArrPair->kdRight);
		}
		else
			spLoggerSafePrintMsg("kdArrPair->kdRight: NULL");
	}
	else {
		spLoggerSafePrintMsg("kd pair is null");
	}
}

bool commitRandomTest(){
	int dim, size, splitting_dim;
	char startMessage[MSG_MAX_SIZE];
	SPPoint* points = NULL;
	SPKDArray kdArr = NULL;
	SPKDArrayPair rsltPair = NULL;
	if (enableLoggingOnRandomTests){
		sprintf(startMessage, DEBUG_LOG_FORMAT_RANDOM_TEST_START, randomTestsIndex+1);
		spLoggerSafePrintMsg(startMessage);
	}


	dim = 1 + (int)(rand() % RANDOM_TESTS_DIM_RANGE);
	size = 2 + (int)(rand() % RANDOM_TESTS_SIZE_RANGE); //size = 1 is tested at a the edge cases
	splitting_dim = (int)(rand() % dim);
	if (enableLoggingOnRandomTests)
		logRandomTestSettings(dim,size,splitting_dim);

	points = generateRandomPointsArray(dim,size);
	if (enableLoggingOnRandomTests)
		logPointsArray(points, size);

	ASSERT_TRUE(points != NULL);
	if (points == NULL)
		return false;

	kdArr = Init(points,size);
	ASSERT_TRUE(kdArr != NULL);
	if (kdArr == NULL){
		destroyPointsArray(points, size);
		return false;
	}

	if (enableLoggingOnRandomTests)
		logKDArray(kdArr);
	ASSERT_TRUE(isIndexesMatrixSorted(kdArr));


	rsltPair = Split(kdArr, splitting_dim);
	if (rsltPair == NULL){
		destroyPointsArray(points, size);
		spKDArrayDestroy(kdArr);
		return false;
	}
	if (enableLoggingOnRandomTests)
		logKDPair(rsltPair);

	ASSERT_TRUE(verifyArraySizes(kdArr->size, rsltPair->kdLeft->size, rsltPair->kdRight->size));
	ASSERT_TRUE(isComplete(kdArr->pointsArray, rsltPair->kdLeft->pointsArray, rsltPair->kdRight->pointsArray,
			kdArr->size, rsltPair->kdLeft->size, rsltPair->kdRight->size));
	ASSERT_TRUE(isIndexesMatrixSorted(rsltPair->kdLeft));
	ASSERT_TRUE(isIndexesMatrixSorted(rsltPair->kdRight));

	destroyPointsArray(points, size);
	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(rsltPair);
	if (enableLoggingOnRandomTests)
		spLoggerSafePrintMsg(DEBUG_LOG_FORMAT_RANDOM_TEST_END);

	return true;

}

void runKDArrayTests(){
	int seed = time(NULL);
	srand(seed);

	//CASE 1 + NULL CASES
	initializePointsArrayCase1();
	if (case1PointsArray == NULL)
		return;

	RUN_TEST(testInitNullCases);
	RUN_TEST(testSplitNullCases);
	RUN_TEST(testInitCase1);
	RUN_TEST(testSplitCase1);

	destroyPointsArray(case1PointsArray, CASE1_NUM_OF_POINTS);
	case1PointsArray = NULL;

	//EDGE CASE 1
	initializePointsArrayEdgeCase1();
	if (edgeCase1PointsArray == NULL)
		return;

	RUN_TEST(testInitEdgeCase1);
	RUN_TEST(testSplitEdgeCase1);

	destroyPointsArray(edgeCase1PointsArray, EDGE_CASE1_NUM_OF_POINTS);
	edgeCase1PointsArray = NULL;

	//CASE 2
	initializePointsArrayCase2();
	if (case2PointsArray == NULL)
		return;

	RUN_TEST(testInitCase2);
	RUN_TEST(testSplitCase2);

	destroyPointsArray(case2PointsArray, CASE2_NUM_OF_POINTS);
	case2PointsArray = NULL;

	//Random tests
	for (randomTestsIndex = 0 ; randomTestsIndex<RANDOM_TESTS_COUNT;randomTestsIndex++){
		RUN_TEST(commitRandomTest);
	}
}
