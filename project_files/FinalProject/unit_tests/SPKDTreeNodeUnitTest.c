#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "unit_test_util.h"
#include "../SPPoint.h"
#include "../SPConfig.h"
#include "../SPKDArray.h"
#include "../SPLogger.h"
#include "SPKDTreeNodeUnitTest.h"
#include "../SPKDTreeNode.h"
#include "SPKDArrayUnitTest.h"
#include "../SPImagesParser.h"

#define ERROR_AT_TREE_SIZE 									"Error at tree size"
#define ERROR_AT_NODES_STRUCTURE 							"Error at nodes structure"
#define ERROR_AT_TREE_MEDIAN_INVARIANT 						"Error at tree median invariant"
#define ERROR_AT_TREE_ORDER_INVARIANT 						"Error at tree order invariant"
#define ERROR_AT_TREE_DIMENSION_SELECTION_METHOD 			"Error at tree dimension selection method"
#define ERROR_ALLOCATING_MEMORY 							"Error allocating memory"


#define COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST 		"Could not create points array for random test"
#define COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST 		"Could not initialize kd-array for random test"
#define COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST			"Could not initialize tree for random test - InitKDTree returned null"

#define COULD_NOT_CREATE_POINTS_ARRAY 						"Could not create points array"
#define COULD_NOT_INITIALIZE_KD_ARRAY 						"Could not initialize kd-array"
#define COULD_NOT_INITIALIZE_TREE 							"Could not initialize tree - InitKDTree returned null"


//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  							500
#define RANDOM_TESTS_DIM_RANGE 								50
#define RANDOM_TESTS_COUNT 									50

//test case 1 data
SPKDTreeNode testCase1Tree 									= NULL;
SPPoint* testCase1Points 									= NULL;
#define testCase1MaxDim           							3
#define testCase1Size 										2
#define testCase1SplitMethod 								MAX_SPREAD

//test case 2 data
SPKDTreeNode testCase2Tree 									= NULL;
SPPoint* testCase2Points 									= NULL;
#define testCase2MaxDim 									2
#define testCase2Size 										5
#define testCase2SplitMethod 								INCREMENTAL

//edge case 1 data
SPKDTreeNode edgeTestCase1Tree 								= NULL;
SPPoint* edgeTestCase1Points 								= NULL;
#define edgeTestCase1MaxDim 								1
#define edgeTestCase1Size 									1
#define edgeTestCase1SplitMethod 							INCREMENTAL

//verify correct data
bool isNodeDataCorrect(SPKDTreeNode node){
	if (node == NULL)
		return true;
	//node is a leaf and than data != NULL Xor node is not leaf and than it has a son
	return (node->data != NULL && node->val == NULL) ^
			(node->kdtLeft != NULL || node->kdtRight != NULL);
}

bool isKDTreeDataCorrect(SPKDTreeNode treeNode){
	if (treeNode == NULL)
		return true;
	return isNodeDataCorrect(treeNode) && isNodeDataCorrect(treeNode->kdtLeft)
			&& isNodeDataCorrect(treeNode->kdtRight);
}

bool verifyLeaf(SPKDTreeNode treeNode){
	return (treeNode != NULL) &&
		(treeNode->val == NULL) &&
	 (treeNode->kdtLeft == NULL) &&
	 (treeNode->kdtLeft == NULL) &&
	 (treeNode->data != NULL);
}

bool verifyNotLeafAndData(SPKDTreeNode treeNode, int dim, double value){
	if (treeNode == NULL)
		return false;
	return (treeNode->data == NULL) &&
			(treeNode->val != NULL &&
			isEqual(*(treeNode->val),value) ) &&
			(treeNode->dim == dim);
}

//TODO - this should be in sppoint?
bool verifyPointData(SPPoint point, int index, int dim, ...){
	int i;
    va_list args;

	if (point == NULL)
		return false;
	if (spPointGetIndex(point) != index)
		return false;
	if (spPointGetDimension(point) != dim)
		return false;

    va_start(args, dim);

	for (i=0;i<dim;i++){
		if (!isEqual(spPointGetAxisCoor(point, i),va_arg(args,double)))
			return false;
	}

	va_end(args);

	return true;
}

int getTreeNumOfLeaves(SPKDTreeNode treeNode){
	if (treeNode == NULL)
		return 0;
	if (isLeaf(treeNode))
		return 1;
	return getTreeNumOfLeaves(treeNode->kdtLeft) + getTreeNumOfLeaves(treeNode->kdtRight);
}

