#include "SPKDTreeNodeKNN.h"
#include "../../general_utils/SPUtils.h"
#include "assert.h"

#define ERROR_PUSHING_LIST_ELEMENT 							    "Could not add list element due to memory issue, k-NN search failed"
#define ERROR_CREATING_LIST_ELEMENT 							"Could not create list element, k-NN search failed"

#define DEBUG_KNN_SEARCH_LOOKING_AT_THE_OTHER_PLANE 			"Knn search, looking at the other plane, dim index - "

double getSquaredDistance(double a, double b){
	return (a-b)*(a-b);
}

SPKDTreeNode getSecondChild(SPKDTreeNode tree, SPKDTreeNode firstChild){
	return firstChild == tree->kdtLeft ? tree->kdtRight : tree->kdtLeft;
}



bool pushLeafToQueue(SPPoint currPoint, SPBPQueue bpq, SPPoint queryPoint){
	SPListElement elem = NULL;
	SP_BPQUEUE_MSG queueMessage;
	double distance = spPointL2SquaredDistance(currPoint, queryPoint);
	//TODO - talk about this...
	if (distance <= epsilon) // this is the most precise we can get => any lesser number should be treated as same point
		distance = 0;
	spVal((elem = spListElementCreate(spPointGetIndex(currPoint), distance)),
			ERROR_CREATING_LIST_ELEMENT, false);

	queueMessage = spBPQueueEnqueue(bpq, elem);
	spListElementDestroy(elem);

	spValWc(queueMessage == SP_BPQUEUE_FULL  || queueMessage  == SP_BPQUEUE_SUCCESS,
			ERROR_PUSHING_LIST_ELEMENT,
			//since we pre assume the arguments are not null, this case is
			//only when queueMessage ==  SP_BPQUEUE_OUT_OF_MEMORY
			assert(queueMessage ==  SP_BPQUEUE_OUT_OF_MEMORY), false);

	return true;
}

bool isCrossingHypersphere(SPKDTreeNode node, double coorValue, double maxBPQValue){
	return getSquaredDistance(*(node->val),coorValue) <= maxBPQValue;
}

bool kNearestNeighbors(SPKDTreeNode curr, SPBPQueue bpq, SPPoint queryPoint){
	assert(queryPoint != NULL && bpq != NULL);
	double relevantAxisValue;
	SPKDTreeNode candidate;

	if (curr == NULL){
		return true;
	}
	//not null
	if (isLeaf(curr)){
		return pushLeafToQueue(curr->data,bpq,queryPoint);
	}

	relevantAxisValue = spPointGetAxisCoor(queryPoint, curr->dim);

	//not a leaf

	//pick the next axis
	candidate = (relevantAxisValue <= *(curr->val)) ? curr->kdtLeft : curr->kdtRight;

	//Recursively continue to the next axis process
	if (!kNearestNeighbors(candidate, bpq, queryPoint))
		return false;

	//check the other plane if needed
	if (!spBPQueueIsFull(bpq) || isCrossingHypersphere(curr, relevantAxisValue, spBPQueueMaxValue(bpq))){
		//spLoggerSafePrintDebugWithIndex(
		//		DEBUG_KNN_SEARCH_LOOKING_AT_THE_OTHER_PLANE, curr->dim,
		//			__FILE__, __FUNCTION__, __LINE__);
		return kNearestNeighbors(getSecondChild(curr,candidate) ,bpq,queryPoint);
	}

	return true;
}

