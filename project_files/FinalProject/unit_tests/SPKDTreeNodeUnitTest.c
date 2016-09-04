#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "unit_test_util.h"
#include "../SPPoint.h"
#include "../SPConfig.h"
#include "../data_structures/kd_ds/SPKDArray.h"
#include "../SPLogger.h"
#include "SPKDTreeNodeUnitTest.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"
#include "SPKDArrayUnitTest.h"
#include "../image_parsing/SPImagesParser.h"
#include "../general_utils/SPUtils.h"

#define ERROR_AT_TREE_SIZE 									"Error at tree size"
#define ERROR_AT_NODES_STRUCTURE 							"Error at nodes structure"
#define ERROR_AT_TREE_MEDIAN_INVARIANT 						"Error at tree median invariant"
#define ERROR_AT_TREE_ORDER_INVARIANT 						"Error at tree order invariant"
#define ERROR_AT_TREE_DIMENSION_SELECTION_METHOD 			"Error at tree dimension selection method"

#define COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST 		"Could not create points array for random test"
#define COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST 		"Could not initialize kd-array for random test"
#define COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST			"Could not initialize tree for random test - InitKDTree returned null"

#define COULD_NOT_CREATE_POINTS_ARRAY 						"Could not create points array"
#define COULD_NOT_CREATE_CASE_DATA							"Could not create case data"
#define COULD_NOT_INITIALIZE_KD_ARRAY 						"Could not initialize kd-array"
#define COULD_NOT_INITIALIZE_TREE 							"Could not initialize tree - InitKDTree returned null"

typedef struct tree_node_test_case_data {
	SPKDTreeNode tree;
	SPPoint* points;
	int max_dim;
	int size;
	SP_KDTREE_SPLIT_METHOD split_method;
} tree_node_test_case_data;

typedef struct tree_node_test_case_data* treeNodeTestCaseData;