//search value on leafs
bool safeSearchValueInTree(SPKDTreeNode treeNode,int dim, double value){
	if (treeNode == NULL)
		return false;
	if (isLeaf(treeNode))
		return isEqual(spPointGetAxisCoor(treeNode->data,dim), value);
	return safeSearchValueInTree(treeNode->kdtLeft,dim,value) ||
			safeSearchValueInTree(treeNode->kdtRight,dim,value);
}

bool safeIsTreeGreaterEqualThan(SPKDTreeNode treeNode, int dim, double value){
	if (treeNode == NULL)
		return true;
	if (isLeaf(treeNode))
		return spPointGetAxisCoor(treeNode->data,dim) >= value;
	return safeIsTreeGreaterEqualThan(treeNode->kdtLeft,dim,value) &&
			safeIsTreeGreaterEqualThan(treeNode->kdtRight,dim,value);
}

bool safeIsTreeLessEqualThan(SPKDTreeNode treeNode, int dim, double value){
	if (treeNode == NULL)
		return true;
	if (isLeaf(treeNode))
		return spPointGetAxisCoor(treeNode->data,dim) <= value;
	return safeIsTreeLessEqualThan(treeNode->kdtLeft,dim,value) &&
			safeIsTreeLessEqualThan(treeNode->kdtRight,dim,value);
}

bool verifyOrderedTree(SPKDTreeNode treeNode){
	if (treeNode == NULL || isLeaf(treeNode))
		return true;
	return safeIsTreeLessEqualThan(treeNode->kdtLeft, treeNode->dim, *(treeNode->val))
			&& safeIsTreeGreaterEqualThan(treeNode->kdtRight, treeNode->dim, *(treeNode->val))
			&& verifyOrderedTree(treeNode->kdtLeft)
			&& verifyOrderedTree(treeNode->kdtRight);
}

bool verifyMedianNode(SPKDTreeNode treeNode){
	if (treeNode == NULL || isLeaf(treeNode))
			return true;
	return safeSearchValueInTree(treeNode->kdtLeft,treeNode->dim, *(treeNode->val)) &&
			getTreeNumOfLeaves(treeNode->kdtLeft) >= getTreeNumOfLeaves(treeNode->kdtRight) &&
			getTreeNumOfLeaves(treeNode->kdtLeft) - getTreeNumOfLeaves(treeNode->kdtRight) <= 1;
}

bool verifyMedianTreeNode(SPKDTreeNode treeNode){
	if (treeNode == NULL)
			return true;
	return verifyMedianNode(treeNode) &&
			verifyMedianTreeNode(treeNode->kdtLeft)&&
			verifyMedianTreeNode(treeNode->kdtLeft);
}

int getPositiveModulo(int num, int mod){
	return (num % mod + mod)%mod;
}

bool internalVerifyDimIncremental(SPKDTreeNode treeNode, int depth, int maxDim){
	if (treeNode == NULL || isLeaf(treeNode))
			return true;
	if (treeNode->kdtLeft != NULL && !isLeaf(treeNode->kdtLeft)){
		if (getPositiveModulo(treeNode->kdtLeft->dim  - treeNode->dim, maxDim ) != 1 % maxDim )
			return false;
	}
	if (treeNode->kdtRight != NULL && !isLeaf(treeNode->kdtRight)){
		if (getPositiveModulo(treeNode->kdtRight->dim  - treeNode->dim, maxDim ) != 1 % maxDim)
				return false;
	}
	return internalVerifyDimIncremental(treeNode->kdtLeft,depth + 1,maxDim) &&
			internalVerifyDimIncremental(treeNode->kdtRight,depth + 1,maxDim);
}

bool verifyDimIncremental(SPKDTreeNode treeNode, int maxDim){
	if (treeNode == NULL || isLeaf(treeNode))
			return true;
	return internalVerifyDimIncremental(treeNode,0, maxDim);
}

void allocatePointsArray(SPKDTreeNode treeNode,SPPoint* relevantPoints,int* i){
	if (treeNode == NULL)
		return;
	if (isLeaf(treeNode)){
		relevantPoints[*i] = treeNode->data;
		(*i)++;
	}
	allocatePointsArray(treeNode->kdtLeft, relevantPoints, i);
	allocatePointsArray(treeNode->kdtRight, relevantPoints, i);
}

