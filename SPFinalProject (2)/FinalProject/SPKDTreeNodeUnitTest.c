#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "unit_test_util.h"
#include "SPPoint.h"
#include "SPConfig.h"
#include "SPKDArray.h"
#include "SPLogger.h"
#include "SPKDTreeNodeUnitTest.h"
#include "SPKDTreeNode.h"
#include "SPKDArrayUnitTest.h"

#define ERROR_AT_TREE_SIZE 									"Error at tree size"
#define ERROR_AT_NODES_STRUCTURE 							"Error at nodes structure"
#define ERROR_AT_TREE_MEDIAN_INVARIANT 						"Error at tree median invariant"
#define ERROR_AT_TREE_ORDER_INVARIANT 						"Error at tree order invariant"
#define ERROR_AT_TREE_DIMENSION_SELECTION_METHOD 			"Error at tree dimension selection method"


#define COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST 		"Could not create points array for random test"
#define COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST 		"Could not initialize kd-array for random test"
#define COULD_NOT_INITIALIZE_TREE 							"Could not initialize tree for random test - InitKDTree returned null"

//random test case macros
#define RANDOM_TESTS_SIZE_RANGE  							200
#define RANDOM_TESTS_DIM_RANGE 								50
#define RANDOM_TESTS_COUNT 									20

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

bool isLeaf(SPKDTreeNode treeNode){
	return (treeNode->data != NULL);
}

int getTreeSize(SPKDTreeNode treeNode){
	if (treeNode == NULL)
		return 0;
	if (isLeaf(treeNode))
		return 1;
	return 1 + getTreeSize(treeNode->kdtLeft) + getTreeSize(treeNode->kdtRight);
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
	return safeSearchValueInTree(treeNode->kdtLeft,dim,value) &&
			safeSearchValueInTree(treeNode->kdtRight,dim,value);
}

bool safeIsTreeLessEqualThan(SPKDTreeNode treeNode, int dim, double value){
	if (treeNode == NULL)
		return true;
	if (isLeaf(treeNode))
		return spPointGetAxisCoor(treeNode->data,dim) <= value;
	return safeSearchValueInTree(treeNode->kdtLeft,dim,value) &&
			safeSearchValueInTree(treeNode->kdtRight,dim,value);
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
			getTreeSize(treeNode->kdtLeft) >= getTreeSize(treeNode->kdtRight) &&
			getTreeSize(treeNode->kdtLeft) - getTreeSize(treeNode->kdtRight) <= 1;
}

bool verifyMedianTreeNode(SPKDTreeNode treeNode){
	if (treeNode == NULL)
			return true;
	return verifyMedianNode(treeNode) &&
			verifyMedianTreeNode(treeNode->kdtLeft)&&
			verifyMedianTreeNode(treeNode->kdtLeft);
}

bool internalVerifyDimIncremental(SPKDTreeNode treeNode, int depth, int maxDim){
	if (treeNode == NULL || isLeaf(treeNode))
			return true;
	if (treeNode->kdtLeft != NULL && !isLeaf(treeNode->kdtLeft)){
		if (treeNode->dim % maxDim - treeNode->kdtLeft->dim % maxDim != 1)
			return false;
	}
	if (treeNode->kdtRight != NULL && !isLeaf(treeNode->kdtRight)){
			if (treeNode->dim % maxDim - treeNode->kdtRight->dim % maxDim != 1)
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

bool verifyDimMaxSpread(SPKDTreeNode treeNode,SPPoint* pointsArray ){
	if (treeNode == NULL || isLeaf(treeNode))
			return true;
	//TODO -write this shit
	return true;
}

bool verifyDimSelection(SPKDTreeNode treeNode, int maxDim,SP_KDTREE_SPLIT_METHOD splitMethod, SPPoint* pointsArray){
	switch (splitMethod)
	{
		case RANDOM:
			return true;
			break;
		case MAX_SPREAD:
			return verifyDimMaxSpread(treeNode,pointsArray);
			break;
		case INCREMENTAL:
			return verifyDimIncremental(treeNode, maxDim);
			break;
		default:
			return false;
	}
	return false;
}

bool testKDTree(SPKDTreeNode treeNode, int maxDim,SP_KDTREE_SPLIT_METHOD splitMethod, SPPoint* pointsArray, int size){
	if (treeNode == NULL && size == 0)
		return true;
	if (treeNode == NULL)
		return false;

	//test tree size
	if (getTreeSize(treeNode) != size){
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
	if (!verifyDimSelection(treeNode,maxDim,splitMethod,pointsArray)){
		spLoggerPrintError(ERROR_AT_TREE_DIMENSION_SELECTION_METHOD, __FILE__,
				__FUNCTION__, __LINE__);
		FAIL(ERROR_AT_TREE_DIMENSION_SELECTION_METHOD);
		return false;
	}

	return true;
}

bool runRandomKDTreeTest(){
	int maxDim, size;
	SPPoint* pointsArray = NULL;
	SPKDArray kdArr = NULL;
	SPKDTreeNode tree = NULL;
	SP_KDTREE_SPLIT_METHOD splitMethod;

	maxDim = 1 + (int)(rand() % RANDOM_TESTS_DIM_RANGE);
	size = 2 + (int)(rand() % RANDOM_TESTS_SIZE_RANGE); //size = 1 is tested at a the edge cases
	splitMethod = 1+(int)(rand()%3);

	pointsArray = generateRandomPointsArray(maxDim,size);

	ASSERT_TRUE(pointsArray != NULL);
	if (pointsArray == NULL){
		spLoggerPrintError(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_CREATE_POINTS_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	kdArr = Init(pointsArray,size);
	if (pointsArray == NULL){
		spLoggerPrintError(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_KD_ARRAY_FOR_RANDOM_TEST);
		return false;
	}

	tree = InitKDTree(kdArr, splitMethod);

	if (pointsArray == NULL){
		spLoggerPrintError(
				COULD_NOT_INITIALIZE_TREE,
				__FILE__, __FUNCTION__,
						__LINE__);
		FAIL(COULD_NOT_INITIALIZE_TREE);
		return false;
	}

	return testKDTree( tree,  maxDim, splitMethod,  pointsArray,  size);
}

void runKDTreeNodeTests(){
	RUN_TEST(runRandomKDTreeTest);
}
