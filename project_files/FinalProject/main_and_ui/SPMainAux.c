#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SPMainAux.h"
#include "SPImageQuery.h"
#include "../SPLogger.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../general_utils/SPUtils.h"


#define DEFAULT_CONFIG_FILE										"spcbir.config"
#define CANNOT_OPEN_MSG 										"The configuration file %s couldn’t be open\n"
#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE 					"Please enter image path:\n"
#define INVALID_CMD_LINE										"Invalid command line : use -c <config_filename>\n"
#define STDOUT													"stdout"
#define CLOSEST_IMAGES 											"Best candidates for - %s - are:\n"
#define EXITING 												"Exiting...\n"
#define QUERY_IMAGE_DEFAULT_INDEX 								0
#define QUERY_STRING_ERROR 										"Query is not in the correct format, or file is not available\n"
#define CONFIG_FILE_PATH_ARG									"-c"
#define READ_FILE_MODE											"r"

#define WARNING_CONFIG_ARG										"Warning, program is running with unknown arguments, did you mean -c ?"
#define WARNING_ZERO_FEATURES_FROM_IMAGE						"Warning, some images have zero features"
#define WARNING_IMAGES_COUNT_CROPPED							\
	"Warning - the number of similar > number of images, only 'number of images' similar images will be presented when querying"
#define WARNING_AT_IMAGES_FEATURES_FILE_PATH 					\
	"An image features file does not exists or not available, please follow the logger for further information"
#define WARNING_WRONG_FILE 										"wrong file path or file not available"
#define FAILED_PRESEINTING_IMAGES_NO_GUI_MODE					"Failed to present similar images at non-GUI mode"

#define ERROR_READING_SETTINGS 									"Could not load data from the configurations"
#define ERROR_AT_CREATEING_QUERY_IMAGE_ITEM 					"Error creating query image item"
#define ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS 				"Error creating image database items"
#define ERROR_AT_IMAGES_FILE_PATH 								"Error at images file path, image does not exists or not available"
#define ERROR_AT_PCA_FILE_PATH									"Error at generating PCA file path"
#define ERROR_AT_GET_CONFIG_FROM_FILE							"Get configuration from file failed with message: %s"
#define ERROR_AT_GET_LOGGER_FILENAME_FROM_CONFIG				"Failed to get logger filename from configuration with message %s"
#define ERROR_AT_GET_LOGGER_LEVEL_FROM_CONFIG					"Failed to get logger level from configuration with message %s"
#define ERROR_AT_CREATE_LOGGER									"Failed to create logger with message %s"
#define ERROR_AT_GET_IMAGE_PATH_FROM_CONFIG						"Failed to get image path from configuration"
#define ERROR_PARSING_IMAGES_DATA 								"Failed at images parsing process"
#define ERROR_INITIALIZING_QUERY_IMAGE 							"Failed to initialize query image item"
#define ERROR_CREATING_FEATURES_ARRAY 							"Failed to create features array"
#define ERROR_CREATING_KD_TREE 									"Failed to create the KD-tree"
#define ERROR_INITIALIZING_BP_QUEUE 							"Failed to initialize priority queue"
#define ERROR_AT_IMAGES_COUNTS									"Error at verifying images counts numbers limits (from settings)"

#define MAIN_RETURNED_ERROR										"An error has been encountered, please check the log file for more information.\n"

#define DEBUG_IMAGES_PRESENTED_NON_GUI							"Similar images are being presented, non GUI mode"
#define DEBUG_IMAGES_PARSER_FINISHED 							"Images parser finished its work successfully"
#define DEBUG_WORKING_IMAGE_INITIALIZED 						"Working image initialized successfully"
#define DEBUG_NUMBER_OF_FEATURES_CALCULATED						"Total number of features calculated"
#define DEBUG_FEATURES_ARRAY_INITIALIZED						"Features array initialized"
#define DEBUG_KD_TREE_INITIALIZED  								"KD Tree initialized"
#define DEBUG_PRIORITY_QUEUE_INITIALIZED						"Priority Queue initialized"
#define DEBUG_PCA_FILE_IS_VERIFIED 								"PCA File is verified"
#define DEBUG_IMAGE_FILE_IS_VERIFIED_AT_INDEX 					"Image file is verified at index - "
#define DEBUG_IMAGE_FEAT_FILE_IS_VERIFIED_AT_INDEX				"Image .feat file is verified at index - "
#define DEBUG_LOGGER_HAS_BEEN_CREATED  							"Logger has been created"
#define DEBUG_RELEVANT_SETTINGS_DATA_LOADED						"Relevant settings data loaded"

