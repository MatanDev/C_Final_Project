#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SPMainAux.h"
#include "SPImageQuery.h"
#include "../SPLogger.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"

//TODO - remove fflush(NULL) at production
#define MAXLINE_LEN 											1024 //TODO - verify what this should be
#define MAX_FILE_PATH_LENGTH									1024
#define DEFAULT_CONFIG_FILE										"spcbir.config"
#define CANNOT_OPEN_MSG 										"The configuration file %s couldn’t be open\n"
#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE 					"Please enter image path:\n"
#define INVALID_CMD_LINE										"Invalid command line : use -c <config_filename>\n"
#define STDOUT													"stdout"
#define CLOSEST_IMAGES 											"Best candidates for - %s - are:\n"
#define EXITING 												"Exiting...\n"
#define QUERY_IMAGE_DEFAULT_INDEX 								0
#define QUERY_STRING_ERROR 										"Query is not in the correct format, or file is not available\n"

#define ERROR_ALLOCATING_MEMORY 								"Could not allocate memory"
#define ERROR_INVALID_ARGUMENT 									"Error Invalid argument"
#define ERROR_WRONG_FILE 										"Error, wrong file path or file not available"
#define ERROR_READING_SETTINGS 									"Could not load data from the configurations"
#define ERROR_AT_CREATEING_QUERY_IMAGE_ITEM 					"Error creating query image item"
#define ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS 				"Error creating image database items"
#define ERROR_AT_IMAGES_FILE_PATH 								"Error at images file path, image does not exists or not available"
#define ERROR_AT_IMAGES_FEATURES_FILE_PATH 						"Error at images features file path, image does not exists or not available"
#define ERROR_AT_PCA_FILE_PATH									"Error at PCA file path, does not exists or not available"
#define ERROR_AT_GET_CONFIG_FROM_FILE							"Get configuration from file failed with message: %s"
#define ERROR_AT_GET_LOGGER_FILENAME_FROM_CONFIG				"Failed to get logger filename from configuration with message %s"
#define ERROR_AT_GET_LOGGER_LEVEL_FROM_CONFIG					"Failed to get logger level from configuration with message %s"
#define ERROR_AT_CREATE_LOGGER									"Failed to create logger with message %s"
#define ERROR_AT_GET_IMAGE_PATH_FROM_CONFIG						"Failed to get image path from configuration"

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
	FILE* fp;

	if (path == NULL) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,__FUNCTION__, __LINE__);
		return false;
	}

	fp = fopen(path,"r");

	if (fp == NULL){
	   spLoggerPrintError(ERROR_WRONG_FILE, __FILE__,__FUNCTION__, __LINE__);
	   //printf(QUERY_STRING_ERROR);
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
	if (image)
		freeImageData(image, !oneImageWasSet);
	if (imagesList) // TODO - not enough to avoid log error - elem in the arr can be NULL
		freeAllImagesData(imagesList,numOfImages);
	spKDTreeDestroy(kdTree);
	spBPQueueDestroy(bpq);
	spLoggerDestroy();
}

