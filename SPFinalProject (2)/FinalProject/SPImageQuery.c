/*
 * SPImageQuery.c
 *
 *  Created on: 2 баев 2016
 *      Author: Matan
 */
#include <stdio.h>
#include <stdlib.h>

#include "SPConfig.h"
#include "SPImageQuery.h"
#include "SPBPriorityQueue.h"
#include "SPKDTreeNodeKNN.h"


static int numOfImages = 0;
static int numOfFeatures = 0;
static int topImages = 0;
static SP_KDTREE_SPLIT_METHOD splitMethod = 0;
static int knn = 0;
static SPImageData* workingImagesDatabase = NULL;
static SPPoint* featuresArray = NULL;

int* spIQ_getTopItems(int* counterArray)
{
	int i,j, tempMaxIndex;
	int *topItems = (int*)calloc(topImages, sizeof(int));

	if (topItems == NULL){
		//TODO - report memory problem
		return NULL;
	}

	for (j = 0; j < topImages; j++) {
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

void initializeFeaturesArray(){
	int i,j,k = 0;
	featuresArray = (SPPoint*)calloc(sizeof(SPPoint),numOfFeatures);
	if (featuresArray == NULL){
		//handle error (memory)
		return;
	}
	for (i=0;i<numOfImages;i++){
		for (j = 0 ;j< workingImagesDatabase[i]->numOfFeatures ; j++){
			featuresArray[k] = (workingImagesDatabase[i]->featuresArray)[j];
			k++;
		}
	}
}


int* createOutputArray(SPBPQueue bpq)
{
	int imageIndex;
	SP_BPQUEUE_MSG queueMessage;
	SPListElement elem;
	int *outputArray = (int*)calloc(knn, sizeof(int));
	if (outputArray == NULL)
	{
		return NULL; //TODO - report relevant error
	}

	//what if queue size is less than top images???

	for (imageIndex = 0; imageIndex < knn; imageIndex++){
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

int* spIQ_BestSIFTL2SquaredDistance(SPPoint relevantFeature){
	int* outputArray = NULL;
	SPBPQueue bpq = NULL;
	SPKDTreeNode kdTree = NULL;

	bool successFlag = true;


	bpq = spBPQueueCreate(knn);

	if (bpq == NULL){
		//log error and return null
		return NULL;
	}

	kdTree = InitKDTreeFromPoints(featuresArray, numOfFeatures, splitMethod);

	if (kdTree == NULL){
		//handle error
		return NULL;
	}

	successFlag = kNearestNeighbors(kdTree, bpq, relevantFeature);

	if (successFlag == false){
			spBPQueueDestroy(bpq);
			//free memory?
			return NULL;
	}

	// allocate output array and fill it
	outputArray = createOutputArray(bpq);

	// free the memory which is no longer needed
	spBPQueueDestroy(bpq);

	return outputArray;
}

int* spIQ_searchForSimmilarImages(SPImageData queryImage)
{
	int i,j;
	int *topItems, *counterArray, *resultsArray;

	// create an index-counter array for the images
	counterArray = (int*)calloc(numOfImages, sizeof(int));

	if (counterArray == NULL){
		//TODO - report memory problem
		return NULL;
	}

	// set up the counter array
	for (i = 0; i < numOfImages; i++) {
		counterArray[i] = 0;
	}


	// for each feature in the working image send a request to compare the best NUM_OF_BEST_DIST_IMGS
	// for each NUM_OF_BEST_DIST_IMGS returned increase a counter at the relevant index
	for (i = 0; i < queryImage->numOfFeatures; i++) {
		resultsArray = spIQ_BestSIFTL2SquaredDistance((queryImage->featuresArray)[i]);

		if (resultsArray == NULL) {
			//TODO - report relevant problem
			free(counterArray);
			return NULL;
		}
		for (j = 0; j < topImages; j++) {
			counterArray[resultsArray[j]]++;
		}
		if (resultsArray != NULL)
			free(resultsArray);
	}

	// sort the index counter array and get the best NUM_OF_BEST_DIST_IMGS
	topItems = spIQ_getTopItems(counterArray);
	return topItems;
}

int calculateTotalNumOfFeatures(){
	int i, sum = 0;
	for (i = 0; i < numOfImages ; i++){
		sum += workingImagesDatabase[i]->numOfFeatures;
	}
	return sum;
}

int* spIQ_getSimilarImages(SPImageData* imagesDatabase,SPImageData workingImage, int countOfSimilar, int argNumOfImages, int argKnn){
	int* rsltArray = NULL;
	if (imagesDatabase == NULL || workingImage == NULL){
		return NULL; //TODO - log relevant error
	}

	workingImagesDatabase = imagesDatabase;
	numOfImages = argNumOfImages;
	numOfFeatures = calculateTotalNumOfFeatures();
	topImages = countOfSimilar;
	knn = argKnn;
	initializeFeaturesArray();

	rsltArray = spIQ_searchForSimmilarImages(workingImage);

	return rsltArray;
}

