#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SPMainAux.h"
#include "SPImageQuery.h"
#include "SPLogger.h"
#include "SPKDTreeNode.h"
#include "SPBPriorityQueue.h"

//TODO - remove fflush(NULL) at production

#define DEFAULT_CONFIG_FILE	"spcbir.config"
#define CANNOT_OPEN_MSG "The configuration file %s couldn�t be open\n"
#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Please enter image path:\n"
#define INVALID_CMD_LINE	"Invalid command line : use -c <config_filename>\n"
#define STDOUT	"stdout"
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



char* getConfigFilename(int argc, char** argv) {
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
		int numOfImages, bool oneImageWasSet, SPKDTreeNode kdTree, SPBPQueue bpq) {
	printf("%s", EXITING);
	spConfigDestroy(config);
	freeImageData(image, !oneImageWasSet);
	freeAllImagesData(imagesList,numOfImages);
	spKDTreeDestroy(kdTree);
	spBPQueueDestroy(bpq);
	spLoggerDestroy();
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

int calculateTotalNumOfFeatures(SPImageData* workingImagesDatabase, int numOfImages){
	int i, sum = 0;
	for (i = 0; i < numOfImages ; i++){
		sum += workingImagesDatabase[i]->numOfFeatures;
	}
	return sum;
}

SPPoint* initializeAllFeaturesArray(SPImageData* workingImagesDatabase, int numOfImages,
		int totalNumOfFeatures){
	SPPoint* featuresArray;
	int i, j, k = 0;
	if (!(featuresArray = (SPPoint*)calloc(sizeof(SPPoint), totalNumOfFeatures))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	for (i = 0; i < numOfImages; i++){
		for (j = 0 ;j < workingImagesDatabase[i]->numOfFeatures; j++) {
			featuresArray[k] = (workingImagesDatabase[i]->featuresArray)[j];
			k++;
		}
	}
	return featuresArray;
}

int* searchSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq) {
	return getSimilarImages(workingImage, kdTree, numOfImages, numOfSimilarImages, bpq);
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
	workingImage = (SPImageData)calloc(1, sizeof(struct sp_image_data));
	if (workingImage == NULL)
	{
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_AT_CREATEING_QUERY_IMAGE_ITEM, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	workingImage->index = QUERY_IMAGE_DEFAULT_INDEX;
	return workingImage;
}

bool initializeKDTreeAndBPQueue(const SPConfig config, SPImageData** imagesDataList,
		SPImageData* currentImageData, SPKDTreeNode* kdTree, SPBPQueue* bpq,
		int numOfImages) {
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	SP_DP_MESSAGES parserMessage = SP_DP_SUCCESS;
	int totalNumOfFeatures, knn;
	SPPoint* allFeaturesArray;
	SP_KDTREE_SPLIT_METHOD splitMethod;

	if (!(*imagesDataList = spImagesParserStartParsingProcess(config, &parserMessage))) {
		return false;
	}

	if (!(*currentImageData = initializeWorkingImage())) {
		return false;
	}

	totalNumOfFeatures = calculateTotalNumOfFeatures(*imagesDataList, numOfImages);

	if (!(allFeaturesArray = initializeAllFeaturesArray(*imagesDataList, numOfImages,
			totalNumOfFeatures))) {
		//TODO - talk with Matan about where it's better to free
		//freeAllImagesData(imagesDataList, numOfImages);
		return false;
	}

	splitMethod = spConfigGetSplitMethod(config, &configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		free(allFeaturesArray);
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return false;
	}

	if (!(*kdTree = InitKDTreeFromPoints(allFeaturesArray, totalNumOfFeatures,
			splitMethod))) {
		free(allFeaturesArray);
		//TODO - talk with Matan about where it's better to free
		//freeAllImagesData(imagesDataList, numOfImages);
		return false;
	}

	free(allFeaturesArray);
	//TODO - talk with Matan about where it's better to free
	//freeAllImagesData(imagesDataList, numOfImages);

	knn = spConfigGetKNN(config, &configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		//TODO - this is not a relevant error
		spLoggerPrintError(ERROR_WRONG_QUERY, __FILE__,__FUNCTION__, __LINE__);
		return false;
	}

	if (!(*bpq = spBPQueueCreate(knn))) {
		return false;
	}

	return true;
}

bool initConfigAndSettings(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag) {
	char *configFilename, *loggerFilename;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SP_LOGGER_LEVEL loggerLevel;

	if (!(configFilename = getConfigFilename(argc, argv))) {
		printf(INVALID_CMD_LINE);
		return false; //exit - maybe return something...
	}

	if (!(*config = getConfigFromFile(configFilename, &msg)) || msg != SP_CONFIG_SUCCESS)
		return false; // TODO - maybe report relevant error (log still not initialized)

	loggerFilename = spConfigGetLoggerFilename(*config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return false; // TODO - maybe report relevant error (log still not initialized)

	loggerLevel = spConfigGetLoggerLevel(*config, &msg);

	if (msg != SP_CONFIG_SUCCESS)
		return false;

	if (spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			loggerLevel) != SP_LOGGER_SUCCESS)
		return false;// TODO - maybe report relevant error (log not initialized)

	//load relevant data from settings
	if (loadRelevantSettingsData(*config, numOfImages, numOfSimilarImages, extractFlag,
			GUIFlag) != SP_CONFIG_SUCCESS) {
		return false;
	}
	return true;
}
