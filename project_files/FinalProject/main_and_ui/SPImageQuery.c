#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "SPImageQuery.h"
#include "../data_structures/kd_ds/SPKDTreeNodeKNN.h"
#include "../SPLogger.h"
#include "../general_utils/SPUtils.h"

#define ERROR_EMPTY_QUEUE 							"Queue is empty"
#define ERROR_K_NEAREST_NEIGHBORS 					"Error in kNearestNeighbors func"
#define ERROR_GET_SIMILAR_IMAGES_INDICES_TO_FEAURE 	"Error in getSimilarImagesIndicesToFeature func"
#define ERROR_UPDATE_COUNTER_ARRAY_PER_FEATURE 		"Error in updateCounterArrayPerFeature func"
#define ERROR_GENERATING_SIMILAR_IMAGES				"Error generating similar images"

#define DEBUG_SIMILAR_IMAGES_ENDED 					"Similar images search process ended, selecting best images"
#define DEBUG_SIMILAR_IMAGES_SEARCH_STARTED 		"Similar images search process started"


int* initializeCounterArray(int size) {
	int i, *counterArray;

	spCalloc(counterArray, int, size);

	// set up the counter array
	for (i = 0; i < size; i++)
		counterArray[i] = 0;

	return counterArray;
}

SP_BPQUEUE_MSG popFromQueueToIndicesArray(SPBPQueue bpq, int* indicesArray,
		int arrayIndex) {
	SPListElement elem = spBPQueuePeek(bpq);
	indicesArray[arrayIndex] = spListElementGetIndex(elem);
	spListElementDestroy(elem);
	return spBPQueueDequeue(bpq);
}

int* createSimImagesToFeatureIndicesArray(SPBPQueue bpq, int originalQueueSize) {
	int arrayIndex, *indicesArray;

	spCalloc(indicesArray, int, originalQueueSize);

	for (arrayIndex = 0; arrayIndex < originalQueueSize; arrayIndex++) {
		spValWcRn((popFromQueueToIndicesArray(bpq, indicesArray, arrayIndex)
				== SP_BPQUEUE_SUCCESS), ERROR_EMPTY_QUEUE, free(indicesArray));
	}

	assert(spBPQueueIsEmpty(bpq));

	return indicesArray;
}

int* getSimilarImagesIndicesToFeature(SPPoint relevantFeature, SPKDTreeNode kdTree,
		SPBPQueue bpq, int* finalQueueSize) {

	spValRn(kNearestNeighbors(kdTree, bpq, relevantFeature), ERROR_K_NEAREST_NEIGHBORS);
	*finalQueueSize = spBPQueueSize(bpq);

	return createSimImagesToFeatureIndicesArray(bpq, *finalQueueSize);
}



bool updateCounterArrayPerFeature(int* counterArray, SPPoint relevantFeature,
		SPKDTreeNode kdTree, SPBPQueue bpq) {
	int j, finalQueueSize, *similarImagesIndices;

	spVal((similarImagesIndices = getSimilarImagesIndicesToFeature(relevantFeature, kdTree,
			bpq, &finalQueueSize)), ERROR_GET_SIMILAR_IMAGES_INDICES_TO_FEAURE, false);

	for (j = 0; j < finalQueueSize; j++)
		counterArray[similarImagesIndices[j]]++;

	// if we get here closestImagesIndices is not null
	free(similarImagesIndices);

	return true;
}

int* getTopItems(int* counterArray, int counterArraySize, int retArraySize) {
	int i, j, tempMaxIndex, *topItems;

	spCalloc(topItems, int, retArraySize);

	for (j = 0; j < retArraySize; j++) {
		tempMaxIndex = 0;

		for (i = 1; i < counterArraySize; i++) {
			// the ">" is important (not >=) to keep the "internal sort" so that
			// if 2 indexes has the same value we will have the smaller one first
			if (counterArray[i] > counterArray[tempMaxIndex])
				tempMaxIndex = i;
		}

		topItems[j] = tempMaxIndex;
		counterArray[tempMaxIndex] = -1;
	}

	return topItems;
}

int* getSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq) {
	int i, *topItems, *counterArray;
	spVerifyArguments(workingImage != NULL && kdTree != NULL && bpq != NULL,
			ERROR_GENERATING_SIMILAR_IMAGES, NULL);


	// create an index-counter array for the images
	spValRn((counterArray = initializeCounterArray(numOfImages)),
			ERROR_GENERATING_SIMILAR_IMAGES);

	spLoggerSafePrintDebug(DEBUG_SIMILAR_IMAGES_SEARCH_STARTED,
				__FILE__, __FUNCTION__, __LINE__);

	for (i = 0; i < workingImage->numOfFeatures; i++) {
		spValWcRn((updateCounterArrayPerFeature(counterArray, (workingImage->featuresArray)[i],
				kdTree, bpq)),
				ERROR_UPDATE_COUNTER_ARRAY_PER_FEATURE,
				free(counterArray));
	}

	spLoggerSafePrintDebug(DEBUG_SIMILAR_IMAGES_ENDED,
				__FILE__, __FUNCTION__, __LINE__);

	topItems = getTopItems(counterArray, numOfImages, numOfSimilarImages);

	// if we get here counterArray is not null
	free(counterArray);

	return topItems;
}
