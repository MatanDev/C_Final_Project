#include <stdlib.h>
#include <time.h>
#include "SPKDTreeNode.h"
#include "../../general_utils/SPUtils.h"

#define INVALID_DIM 					-1
#define ERROR_POINT_COPY				"Error in copying point"
#define ERROR_CREATING_KD_INNER_NODE 	"Could not create inner node for KD tree"
#define ERROR_INITIALIZING_KD_TREE	 	"Could not create KD tree"



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
	spValRCb((node->data = spPointCopy(array->pointsArray[0])), ERROR_POINT_COPY,
			onErrorInInitKDTree(node));

	return node;
}

int getSplitDimInMaxSpreadMethod(SPKDArray array) {
	int splitDim = 0, j, maxPtrIndex, minPtrIndex;
	double maxPtrCoor, minPtrCoor, maxSpread = 0.0;
	for (j = 0; j < array->dim; j++) {
		maxPtrIndex = array->indicesMatrix[j][array->size - 1];
		minPtrIndex = array->indicesMatrix[j][0];
		maxPtrCoor = spPointGetAxisCoor(array->pointsArray[maxPtrIndex], j);
		minPtrCoor = spPointGetAxisCoor(array->pointsArray[minPtrIndex], j);
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
	//allocate node's value
	spCallocErWcRCb((node->val), double, 1, ERROR_CREATING_KD_INNER_NODE,
			spKDArrayPairDestroy(splitResPair), onErrorInInitKDTree(node));

	*(node->val) = spPointGetAxisCoor(
			array->pointsArray[array->indicesMatrix[splitDim][(array->size - 1) / 2]],
			splitDim);

	if (!(node->kdtLeft = internalInitKDTree(splitResPair->kdLeft, splitMethod,
					(recDepth + 1) % array->dim)) ||

	// valid cause we get here only if array->size > 1
			!(node->kdtRight = internalInitKDTree(splitResPair->kdRight, splitMethod,
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

	spVerifyArgumentsRn(array, ERROR_INITIALIZING_KD_TREE);

	spCalloc(ret, sp_kd_tree_node, 1);

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
		spFree(kdTreeNode->val);

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

bool isLeaf(SPKDTreeNode treeNode) {
	return (treeNode->data != NULL);
}

