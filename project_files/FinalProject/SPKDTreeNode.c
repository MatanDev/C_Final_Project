#include "SPKDTreeNode.h"
#include <stdlib.h>
#include <time.h>

#define INVALID_DIM 			-1
#define ERROR_INVALID_ARGUMENT	"Error Invalid argument"
#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"
#define ERROR_POINT_COPY		"Error in copying point"

SPKDTreeNode InitKDTreeFromPoints(SPPoint* pointsArray, int size,
		SP_KDTREE_SPLIT_METHOD splitMethod) {
	SPKDTreeNode ret = NULL;
	SPKDArray kdArray = NULL;
	kdArray = Init(pointsArray, size);
	ret = InitKDTree(kdArray, splitMethod);
	spKDArrayDestroy(kdArray);
	return ret;
}

SPKDTreeNode InitKDTree(SPKDArray array,
		SP_KDTREE_SPLIT_METHOD splitMethod) {
	return internalInitKDTree(array, splitMethod, 0);
}

SPKDTreeNode onErrorInInitKDTree(SPKDTreeNode node) {
	spKDTreeDestroy(node);
	return NULL;
}

SPKDTreeNode createLeaf(SPKDTreeNode node, SPKDArray array) {
	node->dim = INVALID_DIM;
	node->val = NULL;
	node->kdtLeft = NULL;
	node->kdtRight = NULL;
	if (!(node->data = spPointCopy(array->pointsArray[0]))) {
		spLoggerPrintError(ERROR_POINT_COPY, __FILE__,
				__FUNCTION__, __LINE__);
		return onErrorInInitKDTree(node);
	}

	return node;
}

//TODO - validate that its okay that default is 0
int getSplitDimInMaxSpreadMethod(SPKDArray array) {
	int splitDim = 0, j, maxPtrIndex, minPtrIndex;
	double maxPtrCoor, minPtrCoor, maxSpread = 0.0;
	for (j = 0; j < array->dim; j++) {
		maxPtrIndex = array->indicesMatrix[j][array->size - 1];
		minPtrIndex = array->indicesMatrix[j][0];
		maxPtrCoor =
			spPointGetAxisCoor(array->pointsArray[maxPtrIndex], j);
		minPtrCoor =
			spPointGetAxisCoor(array->pointsArray[minPtrIndex], j);
		if (maxSpread < maxPtrCoor - minPtrCoor) {
			maxSpread = maxPtrCoor - minPtrCoor;
			splitDim = j;
		}
	}
	return splitDim;
}

SPKDTreeNode createInnerNode(SPKDTreeNode node, SPKDArray array,
		SP_KDTREE_SPLIT_METHOD splitMethod, int recDepth, int splitDim) {
	SPKDArrayPair splitResPair = Split(array, splitDim);

	if (!splitResPair)
		return onErrorInInitKDTree(node);

	node->dim = splitDim;

	if (!(node->val = (double*)calloc(1, sizeof(double)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		spKDArrayPairDestroy(splitResPair);
		return onErrorInInitKDTree(node);
	}

	//TODO 1 - validate that val should be kind of median value
	//TODO 2 - maybe change getMedianIndex function
	*(node->val) = spPointGetAxisCoor(array->pointsArray[
		array->indicesMatrix[splitDim][getMedianIndex(array->size) - 1]],
			splitDim);

	if (	!(node->kdtLeft =
			internalInitKDTree(splitResPair->kdLeft, splitMethod,
					(recDepth + 1) % array->dim)) ||

	// valid cause we get here only if array->size > 1
			!(node->kdtRight =
			internalInitKDTree(splitResPair->kdRight, splitMethod,
					(recDepth + 1) % array->dim))	) {
		spKDArrayPairDestroy(splitResPair);
		return onErrorInInitKDTree(node);
	}

	node->data = NULL;

	spKDArrayPairDestroy(splitResPair);

	return node;
}

SPKDTreeNode internalInitKDTree(SPKDArray array,
		SP_KDTREE_SPLIT_METHOD splitMethod, int recDepth) {
	SPKDTreeNode ret;
	int splitDim;

	if (!array) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if (!(ret = (SPKDTreeNode)calloc(1, sizeof(struct sp_kd_tree_node))))
	{
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if (array->size == 1)
		return createLeaf(ret, array);

	switch (splitMethod) {
	case MAX_SPREAD:
		splitDim = getSplitDimInMaxSpreadMethod(array);
		break;
	case RANDOM:
		srand(time(NULL));
		splitDim = rand() % array->dim;
		break;
	case INCREMENTAL:
		splitDim = recDepth;
		break;
	}

	return createInnerNode(ret, array, splitMethod, recDepth, splitDim);
}

void spKDTreeDestroy(SPKDTreeNode kdTreeNode) {
	if (kdTreeNode) {
		if (kdTreeNode->val) {
			free(kdTreeNode->val);
			kdTreeNode->val = NULL;
		}

		if (kdTreeNode->kdtLeft) {
			spKDTreeDestroy(kdTreeNode->kdtLeft);
			kdTreeNode->kdtLeft = NULL;
		}

		if (kdTreeNode->kdtRight) {
			spKDTreeDestroy(kdTreeNode->kdtRight);
			kdTreeNode->kdtRight = NULL;
		}

		if (kdTreeNode->data) {
			spPointDestroy(kdTreeNode->data);
			kdTreeNode->data = NULL;
		}
		free(kdTreeNode);
	}
}

bool isLeaf(SPKDTreeNode treeNode){
	return (treeNode->data != NULL);
}
