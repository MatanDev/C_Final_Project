#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "unit_test_util.h"
#include "SPPoint.h"
#include "SPKDArrayUnitTest.h"
#include "SPKDArray.h"
#include "SPLogger.h"

#define CASE1_NUM_OF_POINTS 2
#define CASE1_DIM 3

#define CASE2_NUM_OF_POINTS 5
#define CASE2_DIM 2

#define EDGE_CASE1_NUM_OF_POINTS 1
#define EDGE_CASE1_DIM 1

#define SPKDARRAY_TESTS_ALLOCATION_ERROR "Error allocating memory at kd-array test unit"
#define SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR "Error initializing point, at kd-array test unit"

SPPoint* case1PointsArray = NULL; //also used in NULL cases tests
SPPoint* case2PointsArray = NULL;
SPPoint* edgeCase1PointsArray = NULL;


void initializePointsArrayCase1(){
	double data1[3] = {1,2,3},data2[3] = {5,-7,13};
	SPPoint p1,p2;
	case1PointsArray = (SPPoint*)calloc(sizeof(SPPoint),CASE1_NUM_OF_POINTS);
	if (case1PointsArray == NULL){
		spLoggerPrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,CASE1_DIM,1);
	if (p1 == NULL){
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,CASE1_DIM,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
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
		spLoggerPrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,CASE2_DIM,1);
	if (p1 == NULL){
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,CASE2_DIM,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p3 = spPointCreate(data3,CASE2_DIM,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p4 = spPointCreate(data4,CASE2_DIM,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p5 = spPointCreate(data5,CASE2_DIM,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
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
		spLoggerPrintError(SPKDARRAY_TESTS_ALLOCATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,EDGE_CASE1_DIM,1);
	if (p1 == NULL){
		spLoggerPrintError(SPKDARRAY_TESTS_POINT_INITIALIZATION_ERROR,__FILE__,__FUNCTION__,__LINE__);
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
	SPKDArrayPair kdArrPair;

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

	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(kdArrPair);
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
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][1] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][2] == 0);

	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == CASE1_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 2);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 5));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,1), -7));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,2), 13));

	//left is 1x3 with point {1,2,3} and index 1
	ASSERT_TRUE((kdArrPair->kdLeft)!=NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->dim == CASE1_DIM);
	ASSERT_TRUE((kdArrPair->kdLeft)->size == 1);
	ASSERT_TRUE((kdArrPair->kdLeft)->indicesMatrix != NULL);

	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][0] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][1] == 0);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][2] == 0);

	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == CASE1_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 1);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 1));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,1), 2));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,2), 3));


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
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[2]) == 4);
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
	ASSERT_TRUE((kdArrPair->kdLeft)!=NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->dim == CASE2_DIM);
	ASSERT_TRUE((kdArrPair->kdLeft)->size == 2);
	ASSERT_TRUE((kdArrPair->kdLeft)->indicesMatrix != NULL);
	ASSERT_TRUE((kdArrPair->kdLeft)->pointsArray != NULL);

	// points array should be : {123,70}[2] , {9,11}[4]
	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[0]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[0]) == 2);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,0), 123));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[0] ,1), 70));

	ASSERT_TRUE(spPointGetDimension(((kdArrPair->kdLeft)->pointsArray)[1]) == CASE2_DIM);
	ASSERT_TRUE(spPointGetIndex(((kdArrPair->kdLeft)->pointsArray)[1]) == 4);
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[1] ,0), 9));
	ASSERT_TRUE(isEqual(spPointGetAxisCoor(((kdArrPair->kdLeft)->pointsArray)[1] ,1), 11));


	// matrix should be {{1,0},{1,0}}
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][0] == 1);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[0][1] == 0);

	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][0] == 1);
	ASSERT_TRUE(((kdArrPair->kdLeft)->indicesMatrix)[1][1] == 0);


	spKDArrayDestroy(kdArr);
	spKDArrayPairDestroy(kdArrPair);
	return true;
}




void runKDArrayTests(){
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
}
