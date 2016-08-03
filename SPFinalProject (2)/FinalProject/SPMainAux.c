#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SPMainAux.h"
#include "SPImageQuery.h"
#include "SPLogger.h"

#define DEFAULT_CONFIG_FILE	"spcbir.config"
#define CANNOT_OPEN_MSG "The configuration file %s couldn’t be open\n"
#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Enter a query image or # to terminate:\n"
#define CLOSEST_IMAGES "The closest images are: "
#define EXITING "Exiting...\n"
#define QUERY_IMAGE_DEFAULT_INDEX 0
#define QUERY_STRING_ERROR 	"Query is not in the correct format, or file is not available\n"

#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"

const char* getConfigFilename(int argc, char** argv) {
	if (argc == 1)
		return DEFAULT_CONFIG_FILE;
	if (argc == 3 && !strcmp(argv[1], "-c"))
		return argv[2];
	return NULL;
}

SPConfig getConfigFromFile(const char* configFilename, SP_CONFIG_MSG* msg) {
	SPConfig config;
	config = spConfigCreate(configFilename, msg);
	if (*msg == SP_CONFIG_CANNOT_OPEN_FILE)
		printf(CANNOT_OPEN_MSG, configFilename);
	return config;
}

SP_LOGGER_MSG initializeLogger(int loggerLevel, const char* loggerFilename) {
	assert(1 <= loggerLevel <= 4);
	switch (loggerLevel) {
	case 1:
		return spLoggerCreate(loggerFilename, SP_LOGGER_ERROR_LEVEL);
	case 2:
		return spLoggerCreate(loggerFilename, SP_LOGGER_WARNING_ERROR_LEVEL);
	case 3:
		return spLoggerCreate(loggerFilename, SP_LOGGER_INFO_WARNING_ERROR_LEVEL);
	case 4:
		return spLoggerCreate(loggerFilename, SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL);
	}
}


bool verifyPathAndAvailableFile(char* path){
	FILE* fp;
	if (path == NULL)
		return false;
   fp = fopen(path,"r");
   if (fp == NULL){
	   //TODO log error QUERY_STRING_ERROR
	   printf(QUERY_STRING_ERROR);
	   return false;
   }
   fclose(fp);
   return true;
}

void getQuery(char* destination){
	//TODO - need to verify the user's input somehow
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
	workingImage->index = QUERY_IMAGE_DEFAULT_INDEX;
	return workingImage;
}
