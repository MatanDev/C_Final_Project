#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "SPImageQuery.h"
#include "SPKDTreeNodeKNN.h"
#include "SPLogger.h"

#define ERROR_ALLOCATING_MEMORY 					"Could not allocate memory"
#define ERROR_EMPTY_QUEUE 							"Queue is empty"
#define ERROR_K_NEAREST_NEIGHBORS 					"Error in kNearestNeighbors func"
#define ERROR_GET_SIMILAR_IMAGES_INDICES_TO_FEAURE 	\
	"Error in getSimilarImagesIndicesToFeature func"
#define ERROR_UPDATE_COUNTER_ARRAY_PER_FEATURE 		\
	"Error in updateCounterArrayPerFeature func"

int* initializeCounterArray(int size) {
	int i, *counterArray;

	if (!(counterArray = (int*)calloc(size, sizeof(int)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

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

	if (!(indicesArray = (int*)calloc(originalQueueSize, sizeof(int)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	for (arrayIndex = 0; arrayIndex < originalQueueSize; arrayIndex++) {
		if (popFromQueueToIndicesArray(bpq, indicesArray, arrayIndex)
				!= SP_BPQUEUE_SUCCESS) {
			// we assume bpq is not NULL
			spLoggerPrintError(ERROR_EMPTY_QUEUE, __FILE__, __FUNCTION__, __LINE__);
			free(indicesArray);
			return NULL;
		}
	}

	assert(spBPQueueIsEmpty(bpq));

	return indicesArray;
}

int* getSimilarImagesIndicesToFeature(SPPoint relevantFeature, SPKDTreeNode kdTree,
		SPBPQueue bpq, int* finalQueueSize) {
	if (!kNearestNeighbors(kdTree, bpq, relevantFeature)){
		spLoggerPrintError(ERROR_K_NEAREST_NEIGHBORS, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	*finalQueueSize = spBPQueueSize(bpq);

	return createSimImagesToFeatureIndicesArray(bpq, *finalQueueSize);
}

bool updateCounterArrayPerFeature(int* counterArray, SPPoint relevantFeature,
		SPKDTreeNode kdTree, SPBPQueue bpq) {
	int j, finalQueueSize, *similarImagesIndices;

	if (!(similarImagesIndices = getSimilarImagesIndicesToFeature(relevantFeature, kdTree,
			bpq, &finalQueueSize))) {
		spLoggerPrintError(ERROR_GET_SIMILAR_IMAGES_INDICES_TO_FEAURE, __FILE__,
				__FUNCTION__, __LINE__);
		return false;
	}

	for (j = 0; j < finalQueueSize; j++)
		counterArray[similarImagesIndices[j]]++;

	// if we get here closestImagesIndices is not null
	free(similarImagesIndices);

	return true;
}

int* getTopItems(int* counterArray, int counterArraySize, int retArraySize) {
	int i, j, tempMaxIndex, *topItems;

	if (!(topItems = (int*)calloc(retArraySize, sizeof(int)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	// TODO - is best way?
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
	assert(workingImage != NULL && kdTree != NULL && bpq != NULL);
	int i, *topItems, *counterArray;

	// create an index-counter array for the images
	if (!(counterArray = initializeCounterArray(numOfImages))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	for (i = 0; i < workingImage->numOfFeatures; i++) {
		if (!updateCounterArrayPerFeature(counterArray, (workingImage->featuresArray)[i],
				kdTree, bpq)) {
			spLoggerPrintError(ERROR_UPDATE_COUNTER_ARRAY_PER_FEATURE, __FILE__,
					__FUNCTION__, __LINE__);
			// if we get here counterArray is not null
			free(counterArray);
			return NULL;
		}
	}

	topItems = getTopItems(counterArray, numOfImages, numOfSimilarImages);

	// TODO - maybe free inside previous line function
	// if we get here counterArray is not null
	free(counterArray);

	return topItems;
}