void presentSimilarImagesNoGUI(char* queryImagePath, SPConfig config,
		int* imagesIndicesArray, int imagesCount) {
	int i = 0;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	char tmpImagePath[MAX_FILE_PATH_LENGTH];

	if (queryImagePath == NULL || config == NULL || imagesIndicesArray == NULL ||
			imagesCount < 0) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,__FUNCTION__, __LINE__);
		return;
	}

	printf(CLOSEST_IMAGES, queryImagePath);
	fflush(NULL);
	for (i = 0; i < imagesCount; i++) {
		msg = spConfigGetImagePath(tmpImagePath, config, imagesIndicesArray[i]);
		if (msg != SP_CONFIG_SUCCESS) {
			spLoggerPrintError(ERROR_AT_GET_IMAGE_PATH_FROM_CONFIG, __FILE__, __FUNCTION__,
					__LINE__);
			return;
		}
		printf("%s\n", tmpImagePath);
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
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*numOfImages = spConfigGetNumOfImages(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*GUIFlag = spConfigMinimalGui(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
		return rslt;
	}

	*numOfSimilar = spConfigGetNumOfSimilarImages(config, &rslt);
	if (rslt != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
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

bool initializeWorkingImageKDTreeAndBPQueue(const SPConfig config,
		SPImageData** imagesDataList, SPImageData* currentImageData, SPKDTreeNode* kdTree,
		SPBPQueue* bpq, int numOfImages) {
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
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
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
		spLoggerPrintError(ERROR_READING_SETTINGS, __FILE__,__FUNCTION__, __LINE__);
		return false;
	}

	if (!(*bpq = spBPQueueCreate(knn))) {
		return false;
	}

	return true;
}

bool verifyImagesFiles(SPConfig config, int numOfImages, bool extractFlag){
	char tempPath[MAXLINE_LEN];
	int i;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	//verify PCA file
	msg = spConfigGetPCAPath(tempPath, config);
	if (msg != SP_CONFIG_SUCCESS || !verifyPathAndAvailableFile(tempPath)) {
		spLoggerPrintError(ERROR_AT_PCA_FILE_PATH, __FILE__,
				__FUNCTION__, __LINE__);
		return false;
	}
	//verify images files
	for (i = 0;i < numOfImages ; i++){
		msg = spConfigGetImagePath(tempPath, config, i);
		if (msg != SP_CONFIG_SUCCESS || !verifyPathAndAvailableFile(tempPath)) {
			spLoggerPrintError(ERROR_AT_IMAGES_FILE_PATH, __FILE__,
					__FUNCTION__, __LINE__);
			return false;
		}

		if (!extractFlag){
			msg = spConfigGetImagePathFeats(tempPath, config, i, true);
			if (msg != SP_CONFIG_SUCCESS || !verifyPathAndAvailableFile(tempPath)) {
				spLoggerPrintError(ERROR_AT_IMAGES_FEATURES_FILE_PATH, __FILE__,
						__FUNCTION__, __LINE__);
				return false;
			}
		}
	}
	return true;
}

bool initConfigAndSettings(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag) {
	char *configFilename, *loggerFilename;
	SP_CONFIG_MSG configMsg = SP_CONFIG_SUCCESS;
	SP_LOGGER_LEVEL loggerLevel;
	SP_LOGGER_MSG loggerMsg;

	if (!(configFilename = getConfigFilename(argc, argv))) {
		printf(INVALID_CMD_LINE);
		return false;
	}

	if (!(*config = getConfigFromFile(configFilename, &configMsg)) ||
			configMsg != SP_CONFIG_SUCCESS) {
		printf(ERROR_AT_GET_CONFIG_FROM_FILE, configMsgToStr(configMsg));
		return false;
	}

	loggerFilename = spConfigGetLoggerFilename(*config, &configMsg);
	if (loggerFilename == NULL || configMsg != SP_CONFIG_SUCCESS) {
		printf(ERROR_AT_GET_LOGGER_FILENAME_FROM_CONFIG, configMsgToStr(configMsg));
		return false;
	}

	loggerLevel = spConfigGetLoggerLevel(*config, &configMsg);

	if (configMsg != SP_CONFIG_SUCCESS) {
		printf(ERROR_AT_GET_LOGGER_LEVEL_FROM_CONFIG, configMsgToStr(configMsg));
		return false;
	}

	loggerMsg = spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			loggerLevel);

	if (loggerMsg != SP_LOGGER_SUCCESS) {
		printf(ERROR_AT_CREATE_LOGGER, loggerMsgToStr(loggerMsg));
		return false;
	}

	//load relevant data from settings
	if (loadRelevantSettingsData(*config, numOfImages, numOfSimilarImages, extractFlag,
			GUIFlag) != SP_CONFIG_SUCCESS) {
		return false;
	}

	return verifyImagesFiles(*config, *numOfImages, *extractFlag);
}

