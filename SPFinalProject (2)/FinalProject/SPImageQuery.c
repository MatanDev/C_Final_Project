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

#define NUM_OF_BEST_DIST_IMGS 5

static int numOfImages = 0;
static int topImages = 0;
static SPImageData* workingImagesDatabase = NULL;

typedef struct distanceWithPoint {
	double distance;
	SPPoint point;
} distanceWithPoint;

int* spIQ_getTopItems(int* counterArray)
{
	int i,j, tempMaxIndex;
	int *topItems = (int*)calloc(NUM_OF_BEST_DIST_IMGS, sizeof(int));

	if (topItems == NULL){
		//TODO - report memory problem
		return NULL;
	}

	for (j = 0; j < NUM_OF_BEST_DIST_IMGS; j++) {
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

int distanceWithPointComparator(const void * firstItem, const void * secondItem)
{
	distanceWithPoint* item1;
	distanceWithPoint* item2;
	double dist;
	item1 = (distanceWithPoint*)firstItem;
	item2 = (distanceWithPoint*)secondItem;

	dist = (item1)->distance - (item2)->distance;

	if (dist < 0.0)
		return -1;

	if (dist > 0.0)
		return 1;

	return spPointGetIndex((item1)->point) - spPointGetIndex((item2)->point);
}

distanceWithPoint* createAndSortDistancesArray(int totalNumberOfFeatures, SPPoint relevantFeature){
	distanceWithPoint* distancesArray = NULL;
	int i,j,k=0;
	distancesArray = (distanceWithPoint*)calloc(sizeof(distanceWithPoint),totalNumberOfFeatures);
	if (distancesArray  == NULL){
		return NULL; //TODO -report relevant error
	}

	for (i=0;i<numOfImages;i++){
		for (j=0;j<workingImagesDatabase[i]->numOfFeatures;j++){
			distancesArray[k].point =(workingImagesDatabase[i]->featuresArray)[j];
			distancesArray[k].distance = spPointL2SquaredDistance(relevantFeature, distancesArray[k].point);
			k++;
		}
	}
	qsort(distancesArray, totalNumberOfFeatures, sizeof(distanceWithPoint), distanceWithPointComparator);
	return distancesArray;
}

int* createOutputArray(distanceWithPoint* distancesArray)
{
	int feature;
	int *outputArray = (int*)calloc(NUM_OF_BEST_DIST_IMGS, sizeof(int));
	if (outputArray == NULL)
	{
		return NULL; //TODO - report relevant error
	}

	for (feature = 0; feature < NUM_OF_BEST_DIST_IMGS; feature++)
		outputArray[feature] = spPointGetIndex((distancesArray[feature]).point);

	return outputArray;
}

int* spIQ_BestSIFTL2SquaredDistance(SPPoint relevantFeature, int totalNumberOfFeatures){
	int* outputArray = NULL;
	distanceWithPoint* distancesArray = NULL;

	// create distances array, fill it and sort it
	distancesArray = createAndSortDistancesArray(totalNumberOfFeatures, relevantFeature);

	if (distancesArray == NULL){
		return NULL;
	}

	// allocate output array and fill it
	outputArray = createOutputArray(distancesArray);

	// free the memory which is no longer needed
	free(distancesArray);

	return outputArray;
}

int calcTotalNumberOfFeatures(){
	int i,k=0;
	for (i=0;i<numOfImages;i++){
		k+= workingImagesDatabase[i]->numOfFeatures;
	}
	return k;
}

int* spIQ_searchForSimmilarImages(SPImageData queryImage)
{
	int i,j;
	int *topItems, *counterArray, *resultsArray;
	int totalNumberOfFeatures;
	// create an index-counter array for the images
	counterArray = (int*)calloc(numOfImages, sizeof(int));

	if (counterArray == NULL){
		//TODO - report memory problem
		return NULL;
	}

	// set up the counter array
	for (i = 0; i < numOfImages; i++)
	{
		counterArray[i] = 0;
	}

	totalNumberOfFeatures = calcTotalNumberOfFeatures();

	// for each feature in the working image send a request to compare the best NUM_OF_BEST_DIST_IMGS
	// for each NUM_OF_BEST_DIST_IMGS returned increase a counter at the relevant index
	for (i = 0; i < queryImage->numOfFeatures; i++)
	{
		resultsArray = spIQ_BestSIFTL2SquaredDistance((queryImage->featuresArray)[i], totalNumberOfFeatures);

		if (resultsArray == NULL)
		{
			//TODO - report relevant problem
			free(counterArray);
			return NULL;
		}

		for (j = 0; j < NUM_OF_BEST_DIST_IMGS; j++)
		{
			counterArray[resultsArray[j]]++;
		}

		if (resultsArray != NULL)
			free(resultsArray);
	}

	// sort the index counter array and get the best NUM_OF_BEST_DIST_IMGS
	topItems = spIQ_getTopItems(counterArray);
	return topItems;
}

int* spIQ_getSimilarImages(SPConfig config,SPImageData* imagesDatabase,SPImageData workingImage, int countOfSimilar){
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	int* rsltArray = NULL;
	if (config == NULL || imagesDatabase == NULL || workingImage == NULL){
		return NULL; //TODO - log relevant error
	}

	numOfImages = spConfigGetNumOfImages(config, &configMessage);
	if (configMessage != SP_CONFIG_SUCCESS){
		return NULL; //TODO - log relevant error
	}

	topImages = countOfSimilar;
	workingImagesDatabase = imagesDatabase;
	rsltArray = spIQ_searchForSimmilarImages(workingImage);

	return rsltArray;
}

