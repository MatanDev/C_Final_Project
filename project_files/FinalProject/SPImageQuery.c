#include <stdio.h>
#include <stdlib.h>

#include "SPConfig.h"
#include "SPImageQuery.h"
#include "SPKDTreeNodeKNN.h"

int* spIQ_getTopItems(int* counterArray, int numOfImages, int numOfSimilarImages)
{
	int i,j, tempMaxIndex;
	int *topItems = (int*)calloc(numOfSimilarImages, sizeof(int));

	if (topItems == NULL){
		//TODO - report memory problem
		return NULL;
	}

	// TODO - is best way?
	for (j = 0; j < numOfSimilarImages; j++) {
		tempMaxIndex = 0;
		for (i = 0; i < numOfImages; i++) {
			// the ">" is important (not >=) to keep the "internal sort" so that
			// if 2 indexes has the same value we will have the smaller one first
			if (counterArray[i] > counterArray[tempMaxIndex]) {
				tempMaxIndex = i;
			}
		}
		topItems[j] = tempMaxIndex;
		counterArray[tempMaxIndex] = -1;
	}
	return topItems;
}


int* createOutputArray(SPBPQueue bpq, int originalQueueSize)
{
	int imageIndex;
	SP_BPQUEUE_MSG queueMessage;
	SPListElement elem;
	int *outputArray = (int*)calloc(spBPQueueSize(bpq), sizeof(int));
	if (outputArray == NULL)
	{
		return NULL; //TODO - report relevant error
	}

	//what if queue size is less than top images???

	for (imageIndex = 0; imageIndex < originalQueueSize; imageIndex++){
		elem = spBPQueuePeek(bpq);
		outputArray[imageIndex] = spListElementGetIndex(elem);
		spListElementDestroy(elem);
		queueMessage = spBPQueueDequeue(bpq);
		if (queueMessage != SP_BPQUEUE_SUCCESS){
			//report error
			free(outputArray);
			return NULL;
		}
	}

	return outputArray;
}

int* spIQ_BestSIFTL2SquaredDistance(SPPoint relevantFeature, SPKDTreeNode kdTree,
		SPBPQueue bpq, int* finalQueueSize){
	int* outputArray = NULL;

	bool successFlag = true;

	if (bpq == NULL){
		//log error and return null
		return NULL;
	}

	if (kdTree == NULL){
		//handle error
		return NULL;
	}

	successFlag = kNearestNeighbors(kdTree, bpq, relevantFeature);

	if (successFlag == false){
		spBPQueueDestroy(bpq); // TODO - validate if needed or handle in main
		//free memory?
		return NULL;
	}

	*finalQueueSize = spBPQueueSize(bpq);

	// allocate output array and fill it
	outputArray = createOutputArray(bpq, *finalQueueSize);

	return outputArray;
}

int* initializeCounterArray(int numOfImages) {
	int i, *counterArray;
	counterArray = (int*)calloc(numOfImages, sizeof(int));

	if (counterArray == NULL){
		//TODO - report memory problem
		return NULL;
	}

	// set up the counter array
	for (i = 0; i < numOfImages; i++) {
		counterArray[i] = 0;
	}

	return counterArray;
}

int* spIQ_getSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq) {
	int i, j, finalQueueSize;
	int *topItems, *counterArray, *resultsArray;
	if (workingImage == NULL || kdTree == NULL){
		return NULL; //TODO - log relevant error
	}

	// create an index-counter array for the images
	counterArray = initializeCounterArray(numOfImages);

	if (counterArray == NULL){
		//TODO - report memory problem
		return NULL;
	}

	// for each feature in the working image send a request to compare the best NUM_OF_BEST_DIST_IMGS
	// for each NUM_OF_BEST_DIST_IMGS returned increase a counter at the relevant index
	for (i = 0; i < workingImage->numOfFeatures; i++) {
		resultsArray = spIQ_BestSIFTL2SquaredDistance((workingImage->featuresArray)[i],
				kdTree, bpq, &finalQueueSize);

		if (resultsArray == NULL) {
			//TODO - report relevant problem
			free(counterArray);
			return NULL;
		}

		for (j = 0; j < finalQueueSize; j++) {
			counterArray[resultsArray[j]]++;
		}

		if (resultsArray != NULL)
			free(resultsArray);
	}

	// sort the index counter array and get the best NUM_OF_BEST_DIST_IMGS
	topItems = spIQ_getTopItems(counterArray, numOfImages, numOfSimilarImages);
	return topItems;
}
