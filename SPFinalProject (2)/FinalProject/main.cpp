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
#include "SPImageQuery.h"
#include "SPMainAux.h"
}

#define DEFAULT_CONFIG_FILE	"spcbir.config"
#define MAXLINE_LEN 1024 //TODO - verify what this should be

int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SP_DP_MESSAGES parserMessage = SP_DP_SUCCESS;
	SPConfig config;
	int* similarImagesIndexes, countOfSimilar,i,numOfImages;
	SPImageData currentImageData;
	SPImageData* imagesDataList = NULL;
	const char* configFilename;
	char workingImagePath[MAXLINE_LEN], tempPath[MAXLINE_LEN] ;
	SP_LOGGER_MSG loggerMsg;
	bool extractFlag, GUIFlag;

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

	//load relevant data from settings
	msg = loadRelevantSettingsData(config, &numOfImages,&countOfSimilar,&extractFlag, &GUIFlag);
	if (msg != SP_CONFIG_SUCCESS) {
			//TODO - report relevant error
			return -1;
	}

	//build features database
	sp::ImageProc imageProcObject(config);


	if (extractFlag) {
		initializeImagesDataList(&imagesDataList,numOfImages);
		if (imagesDataList == NULL) {
			//TODO - report relevant error
			return -1;
		}
		for (i=0 ; i<numOfImages; i++){
			msg = spConfigGetImagePath(tempPath, config,i);
			if (msg != SP_CONFIG_SUCCESS){
				//TODO - report error
				return -1;
			}
			imagesDataList[i]->featuresArray = imageProcObject.getImageFeatures(tempPath,i,&(imagesDataList[i]->numOfFeatures));
		}
	}

	//handle images data
	imagesDataList = spImagesParserStartParsingProcess(config, &parserMessage);

	currentImageData = initializeWorkingImage();
	if (currentImageData == NULL) {
		return -1;
	}

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "#"
	while (strcmp(workingImagePath, "#"))
	{
		if (currentImageData->featuresArray != NULL){
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndexes = searchSimilarImages(config, imagesDataList, currentImageData, countOfSimilar);

		if (GUIFlag){
			for (i=0;i<countOfSimilar;i++){
				msg = spConfigGetImagePath(tempPath, config,similarImagesIndexes[i]);
				if (msg != SP_CONFIG_SUCCESS){
					//TODO - report error
					return -1;
				}
				imageProcObject.showImage(tempPath);
			}
		}
		else{
			presentSimilarImagesNoGUI(similarImagesIndexes, countOfSimilar);
		}

		getQuery(workingImagePath);
	}

	// end control flow
	endControlFlow(config,currentImageData,imagesDataList,numOfImages);
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
