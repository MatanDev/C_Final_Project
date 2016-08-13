#include "SPKDTreeNode.h"
#include <stdlib.h>
#include <time.h>

SPKDTreeNode InitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod) {
	return internalInitKDTree(array, splitMethod, 0);
}

SPKDTreeNode internalInitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod, int recDepth) {
	SPKDTreeNode ret;
	int splitDim = 0, j, maxPtrIndex, minPtrIndex;
	double maxPtrCoor, minPtrCoor, maxSpread = 0.0;
	SPKDArrayPair splitResPair;

	if (!array)
		return NULL;

	if (!(ret = (SPKDTreeNode)calloc(1, sizeof(struct sp_kd_tree_node))))
		return NULL;

	if (array->size == 1) {
		ret->dim = -1; //invalid
		ret->val = -1; //invalid
		ret->kdtLeft = NULL;
		ret->kdtRight = NULL;
		if (!(ret->data = spPointCopy(array->pointsArray[0]))) {
			free(ret);
			return NULL;
		}
		return ret;
	}

	switch (splitMethod) {
	case MAX_SPREAD:
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
		break;
	case RANDOM:
		srand(time(NULL));
		splitDim = rand() % array->dim;
		break;
	case INCREMENTAL:
		splitDim = recDepth;
		break;
	}

	splitResPair = Split(array, splitDim);
	if (!splitResPair) {
		free(ret);
		return NULL;
	}

	ret->dim = splitDim;
	// TODO 1 - understand what does val mean
	// TODO 2 - export to function find median in KDArray and use here
	// (or should it be median value and not median index??)
	ret->val = (array->size % 2 == 0) ?
			array->indicesMatrix[splitDim][(array->size / 2)] :
			array->indicesMatrix[splitDim][(array->size / 2) + 1];

	ret->kdtLeft =
			internalInitKDTree(splitResPair->kdLeft, splitMethod,
					(recDepth + 1) % array->dim);

	// valid cause we get here only if array->size > 1
	ret->kdtRight =
				internalInitKDTree(splitResPair->kdRight, splitMethod,
						(recDepth + 1) % array->dim);

	ret->data = NULL;

	return ret;
}

void spKDTreeDestroy(SPKDTreeNode kdTreeNode) {
	if (!kdTreeNode) {
		if (!(kdTreeNode->kdtLeft)) {
			spKDTreeDestroy(kdTreeNode->kdtLeft);
			kdTreeNode->kdtLeft = NULL;
		}

		if (!(kdTreeNode->kdtRight)) {
			spKDTreeDestroy(kdTreeNode->kdtRight);
			kdTreeNode->kdtRight = NULL;
		}

		if (!(kdTreeNode->data)) {
			spPointDestroy(kdTreeNode->data);
			kdTreeNode->data = NULL;
		}
		free(kdTreeNode);
	}
}