char* getConfigFilename(int argc, char** argv) {
	if (argc == 1)
		return DEFAULT_CONFIG_FILE;
	if (argc == 3) {
		if (!strcmp(argv[1], CONFIG_FILE_PATH_ARG)){
			return argv[2];
		}
		else { // logger is not initialized yet
			printf(WARNING_CONFIG_ARG);
		}
	}
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

	//TODO - Matan - change macro to return false and not NULL
	spVerifyArgumentsRnNc(path != NULL, WARNING_WRONG_FILE);
	spValNc((fp = fopen(path, READ_FILE_MODE))!= NULL, WARNING_WRONG_FILE, false);
	fclose(fp);

	return true;
}

void getQuery(char* destination) {
	getAsString(ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE, destination);
}

void getAsString(const char* message, char* destination) {
	printf("%s", message);
	scanf("%s", destination);
}

void endControlFlow(SPConfig config, SPImageData image,
		bool isCurrentImageFeaturesArrayAllocated, SPKDTreeNode kdTree, SPBPQueue bpq,
		int returnValue) {
	if (returnValue < 0) {
		printf(MAIN_RETURNED_ERROR);
	}
	printf("%s", EXITING);
	spConfigDestroy(config);
	freeImageData(image, !isCurrentImageFeaturesArrayAllocated, true);
	spKDTreeDestroy(kdTree, true);
	spBPQueueDestroy(bpq);
	spLoggerDestroy();
}

void presentSimilarImagesNoGUI(char* queryImagePath, SPConfig config,
		int* imagesIndicesArray, int imagesCount) {
	int i = 0;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	char tmpImagePath[MAX_PATH_LEN];

	spVerifyArgumentsNc(queryImagePath != NULL && config != NULL &&
			imagesIndicesArray != NULL && imagesCount >= 0,
			FAILED_PRESEINTING_IMAGES_NO_GUI_MODE, ); // return;

	spLoggerSafePrintDebug(DEBUG_IMAGES_PRESENTED_NON_GUI, __FILE__, __FUNCTION__,
			__LINE__);

	printf(CLOSEST_IMAGES, queryImagePath);
	for (i = 0; i < imagesCount; i++) {
		spValWarning((msg = spConfigGetImagePath(tmpImagePath, config, imagesIndicesArray[i])) == SP_CONFIG_SUCCESS,
				WARNING_COULD_NOT_LOAD_IMAGE_PATH,
				printf(RELEVANT_IMAGE_INDEX_IS, imagesIndicesArray[i]),
				printf("%s\n", tmpImagePath));
	}
}

int calculateTotalNumOfFeatures(SPImageData* workingImagesDatabase, int numOfImages){
	int i, sum = 0;
	for (i = 0; i < numOfImages ; i++){
		if (workingImagesDatabase[i]->numOfFeatures == 0){
			spLoggerSafePrintWarning(WARNING_ZERO_FEATURES_FROM_IMAGE, __FILE__,__FUNCTION__, __LINE__);
		}
		sum += workingImagesDatabase[i]->numOfFeatures;
	}
	return sum;
}