bool verifyDimMaxSpread(SPKDTreeNode treeNode, int maxDim ){
	SPPoint* relevantPoints;
	int size;
	int i = 0 ,j , dimIndex;
	double suspectedMaxValue = 0;
	double tempValue;

	if (treeNode == NULL || isLeaf(treeNode))
			return true;

	size = getTreeNumOfLeaves(treeNode);
	relevantPoints = (SPPoint*)calloc(sizeof(SPPoint),size);

	if (relevantPoints == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return false;
	}

	allocatePointsArray(treeNode,relevantPoints,&i);

	//get the max spread that the tree selected
	for (i=0;i<size;i++){
		for (j = 0 ; j < size ; j++){
			//check points i,j by dim dimIndex
			tempValue = spPointGetAxisCoor(relevantPoints[i],treeNode->dim) -
					spPointGetAxisCoor(relevantPoints[j],treeNode->dim);
			if (tempValue < 0)
				tempValue = -tempValue;
			if (tempValue > suspectedMaxValue){
				suspectedMaxValue = tempValue;
			}
		}
	}

	//verify that it is indeed the maximum one
	for (dimIndex = 0 ; dimIndex < maxDim ; dimIndex++){
		for (i=0;i<size;i++){
			for (j = 0 ; j < size ; j++){
				//check points i,j by dim dimIndex
				tempValue = spPointGetAxisCoor(relevantPoints[i],dimIndex) -
						spPointGetAxisCoor(relevantPoints[j],dimIndex);
				if (tempValue < 0)
					tempValue = -tempValue;
				if (dimIndex != treeNode->dim && tempValue > suspectedMaxValue){
					free(relevantPoints);
					return false;
				}
			}
		}
	}
	free(relevantPoints);
	return true;
}

bool verifyDimSelection(SPKDTreeNode treeNode, int maxDim,
		SP_KDTREE_SPLIT_METHOD splitMethod){
	switch (splitMethod)
	{
		case RANDOM:
			return treeNode->dim >= 0;
			break;
		case MAX_SPREAD:
			return verifyDimMaxSpread(treeNode, maxDim);
			break;
		case INCREMENTAL:
			return verifyDimIncremental(treeNode, maxDim);
			break;
		default:
			return false;
	}
	return false;
}

bool testKDTree(SPKDTreeNode treeNode, int maxDim,
		SP_KDTREE_SPLIT_METHOD splitMethod, SPPoint* pointsArray, int size){
	if (treeNode == NULL && size == 0)
		return true;
	if (treeNode == NULL)
		return false;
	ASSERT_TRUE(pointsArray != NULL);
	//test tree size
	if (getTreeNumOfLeaves(treeNode) != size){
		spLoggerPrintError(ERROR_AT_TREE_SIZE, __FILE__, __FUNCTION__,
				__LINE__);
		FAIL(ERROR_AT_TREE_SIZE);
		return false;
	}

	//test correct nodes structure
	if (!isKDTreeDataCorrect(treeNode)){
		spLoggerPrintError(ERROR_AT_NODES_STRUCTURE, __FILE__, __FUNCTION__,
				__LINE__);
		FAIL(ERROR_AT_NODES_STRUCTURE);
		return false;
	}

	//test median
	if (!verifyMedianTreeNode(treeNode)){
		spLoggerPrintError(ERROR_AT_TREE_MEDIAN_INVARIANT, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_MEDIAN_INVARIANT);
		return false;
	}

	//test ordered
	if (!verifyOrderedTree(treeNode)){
		spLoggerPrintError(ERROR_AT_TREE_ORDER_INVARIANT, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_ORDER_INVARIANT);
		return false;
	}

	//test dim
	if (!verifyDimSelection(treeNode,maxDim,splitMethod)){
		spLoggerPrintError(ERROR_AT_TREE_DIMENSION_SELECTION_METHOD, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_DIMENSION_SELECTION_METHOD);
		return false;
	}

	return true;
}

