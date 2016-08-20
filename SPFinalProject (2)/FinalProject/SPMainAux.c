#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SPMainAux.h"
#include "SPImageQuery.h"
#include "SPLogger.h"

//TODO - remove fflush(NULL) at production

#define DEFAULT_CONFIG_FILE	"spcbir.config"
#define CANNOT_OPEN_MSG "The configuration file %s couldn’t be open\n"
#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Please enter image path:\n"
#define CLOSEST_IMAGES "The closest images are: " //TODO - dont forget to print output as requested
#define EXITING "Exiting...\n"
#define QUERY_IMAGE_DEFAULT_INDEX 0
#define QUERY_STRING_ERROR 	"Query is not in the correct format, or file is not available\n"

#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"

#define ERROR_INVALID_ARGUMENT "Error Invalid argument"
#define ERROR_WRONG_QUERY "Query is not a valid path, or file is not available"
#define ERROR_READING_SETTINGS "Could not load data from the configurations"

#define ERROR_AT_CREATEING_QUERY_IMAGE_ITEM "Error creating query image item"
#define ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS "Error creating image database items"



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


bool verifyPathAndAvailableFile(char* path) {
	if (path == NULL){
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,__FUNCTION__, __LINE__);
		return false;
	}
	FILE* fp;
	if (path == NULL)
		return false;
   fp = fopen(path,"r");
   if (fp == NULL){
	   spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
	   printf(QUERY_STRING_ERROR);
	   return false;
   }
   fclose(fp);
   return true;
}

void getQuery(char* destination) {
	getAsString(ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE, destination);
}

void getAsString(const char* message, char* destination) {
	printf("%s", message);
	fflush(NULL);
	scanf("%s", destination);
	fflush(NULL);
}

void endControlFlow(SPConfig config, SPImageData image, SPImageData* imagesList,
		int numOfImages, bool oneImageWasSet) {
	printf("%s", EXITING);
	spConfigDestroy(config);
	freeImageData(image, !oneImageWasSet);
	freeAllImagesData(imagesList,numOfImages);
}

void presentSimilarImagesNoGUI(int* imagesIndexesArray, int imagesCount) {
	int i = 0;

	if (imagesIndexesArray == NULL || imagesCount < 0){
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,__FUNCTION__, __LINE__);
		return;
	}

	printf(CLOSEST_IMAGES);
	fflush(NULL);
	for (i = 0; i < imagesCount; i++) {
		printf("%d%s", imagesIndexesArray[i], i == (imagesCount - 1) ? "\n" : ", ");
		fflush(NULL);
	}
}


int* searchSimilarImages(const SPConfig config,SPImageData* imagesDatabase,
		SPImageData workingImage, int simmilarCount) {
	return spIQ_getSimilarImages(config, imagesDatabase, workingImage, simmilarCount);
}

SP_CONFIG_MSG loadRelevantSettingsData(const SPConfig config, int* numOfImages,
		int* numOfSimilar, bool* extractFlag, bool* GUIFlag) {
	SP_CONFIG_MSG rslt = SP_CONFIG_SUCCESS;
	assert( config != NULL);

	*extractFlag = spConfigIsExtractionMode(config , &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*numOfImages = spConfigGetNumOfImages(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*GUIFlag = spConfigMinimalGui(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*numOfSimilar = spConfigGetNumOfSimilarImages(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	return rslt;
}

void initializeImagesDataList(SPImageData** imagesDataList, int numOfImages) {
	int i,j;
	*imagesDataList = (SPImageData*)calloc(sizeof(SPImageData),numOfImages);
	if ((*imagesDataList) == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS, __FILE__,__FUNCTION__, __LINE__);
		return;
	}
	for (i=0 ; i<numOfImages; i++){
		//extract each relevant image data
		(*imagesDataList)[i] = (SPImageData)malloc(sizeof(struct sp_image_data));
		if ((*imagesDataList)[i] == NULL){ //error allocating memory
			//TODO - report relevant error
			spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
			spLoggerPrintError(ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS, __FILE__,__FUNCTION__, __LINE__);
			//roll-back
			for (j=0;j<i;j++){
				free((*imagesDataList)[j]);
			}
			free((*imagesDataList));
			(*imagesDataList) = NULL;
			return;
		}
		(*imagesDataList)[i]->index = i;
	}
	setFeaturesMatrix(*imagesDataList);
}


SPImageData initializeWorkingImage() {
	SPImageData workingImage = NULL;
	workingImage = (SPImageData)malloc(sizeof(struct sp_image_data));
	if (workingImage == NULL)
	{
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_AT_CREATEING_QUERY_IMAGE_ITEM, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	workingImage->index = QUERY_IMAGE_DEFAULT_INDEX;
	return workingImage;
}
