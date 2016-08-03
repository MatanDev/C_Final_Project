#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SPMainAux.h"

#include "SPImageQuery.h"


#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Enter a query image or # to terminate:\n"
#define CLOSEST_IMAGES "The closest images are: "
#define EXITING "Exiting...\n"

#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"

void getQuery(char* destination){
	getAsString(ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE, destination);
}

void getAsString(const char* message, char* destination)
{
	printf("%s", message);
	fflush(NULL);
	scanf("%s", destination);
	fflush(NULL);
}

void endControlFlow(SPConfig config, SPImageData image, SPImageData* imagesList, int numOfImages){
	printf("%s", EXITING);
	spConfigDestroy(config);
	freeImageData(image);
	freeAllImagesData(imagesList,numOfImages);
}

void presentSimilarImagesNoGUI(int* imagesIndexesArray, int imagesCount){
	int i = 0;
	printf(CLOSEST_IMAGES);
	fflush(NULL);
	for (i = 0; i < imagesCount; i++) {
		printf("%d%s", imagesIndexesArray[i], i == (imagesCount - 1) ? "\n" : ", ");
		fflush(NULL);
	}
}


int* searchSimilarImages(const SPConfig config,SPImageData* imagesDatabase,SPImageData workingImage, int simmilarCount){
	return spIQ_getSimilarImages(config, imagesDatabase, workingImage, simmilarCount);
}

SP_CONFIG_MSG loadRelevantSettingsData(const SPConfig config, int* numOfImages, int* numOfSimilar, bool* extractFlag, bool* GUIFlag){
	SP_CONFIG_MSG rslt = SP_CONFIG_SUCCESS;
	*extractFlag = spConfigIsExtractionMode(config , &rslt);
	if (rslt != SP_CONFIG_SUCCESS)
		return rslt;

	*numOfImages = spConfigGetNumOfImages(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS)
			return rslt;

	*GUIFlag = spConfigMinimalGui(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS)
				return rslt;

	*numOfSimilar = 5; //TODO - create a getter for "spNumOfSimilarImages" and use here
	/*if (rslt != SP_CONFIG_SUCCESS)
			return rslt;*/
	return rslt;
}

void initializeImagesDataList(SPImageData** imagesDataList, int numOfImages){
	int i;
	*imagesDataList = (SPImageData*)calloc(sizeof(SPImageData),numOfImages);
	for (i=0 ; i<numOfImages; i++){
		//extract each relevant image data
		(*imagesDataList)[i] = (SPImageData)malloc(sizeof(struct sp_image_data));
		if ((*imagesDataList)[i] == NULL){ //error allocating memory
			//TODO - report relevant error
			free((*imagesDataList));
			(*imagesDataList) = NULL;
			return;
		}
		(*imagesDataList)[i]->index = i;
	}
	setFeaturesMatrix(*imagesDataList);
}


SPImageData initializeWorkingImage(){
	SPImageData workingImage = NULL;
	workingImage = (SPImageData)malloc(sizeof(struct sp_image_data));
	if (workingImage == NULL)
	{
		return NULL; //TODO - log relevant error
	}
	workingImage->index = 0;
	return workingImage;
}