bool runRandomKDTreeTest(){
	int maxDim, size;
	bool successFlag = true;
	SPPoint* pointsArray = NULL;
	SPKDArray kdArr = NULL;
	SPKDTreeNode tree = NULL;
	SP_KDTREE_SPLIT_METHOD splitMethod;

	maxDim = 1 + (int)(rand() % RANDOM_TESTS_DIM_RANGE);
	size = 2 + (int)(rand() % RANDOM_TESTS_SIZE_RANGE); //size = 1 is tested at a the edge cases
	splitMethod = (int)(rand()%3);

	pointsArray = generateRandomPointsArray(maxDim,size);


	if (pointsArray == NULL){
		spKDTreeDestroy(tree);
		spKDArrayDestroy(kdArr);
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	kdArr = Init(pointsArray,size);

	if (kdArr == NULL){
		spKDTreeDestroy(tree);
		destroyPointsArray(pointsArray,size);
		spLoggerPrintError(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	tree = InitKDTree(kdArr, splitMethod);

	if (tree == NULL){
		spKDArrayDestroy(kdArr);
		destroyPointsArray(pointsArray,size);
		spLoggerPrintError(
				COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST);
		return false;
	}

	successFlag = testKDTree( tree,  maxDim, splitMethod,  pointsArray,  size);

	spKDTreeDestroy(tree);
	spKDArrayDestroy(kdArr);
	destroyPointsArray(pointsArray,size);

	return successFlag;
}


//null test
bool verifyNullArgument(){
	SP_KDTREE_SPLIT_METHOD splitMethod = 1;
	SPKDTreeNode tree = NULL;
	tree = InitKDTree(NULL, splitMethod);

	ASSERT_TRUE(tree == NULL);
	if (tree != NULL){
		spKDTreeDestroy(tree);
		return false;
	}
	return true;
}

//specific case tests

//test case 1
void initializeTestCase1(){
	double data1[3] = {1,2,3},data2[3] = {5,-7,13};
	SPPoint p1,p2;
	SPKDArray kdArr = NULL;

	testCase1Points = (SPPoint*)calloc(sizeof(SPPoint),testCase1Size);
	if (testCase1Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,testCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,testCase1MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	testCase1Points[0] = p1;
	testCase1Points[1] = p2;


	kdArr = Init(testCase1Points,testCase1Size);
	testCase1Tree = InitKDTree(kdArr, testCase1SplitMethod);
}

bool runTestCase1(){
	SPKDTreeNode left,right;
	if (testCase1Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (testCase1Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	ASSERT_TRUE(testKDTree(testCase1Tree,testCase1MaxDim,
			testCase1SplitMethod,testCase1Points,testCase1Size));

	//root node - not a leaf and split dim should be 2
	ASSERT_TRUE(verifyNotLeafAndData(testCase1Tree,2,3));

	left = testCase1Tree->kdtLeft;
	right = testCase1Tree->kdtRight;

	//left should be a point with value {1,2,3}
	ASSERT_TRUE(verifyLeaf(left));
	ASSERT_TRUE(verifyPointData(left->data, 1, testCase1MaxDim,1.0,2.0,3.0));

	//right should be a point with value {5,-7,13}
	ASSERT_TRUE(verifyLeaf(right));
	ASSERT_TRUE(verifyPointData(right->data, 2, testCase1MaxDim,5.0,-7.0,13.0));

	return true;
}

void destroyTestCase1(){
	destroyPointsArray(testCase1Points,testCase1Size);
	testCase1Points = NULL;

	spKDTreeDestroy(testCase1Tree);
	testCase1Tree = NULL;
}

//test case 2
void initializeTestCase2(){
	double data1[2] = {1,2},data2[2] = {123,70},data3[2] = {2,7},data4[2] = {9,11},
			data5[2] = {3,4};
	SPPoint p1,p2,p3,p4,p5;
	SPKDArray kdArr = NULL;
	testCase2Points = (SPPoint*)calloc(sizeof(SPPoint),testCase2Size);
	if (testCase2Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,testCase2MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p2 = spPointCreate(data2,testCase2MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p3 = spPointCreate(data3,testCase2MaxDim,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p4 = spPointCreate(data4,testCase2MaxDim,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	p5 = spPointCreate(data5,testCase2MaxDim,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	testCase2Points[0] = p1;
	testCase2Points[1] = p2;
	testCase2Points[2] = p3;
	testCase2Points[3] = p4;
	testCase2Points[4] = p5;

	kdArr = Init(testCase2Points,testCase2Size);
	testCase2Tree = InitKDTree(kdArr, testCase2SplitMethod);
}

bool runTestCase2(){
	//l stands for left branch of the tree and r for right branch
	SPKDTreeNode l,r,ll,lr,rl,rr,lll,llr;

	if (testCase2Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (testCase2Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	ASSERT_TRUE(testKDTree(testCase2Tree,testCase2MaxDim,
			testCase2SplitMethod,testCase2Points,testCase2Size));

	//handle root
	ASSERT_TRUE(verifyNotLeafAndData(testCase2Tree,0,3));

	//handle first level
	l = testCase2Tree->kdtLeft;
	r = testCase2Tree->kdtRight;

	ASSERT_TRUE(verifyNotLeafAndData(l,1,4));
	ASSERT_TRUE(verifyNotLeafAndData(r,1,11));

	//handle second level
	ll = l->kdtLeft;
	lr = l->kdtRight;
	rl = r->kdtLeft;
	rr = r->kdtRight;

	ASSERT_TRUE(verifyNotLeafAndData(ll,0,1));

	ASSERT_TRUE(verifyLeaf(lr));
	ASSERT_TRUE(verifyPointData(lr->data, 3, testCase2MaxDim,2.0,7.0));

	ASSERT_TRUE(verifyLeaf(rl));
	ASSERT_TRUE(verifyPointData(rl->data, 4, testCase2MaxDim,9.0,11.0));

	ASSERT_TRUE(verifyLeaf(rr));
	ASSERT_TRUE(verifyPointData(rr->data, 2, testCase2MaxDim,123.0,70.0));


	//handle 3rd level
	lll = ll->kdtLeft;
	llr = ll->kdtRight;

	ASSERT_TRUE(verifyLeaf(lll));
	ASSERT_TRUE(verifyPointData(lll->data, 1, testCase2MaxDim,1.0,2.0));

	ASSERT_TRUE(verifyLeaf(llr));
	ASSERT_TRUE(verifyPointData(llr->data, 5, testCase2MaxDim,3.0,4.0));

	return true;
}

void destroyTestCase2(){
	destroyPointsArray(testCase2Points,testCase2Size);
	testCase2Points = NULL;

	spKDTreeDestroy(testCase2Tree);
	testCase2Tree = NULL;
}

//edge test case 1
void initializeEdgeTestCase1(){
	double data1[1] = {2};
	SPPoint p1;
	SPKDArray kdArr = NULL;

	edgeTestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),edgeTestCase1Size);
	if (edgeTestCase1Points == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}

	p1 = spPointCreate(data1,edgeTestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return;
	}


	edgeTestCase1Points[0] = p1;

	kdArr = Init(edgeTestCase1Points,edgeTestCase1Size);
	edgeTestCase1Tree = InitKDTree(kdArr, edgeTestCase1SplitMethod);
}

bool runEdgeTestCase1(){
	if (edgeTestCase1Tree == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY);
		return false;
	}
	if (edgeTestCase1Points == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_TREE,
						__FILE__, __FUNCTION__,
								__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	ASSERT_TRUE(testKDTree(edgeTestCase1Tree,edgeTestCase1MaxDim,
			edgeTestCase1SplitMethod,edgeTestCase1Points,edgeTestCase1Size));

	//root node - leaf with value {2}
	ASSERT_TRUE(verifyLeaf(edgeTestCase1Tree));
	ASSERT_TRUE(verifyPointData(edgeTestCase1Tree->data, 1, edgeTestCase1MaxDim,2.0));

	return true;
}

void destroyEdgeTestCase1(){
	destroyPointsArray(edgeTestCase1Points,edgeTestCase1Size);
	edgeTestCase1Points = NULL;

	spKDTreeDestroy(edgeTestCase1Tree);
	edgeTestCase1Tree = NULL;
}

void runKDTreeNodeTests(){
	int i;
	int seed = time(NULL);
	srand(seed);

	//null case
	RUN_TEST(verifyNullArgument);

	//case 1
	initializeTestCase1();
	RUN_TEST(runTestCase1);
	destroyTestCase1();

	//case 2
	initializeTestCase2();
	RUN_TEST(runTestCase2);
	destroyTestCase2();

	//edge case 1
	initializeEdgeTestCase1();
	RUN_TEST(runEdgeTestCase1);
	destroyEdgeTestCase1();

	//random tests
	for (i = 0 ; i< RANDOM_TESTS_COUNT;i++)
		RUN_TEST(runRandomKDTreeTest);
}
