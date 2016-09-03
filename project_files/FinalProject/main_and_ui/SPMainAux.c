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

//TODO - remove fflush(NULL) at production
#define DEFAULT_CONFIG_FILE										"spcbir.config"
#define CANNOT_OPEN_MSG 										"The configuration file %s couldn�t be open\n"
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
#define ERROR_PRESEINTING_IMAGES_NO_GUI_MODE					"Failed to present similar images at non-GUI mode"
#define ERROR_PARSING_IMAGES_DATA 								"Failed at images parsing process"
#define ERROR_INITIALIZING_QUERY_IMAGE 							"Failed to initialize query image item"
#define ERROR_CREATING_FEATURES_ARRAY 							"Failed to create features array"
#define ERROR_CREATING_KD_TREE 									"Failed to create the KD-tree"
#define ERROR_INITIALIZING_BP_QUEUE 							"Failed to initialize priority queue"
#define ERROR_AT_IMAGES_COUNTS									"Error at verifying images counts numbers limits (from settings)"

#define MAIN_RETURNED_ERROR										"An error has been encountered, please check the log file for more information.\n"


char* getConfigFilename(int argc, char** argv) {
	if (argc == 1)
		return DEFAULT_CONFIG_FILE;
	if (argc == 3) {
		if (!strcmp(argv[1], CONFIG_FILE_PATH_ARG)){
			return argv[2];
		}
		else { // logger is not yet initialized
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

	spVerifyArgumentsRn(path != NULL, ERROR_WRONG_FILE);
	spVal((fp = fopen(path, READ_FILE_MODE))!= NULL, ERROR_WRONG_FILE, false);
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

void endControlFlow(SPConfig config, SPImageData image,
		bool isCurrentImageFeaturesArrayAllocated, SPKDTreeNode kdTree, SPBPQueue bpq, int returnValue) {
	if (returnValue < 0){
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

	spVerifyArguments(queryImagePath != NULL && config != NULL && imagesIndicesArray != NULL &&
			imagesCount >= 0, ERROR_PRESEINTING_IMAGES_NO_GUI_MODE, ); //returns;

	printf(CLOSEST_IMAGES, queryImagePath);
	fflush(NULL);
	for (i = 0; i < imagesCount; i++) {
		spValWarning((msg = spConfigGetImagePath(tmpImagePath, config, imagesIndicesArray[i])) == SP_CONFIG_SUCCESS,
				WARNING_COULD_NOT_LOAD_IMAGE_PATH,
				printf(RELEVANT_IMAGE_INDEX_IS, imagesIndicesArray[i]),
				printf("%s\n", tmpImagePath));
		fflush(NULL);
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
	assert( config != NULL);

	*extractFlag = spConfigIsExtractionMode(config , &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*numOfImages = spConfigGetNumOfImages(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*GUIFlag = spConfigMinimalGui(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	*numOfSimilar = spConfigGetNumOfSimilarImages(config, &rslt);
	spVal(rslt == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, rslt);

	return rslt;
}

SPImageData* initializeImagesDataList(int numOfImages) {
	SPImageData* imagesDataList = NULL;
	int i;
	spCallocEr(imagesDataList, SPImageData, numOfImages,
			ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS, NULL); //returns;

	for (i=0 ; i<numOfImages; i++){
		//extract each relevant image data
		spValWcRn(((imagesDataList)[i] = createImageData(i)) != NULL,
				ERROR_AT_CREATEING_IMAGES_DATABASE_ITEMS,
				//roll-back
				freeAllImagesData(imagesDataList,i,false)); //false because features list is not yet allocated
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

	spVal(spImagesParserStartParsingProcess(config, imagesDataList) == SP_DP_SUCCESS, ERROR_PARSING_IMAGES_DATA, false);

	spVal((*currentImageData = initializeWorkingImage()), ERROR_INITIALIZING_QUERY_IMAGE, false);

	totalNumOfFeatures = calculateTotalNumOfFeatures(imagesDataList, numOfImages);

	spVal((allFeaturesArray = initializeAllFeaturesArray(imagesDataList, numOfImages,
			totalNumOfFeatures)), ERROR_CREATING_FEATURES_ARRAY, false);

	splitMethod = spConfigGetSplitMethod(config, &configMessage);
	spValWc(configMessage == SP_CONFIG_SUCCESS,
			ERROR_READING_SETTINGS, free(allFeaturesArray),false);

	spValWc((*kdTree = InitKDTreeFromPoints(allFeaturesArray, totalNumOfFeatures,
			splitMethod)), ERROR_CREATING_KD_TREE,
			free(allFeaturesArray), false);

	free(allFeaturesArray);

	knn = spConfigGetKNN(config, &configMessage);

	spVal(configMessage == SP_CONFIG_SUCCESS, ERROR_READING_SETTINGS, false);

	spVal((*bpq = spBPQueueCreate(knn)), ERROR_INITIALIZING_BP_QUEUE, false);

	return true;
}

bool verifyImagesFiles(SPConfig config, int numOfImages, bool extractFlag){
	//of images (TA) and print a warning to the log (Or)
	char tempPath[MAX_PATH_LEN];
	int i;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	//verify PCA file
	spVal((msg = spConfigGetPCAPath(tempPath, config)) == SP_CONFIG_SUCCESS
			&& verifyPathAndAvailableFile(tempPath),
			ERROR_AT_PCA_FILE_PATH, false);

	//verify images files
	for (i = 0;i < numOfImages ; i++){
		spVal((msg = spConfigGetImagePath(tempPath, config, i)) == SP_CONFIG_SUCCESS
				&& verifyPathAndAvailableFile(tempPath),
				ERROR_AT_IMAGES_FILE_PATH, false);

		if (!extractFlag){
			//TODO - maybe load from image ?
			// or ignore ? http://moodle.tau.ac.il/mod/forum/discuss.php?d=79724
			spVal((msg = spConfigGetImagePathFeats(tempPath, config, i, true)) == SP_CONFIG_SUCCESS
					&& verifyPathAndAvailableFile(tempPath),
					ERROR_AT_IMAGES_FEATURES_FILE_PATH, false);
		}
	}
	return true;
}

bool verifyImagesNumbersLimits(SPConfig config, int numOfImages, int* numOfSimilarImages){
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	spVerifyArguments(config != NULL && numOfImages > 0 && numOfSimilarImages != NULL &&
			*numOfSimilarImages > 0,
			ERROR_AT_IMAGES_COUNTS, false);

	if (*numOfSimilarImages > numOfImages){
		spVal((msg = spConfigCropSimilarImages(config)) == SP_CONFIG_SUCCESS ,
				ERROR_AT_IMAGES_COUNTS, false);
		spLoggerSafePrintWarning(WARNING_IMAGES_COUNT_CROPPED,
						__FILE__, __FUNCTION__, __LINE__);
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

	//load relevant data from settings
	if (loadRelevantSettingsData(*config, numOfImages, numOfSimilarImages, extractFlag,
			GUIFlag) != SP_CONFIG_SUCCESS) {
		return false;
	}

	return verifyImagesNumbersLimits(*config, *numOfImages, numOfSimilarImages) &&
			verifyImagesFiles(*config, *numOfImages, *extractFlag);
}

