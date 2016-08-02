#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "SPImageProc.h"

extern "C" {
#include "SPConfig.h"
#include "SPLogger.h"
#include "SPConfigUnitTest.h"
#include "SPImagesParser.h"
#include "SPPoint.h"
#include "SPImagesParserParserUnitTest.h"
#include "SPUI.h"
}

#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Enter a query image or # to terminate:\n"
#define EXITING "Exiting...\n"

#define DEFAULT_CONFIG_FILE	"spcbir.config"
#define MAXLINE_LEN 1024 //TODO - verify what this should be

int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SP_DP_MESSAGES parserMessage = SP_DP_SUCCESS;
	SPConfig config;
	SPImageData currentImageData;
	SPImageData* imagesDataList = NULL;
	const char* configFilename;
	char workingImagePath[MAXLINE_LEN], tempPath[MAXLINE_LEN] ;
	SP_LOGGER_MSG loggerMsg;
	int i,numOfImages;
	bool extractFlag;

	if (argc == 1) {
		configFilename = DEFAULT_CONFIG_FILE;
		config = spConfigCreate(configFilename, &msg);
		if (msg == SP_CONFIG_CANNOT_OPEN_FILE)
			printf("The default configuration file spcbir.config couldn’t be open\n");
	}
	else if (argc == 3 && !strcmp(argv[1], "-c")) {
		configFilename = argv[2];
		config = spConfigCreate(configFilename, &msg);
		if (msg == SP_CONFIG_CANNOT_OPEN_FILE)
			printf("The configuration file %s couldn’t be open\n", configFilename);
	}
	else {
		printf("Invalid command line : use -c <config_filename>\n");
		//exit - maybe return something...
	}

	switch (config->spLoggerLevel) {
	case 1:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_ERROR_LEVEL);
		break;
	case 2:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_WARNING_ERROR_LEVEL);
		break;
	case 3:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_INFO_WARNING_ERROR_LEVEL);
		break;
	case 4:
		loggerMsg = spLoggerCreate(config->spLoggerFilename, SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL);
		break;
	}
	assert(loggerMsg == SP_LOGGER_SUCCESS);

	// call extract/non extract function according to Extraction Mode field in the config struct


	//build features database
	sp::ImageProc imageProcObject(config);
	extractFlag = spConfigIsExtractionMode(config , &msg);
	numOfImages = spConfigGetNumOfImages(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		//TODO - report relevant error
		return -1;
	}
	if (extractFlag) {
		imagesDataList = (SPImageData*)calloc(sizeof(SPImageData),numOfImages);
		for (i=0 ; i<numOfImages; i++){
			//extract each relevant image data
			imagesDataList[i] = (SPImageData)malloc(sizeof(struct sp_image_data));
			if (imagesDataList[i] == NULL || tempPath == NULL){ //error allocating memory
				//TODO - report relevant error
				return -1;
			}
			msg = spConfigGetImagePath(tempPath, config,i);
			imagesDataList[i]->index = i;
			imagesDataList[i]->featuresArray = imageProcObject.getImageFeatures(tempPath,i,&(imagesDataList[i]->numOfFeatures));
		}
		setFeaturesMatrix(imagesDataList);
	}
	//else
	//	setFeaturesMatrix(NULL);

	imagesDataList = spImagesParserStartParsingProcess(config, &parserMessage);

	//spUI_beginUserInteraction(config, imagesDataList);
	currentImageData = (SPImageData)malloc(sizeof(struct sp_image_data));
	if (currentImageData == NULL)
	{
		return 0; //TODO - log relevant error
	}
	currentImageData->index = 0;
	// first run must always happen
	printf("%s", ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE);
	fflush(NULL);
	scanf("%s", workingImagePath);
	fflush(NULL);

	// iterating until the user inputs "#"
	while (strcmp(workingImagePath, "#"))
	{
		if (currentImageData->featuresArray != NULL){
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		spUI_searchSimilarImages(config, imagesDataList, currentImageData);

		printf("%s", ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE);
		fflush(NULL);
		scanf("%s", workingImagePath);
		fflush(NULL);
	}

	// announce the user for exiting
	printf("%s", EXITING);


	spConfigDestroy(config);
	freeImageData(currentImageData);
	freeAllImagesData(imagesDataList,numOfImages);
	return 0;
}
/*
int main() {
	testGivenConfFile();
	return 0;
}

int main(){
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	RunImagesParserTests(config);
	spConfigDestroy(config);
	return 0;
}
*/