//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  							500
#define RANDOM_TESTS_DIM_RANGE 								50
#define RANDOM_TESTS_COUNT 									15

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
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
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
		spLoggerSafePrintError(ERROR_AT_TREE_SIZE, __FILE__, __FUNCTION__,
				__LINE__);
		FAIL(ERROR_AT_TREE_SIZE);
		return false;
	}

	//test correct nodes structure
	if (!isKDTreeDataCorrect(treeNode)){
		spLoggerSafePrintError(ERROR_AT_NODES_STRUCTURE, __FILE__, __FUNCTION__,
				__LINE__);
		FAIL(ERROR_AT_NODES_STRUCTURE);
		return false;
	}

	//test median
	if (!verifyMedianTreeNode(treeNode)){
		spLoggerSafePrintError(ERROR_AT_TREE_MEDIAN_INVARIANT, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_MEDIAN_INVARIANT);
		return false;
	}

	//test ordered
	if (!verifyOrderedTree(treeNode)){
		spLoggerSafePrintError(ERROR_AT_TREE_ORDER_INVARIANT, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_ORDER_INVARIANT);
		return false;
	}

	//test dim
	if (!verifyDimSelection(treeNode,maxDim,splitMethod)){
		spLoggerSafePrintError(ERROR_AT_TREE_DIMENSION_SELECTION_METHOD, __FILE__,
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
		spKDTreeDestroy(tree, false);
		spKDArrayDestroy(kdArr);
		spLoggerSafePrintError(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	kdArr = Init(pointsArray,size);

	if (kdArr == NULL){
		spKDTreeDestroy(tree, false);
		destroyPointsArray(pointsArray,size);
		spLoggerSafePrintError(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	tree = InitKDTree(kdArr, splitMethod);

	if (tree == NULL){
		spKDArrayDestroy(kdArr);
		destroyPointsArray(pointsArray,size);
		spLoggerSafePrintError(
				COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE_FOR_RANDOM_TEST);
		return false;
	}

	successFlag = testKDTree( tree,  maxDim, splitMethod,  pointsArray,  size);

	spKDTreeDestroy(tree, false);
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
		spKDTreeDestroy(tree, false);
		return false;
	}
	return true;
}

//specific case tests

//test case 1
treeNodeTestCaseData initializeTestCase1(){
	SPKDTreeNode testCase1Tree = NULL;
	SPPoint* testCase1Points = NULL;
	int testCase1MaxDim = 3;
	int testCase1Size =	2;
	SP_KDTREE_SPLIT_METHOD testCase1SplitMethod = MAX_SPREAD;
	treeNodeTestCaseData caseData = NULL;
	double data1[3] = {1,2,3},data2[3] = {5,-7,13};
	SPPoint p1,p2;
	SPKDArray kdArr = NULL;
	spCalloc(caseData, tree_node_test_case_data, 1);

	testCase1Points = (SPPoint*)calloc(sizeof(SPPoint),testCase1Size);
	if (testCase1Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = testCase1Points;
	caseData->size = testCase1Size;

	p1 = spPointCreate(data1,testCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p2 = spPointCreate(data2,testCase1MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}

	testCase1Points[0] = p1;
	testCase1Points[1] = p2;


	kdArr = Init(testCase1Points,testCase1Size);
	testCase1Tree = InitKDTree(kdArr, testCase1SplitMethod);
	spKDArrayDestroy(kdArr);


	caseData->max_dim = testCase1MaxDim;
	caseData->split_method = testCase1SplitMethod;
	caseData->tree = testCase1Tree;


	return caseData;
}

bool runTestCase1(treeNodeTestCaseData caseData){
	SPKDTreeNode left,right;
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

	ASSERT_TRUE(testKDTree(caseData->tree,caseData->max_dim,
			caseData->split_method,caseData->points,caseData->size));

	//root node - not a leaf and split dim should be 2
	ASSERT_TRUE(verifyNotLeafAndData(caseData->tree,2,3));

	left = caseData->tree->kdtLeft;
	right = caseData->tree->kdtRight;

	//left should be a point with value {1,2,3}
	ASSERT_TRUE(verifyLeaf(left));
	ASSERT_TRUE(verifyPointData(left->data, 1, caseData->max_dim,1.0,2.0,3.0));

	//right should be a point with value {5,-7,13}
	ASSERT_TRUE(verifyLeaf(right));
	ASSERT_TRUE(verifyPointData(right->data, 2, caseData->max_dim,5.0,-7.0,13.0));

	return true;
}


void destroyTestCase(treeNodeTestCaseData testCase) {
	if (testCase){
		destroyPointsArray(testCase->points, testCase->size);
		testCase->points = NULL;
		spKDTreeDestroy(testCase->tree, true);
		free(testCase);
	}
}

//test case 2
treeNodeTestCaseData initializeTestCase2(){
	//test case 2 data
	SPKDTreeNode testCase2Tree = NULL;
	SPPoint* testCase2Points = NULL;
	int testCase2MaxDim = 2;
	int testCase2Size = 5;
	SP_KDTREE_SPLIT_METHOD testCase2SplitMethod = INCREMENTAL;
	treeNodeTestCaseData caseData = NULL;
	double data1[2] = {1,2},data2[2] = {123,70},data3[2] = {2,7},data4[2] = {9,11},
			data5[2] = {3,4};
	SPPoint p1,p2,p3,p4,p5;
	SPKDArray kdArr = NULL;
	spCalloc(caseData, tree_node_test_case_data, 1);

	testCase2Points = (SPPoint*)calloc(sizeof(SPPoint),testCase2Size);
	if (testCase2Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = testCase2Points;
	caseData->size = testCase2Size;

	p1 = spPointCreate(data1,testCase2MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p2 = spPointCreate(data2,testCase2MaxDim,2);
	if (p2 == NULL){
		spPointDestroy(p1);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p3 = spPointCreate(data3,testCase2MaxDim,3);
	if (p3 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p4 = spPointCreate(data4,testCase2MaxDim,4);
	if (p4 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	p5 = spPointCreate(data5,testCase2MaxDim,5);
	if (p5 == NULL){
		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);
		spPointDestroy(p4);
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	testCase2Points[0] = p1;
	testCase2Points[1] = p2;
	testCase2Points[2] = p3;
	testCase2Points[3] = p4;
	testCase2Points[4] = p5;

	kdArr = Init(testCase2Points,testCase2Size);
	testCase2Tree = InitKDTree(kdArr, testCase2SplitMethod);
	spKDArrayDestroy(kdArr);


	caseData->max_dim = testCase2MaxDim;

	caseData->split_method = testCase2SplitMethod;
	caseData->tree = testCase2Tree;

	return caseData;
}

bool runTestCase2(treeNodeTestCaseData caseData){
	//l stands for left branch of the tree and r for right branch
	SPKDTreeNode l,r,ll,lr,rl,rr,lll,llr;
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

	ASSERT_TRUE(testKDTree(caseData->tree,caseData->max_dim,
			caseData->split_method,caseData->points,caseData->size));

	//handle root
	ASSERT_TRUE(verifyNotLeafAndData(caseData->tree,0,3));

	//handle first level
	l = caseData->tree->kdtLeft;
	r = caseData->tree->kdtRight;

	ASSERT_TRUE(verifyNotLeafAndData(l,1,4));
	ASSERT_TRUE(verifyNotLeafAndData(r,1,11));

	//handle second level
	ll = l->kdtLeft;
	lr = l->kdtRight;
	rl = r->kdtLeft;
	rr = r->kdtRight;

	ASSERT_TRUE(verifyNotLeafAndData(ll,0,1));

	ASSERT_TRUE(verifyLeaf(lr));
	ASSERT_TRUE(verifyPointData(lr->data, 3, caseData->max_dim,2.0,7.0));

	ASSERT_TRUE(verifyLeaf(rl));
	ASSERT_TRUE(verifyPointData(rl->data, 4, caseData->max_dim,9.0,11.0));

	ASSERT_TRUE(verifyLeaf(rr));
	ASSERT_TRUE(verifyPointData(rr->data, 2, caseData->max_dim,123.0,70.0));


	//handle 3rd level
	lll = ll->kdtLeft;
	llr = ll->kdtRight;

	ASSERT_TRUE(verifyLeaf(lll));
	ASSERT_TRUE(verifyPointData(lll->data, 1, caseData->max_dim,1.0,2.0));

	ASSERT_TRUE(verifyLeaf(llr));
	ASSERT_TRUE(verifyPointData(llr->data, 5, caseData->max_dim,3.0,4.0));

	return true;
}


//edge test case 1
treeNodeTestCaseData initializeEdgeTestCase1(){
	//edge case 1 data
	SPKDTreeNode edgeTestCase1Tree = NULL;
	SPPoint* edgeTestCase1Points = NULL;
	int edgeTestCase1MaxDim = 1;
	int edgeTestCase1Size = 1;
	SP_KDTREE_SPLIT_METHOD edgeTestCase1SplitMethod = INCREMENTAL;
	treeNodeTestCaseData caseData = NULL;
	double data1[1] = {2};
	SPPoint p1;
	SPKDArray kdArr = NULL;
	spCalloc(caseData, tree_node_test_case_data, 1);

	edgeTestCase1Points = (SPPoint*)calloc(sizeof(SPPoint),edgeTestCase1Size);
	if (edgeTestCase1Points == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}
	caseData->points = edgeTestCase1Points;
	caseData->size = edgeTestCase1Size;

	p1 = spPointCreate(data1,edgeTestCase1MaxDim,1);
	if (p1 == NULL){
		spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY,__FILE__,__FUNCTION__,__LINE__);
		return NULL;
	}


	edgeTestCase1Points[0] = p1;

	kdArr = Init(edgeTestCase1Points,edgeTestCase1Size);
	edgeTestCase1Tree = InitKDTree(kdArr, edgeTestCase1SplitMethod);
	spKDArrayDestroy(kdArr);


	caseData->max_dim = edgeTestCase1MaxDim;
	caseData->split_method = edgeTestCase1SplitMethod;
	caseData->tree = edgeTestCase1Tree;

	return caseData;
}

bool runEdgeTestCase1(treeNodeTestCaseData caseData){
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

	ASSERT_TRUE(testKDTree(caseData->tree,caseData->max_dim,
			caseData->split_method,caseData->points,caseData->size));


	//root node - leaf with value {2}
	ASSERT_TRUE(verifyLeaf(caseData->tree));
	ASSERT_TRUE(verifyPointData(caseData->tree->data, 1, caseData->max_dim,2.0));

	return true;
}


void runKDTreeNodeTests(){
	int i;
	treeNodeTestCaseData case1Data = NULL, edgeCase1Data = NULL, case2Data = NULL;
	int seed = time(NULL);
	srand(seed);

	//null case
	RUN_TEST(verifyNullArgument);

	//case 1
	case1Data = initializeTestCase1();
	RUN_TEST_WITH_PARAM(runTestCase1, case1Data);
	destroyTestCase(case1Data);

	//case 2
	case2Data = initializeTestCase2();
	RUN_TEST_WITH_PARAM(runTestCase2, case2Data);
	destroyTestCase(case2Data);

	//edge case 1
	edgeCase1Data = initializeEdgeTestCase1();
	RUN_TEST_WITH_PARAM(runEdgeTestCase1, edgeCase1Data);
	destroyTestCase(edgeCase1Data);

	//random tests
	for (i = 0 ; i< RANDOM_TESTS_COUNT;i++)
		RUN_TEST(runRandomKDTreeTest);
}