SPPoint* initializeAllFeaturesArray(SPImageData* workingImagesDatabase, int numOfImages,
		int totalNumOfFeatures){
	SPPoint* featuresArray;
	int i, j, k = 0;
	spCalloc(featuresArray, SPPoint, totalNumOfFeatures);

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
	assert(config != NULL);

	*numOfImages = spConfigGetNumOfImages(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*numOfSimilar = spConfigGetNumOfSimilarImages(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*extractFlag = spConfigIsExtractionMode(config , &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*GUIFlag = spConfigMinimalGui(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	return rslt;
}

SPImageData* initializeImagesDataList(int numOfImages) {
	SPImageData* imagesDataList = NULL;
	int i;
	spCallocEr(imagesDataList, SPImageData, numOfImages,
			ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS, NULL);

	for (i = 0; i < numOfImages; i++){
		//extract each relevant image data
		spValWcRn(((imagesDataList)[i] = createImageData(i)) != NULL,
				ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS,
				//roll-back
				freeAllImagesData(imagesDataList, i, false)); // false because features list is not yet allocated
	}
	return imagesDataList;
}

SPImageData initializeWorkingImage() {
	SPImageData workingImage = NULL;
	spValRn((workingImage = createImageData(QUERY_IMAGE_DEFAULT_INDEX)) != NULL,
			ERROR_AT_CREATEING_QUERY_IMAGE_ITEM);
	return workingImage;
}

bool initializeWorkingImageKDTreeAndBPQueue(const SPConfig config,
		SPImageData* imagesDataList, SPImageData* currentImageData, SPKDTreeNode* kdTree,
		SPBPQueue* bpq, int numOfImages) {
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	int totalNumOfFeatures, knn;
	SPPoint* allFeaturesArray;
	SP_KDTREE_SPLIT_METHOD splitMethod;

	spVal(spImagesParserStartParsingProcess(config, imagesDataList) == SP_DP_SUCCESS,
			ERROR_PARSING_IMAGES_DATA, false);

	spLoggerSafePrintDebug(DEBUG_IMAGES_PARSER_FINISHED, __FILE__, __FUNCTION__, __LINE__);

	spVal((*currentImageData = initializeWorkingImage()), ERROR_INITIALIZING_QUERY_IMAGE,
			false);

	spLoggerSafePrintDebug(DEBUG_WORKING_IMAGE_INITIALIZED, __FILE__, __FUNCTION__,
			__LINE__);

	totalNumOfFeatures = calculateTotalNumOfFeatures(imagesDataList, numOfImages);

	spLoggerSafePrintDebug(DEBUG_NUMBER_OF_FEATURES_CALCULATED, __FILE__, __FUNCTION__,
			__LINE__);

	spVal((allFeaturesArray = initializeAllFeaturesArray(imagesDataList, numOfImages,
			totalNumOfFeatures)), ERROR_CREATING_FEATURES_ARRAY, false);

	spLoggerSafePrintDebug(DEBUG_FEATURES_ARRAY_INITIALIZED, __FILE__, __FUNCTION__,
			__LINE__);

	splitMethod = spConfigGetSplitMethod(config, &configMessage);
	spValWc(configMessage == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS,
			free(allFeaturesArray), false);

	spValWc((*kdTree = InitKDTreeFromPoints(allFeaturesArray, totalNumOfFeatures,
			splitMethod)), ERROR_CREATING_KD_TREE, free(allFeaturesArray), false);

	spLoggerSafePrintDebug(DEBUG_KD_TREE_INITIALIZED, __FILE__, __FUNCTION__, __LINE__);

	free(allFeaturesArray);

	knn = spConfigGetKNN(config, &configMessage);

	spVal(configMessage == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, false);

	spVal((*bpq = spBPQueueCreate(knn)), ERROR_INITIALIZING_BP_QUEUE, false);

	spLoggerSafePrintDebug(DEBUG_PRIORITY_QUEUE_INITIALIZED, __FILE__, __FUNCTION__,
			__LINE__);

	return true;
}

bool verifyImagesFiles(SPConfig config, int numOfImages, bool extractFlag) {
	char tempPath[MAX_PATH_LEN];
	int i;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	//verify PCA file
	spVal((msg = spConfigGetPCAPath(tempPath, config)) == SP_CONFIG_SUCCESS,
			ERROR_AT_PCA_FILE_PATH, false);

	spLoggerSafePrintDebug(DEBUG_PCA_FILE_IS_VERIFIED, __FILE__, __FUNCTION__, __LINE__);

	//verify images files
	for (i = 0;i < numOfImages ; i++) {
		if (extractFlag) {
			spVal((msg = spConfigGetImagePath(tempPath, config, i)) == SP_CONFIG_SUCCESS
					&& verifyPathAndAvailableFile(tempPath), ERROR_AT_IMAGES_FILE_PATH,
					false);

			spLoggerSafePrintDebugWithIndex(DEBUG_IMAGE_FILE_IS_VERIFIED_AT_INDEX,
					i, __FILE__, __FUNCTION__, __LINE__);
		}
		else {
			spValWarning((msg = spConfigGetImagePathFeats(tempPath, config, i, true))
					== SP_CONFIG_SUCCESS && verifyPathAndAvailableFile(tempPath),
					WARNING_AT_IMAGES_FEATURES_FILE_PATH, continue,
					spLoggerSafePrintDebugWithIndex(
							DEBUG_IMAGE_FEAT_FILE_IS_VERIFIED_AT_INDEX,
							i, __FILE__, __FUNCTION__, __LINE__));
		}
	}
	return true;
}

bool verifyImagesNumbersLimits(SPConfig config, int numOfImages, int* numOfSimilarImages){
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	spVerifyArguments(config != NULL && numOfImages > 0 && numOfSimilarImages != NULL &&
			*numOfSimilarImages > 0, ERROR_AT_IMAGES_COUNTS, false);

	if (*numOfSimilarImages > numOfImages) {
		spVal((msg = spConfigCropSimilarImages(config)) == SP_CONFIG_SUCCESS,
				ERROR_AT_IMAGES_COUNTS, false);
		spLoggerSafePrintWarning(WARNING_IMAGES_COUNT_CROPPED, __FILE__, __FUNCTION__,
				__LINE__);
		*numOfSimilarImages = numOfImages;
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
	spLoggerSafePrintDebug(DEBUG_LOGGER_HAS_BEEN_CREATED, __FILE__, __FUNCTION__,
			__LINE__);

	//load relevant data from settings
	if (loadRelevantSettingsData(*config, numOfImages, numOfSimilarImages, extractFlag,
			GUIFlag) != SP_CONFIG_SUCCESS) {
		return false;
	}
	spLoggerSafePrintDebug(DEBUG_RELEVANT_SETTINGS_DATA_LOADED, __FILE__, __FUNCTION__,
			__LINE__);

	return verifyImagesNumbersLimits(*config, *numOfImages, numOfSimilarImages) &&
			verifyImagesFiles(*config, *numOfImages, *extractFlag);
}

