#include <cstring>
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include "SPImageProc.h"

extern "C" {
#include "SPConfig.h"
#include "SPLogger.h"
#include "SPConfigUnitTest.h"
#include "SPImagesParser.h"
#include "SPPoint.h"
#include "SPImagesParserUnitTest.h"
#include "SPImageQuery.h"
#include "SPMainAux.h"
}

#define INVALID_CMD_LINE	"Invalid command line : use -c <config_filename>\n"
#define MAXLINE_LEN 1024 //TODO - verify what this should be
#define QUERY_EXIT_INPUT "#"

#define ERROR_LOADING_IMAGE_PATH "Error creating image path"
#define STDOUT	"stdout"


/*int main(int argc, char** argv) {
	const char* configFilename;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config;
	const char* loggerFilename;
	SP_LOGGER_MSG loggerMsg;
	SP_DP_MESSAGES parserMessage = SP_DP_SUCCESS;
	int* similarImagesIndexes, countOfSimilar,i,numOfImages;
	SPImageData currentImageData;
	SPImageData* imagesDataList = NULL;
	char workingImagePath[MAXLINE_LEN], tempPath[MAXLINE_LEN] ;
	bool extractFlag, GUIFlag,oneImageWasSet = false;

	configFilename = getConfigFilename(argc, argv);
	if (!configFilename) {
		printf(INVALID_CMD_LINE);
		return -1; //exit - maybe return something...
	}

	config = getConfigFromFile(configFilename, &msg);
	if (config == NULL || msg != SP_CONFIG_SUCCESS)
		return -1; // TODO - maybe report relevant error (log still not initialized)

	loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return -1; // TODO - maybe report relevant error (log still not initialized)

	loggerMsg = spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	if (loggerMsg != SP_LOGGER_SUCCESS || msg != SP_CONFIG_SUCCESS)
		return -1 ;// TODO - maybe report relevant error (log not initialized)

	// call extract/non extract function according to Extraction Mode field in the config struct

	//load relevant data from settings
	msg = loadRelevantSettingsData(config, &numOfImages,&countOfSimilar,&extractFlag, &GUIFlag);
	if (msg != SP_CONFIG_SUCCESS) {
			return -1;
	}

	//build features database
	sp::ImageProc imageProcObject(config);


	if (extractFlag) {
		initializeImagesDataList(&imagesDataList,numOfImages);
		if (imagesDataList == NULL) {
			return -1;
		}
		for (i=0 ; i<numOfImages; i++){
			msg = spConfigGetImagePath(tempPath, config,i);
			if (msg != SP_CONFIG_SUCCESS){
				spLoggerPrintError(ERROR_LOADING_IMAGE_PATH, __FILE__,__FUNCTION__, __LINE__);
				return -1;
			}
			imagesDataList[i]->featuresArray = imageProcObject.getImageFeatures(tempPath,i,&(imagesDataList[i]->numOfFeatures));
		}
	}

	//handle images data
	imagesDataList = spImagesParserStartParsingProcess(config, &parserMessage);
	if (imagesDataList == NULL) {
		return -1;
	}

	currentImageData = initializeWorkingImage();
	if (currentImageData == NULL) {
		return -1;
	}

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "#"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT))
	{
		oneImageWasSet = true;
		if (!verifyPathAndAvailableFile(workingImagePath)){
			endControlFlow(config,currentImageData,imagesDataList,numOfImages, oneImageWasSet);
			return -1;
		}
		if (currentImageData->featuresArray != NULL){
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndexes = searchSimilarImages(config, imagesDataList, currentImageData, countOfSimilar);

		if (GUIFlag){
			for (i=0;i<countOfSimilar;i++){
				msg = spConfigGetImagePath(tempPath, config,similarImagesIndexes[i]);

				if (msg != SP_CONFIG_SUCCESS){
					endControlFlow(config,currentImageData,imagesDataList,numOfImages, oneImageWasSet);
					spLoggerPrintError(ERROR_LOADING_IMAGE_PATH, __FILE__,__FUNCTION__, __LINE__);
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
	endControlFlow(config,currentImageData,imagesDataList,numOfImages, oneImageWasSet);
	return 0;
}
*/
int main() {
	runConfigTests();
	return 0;
}

/*
int main(){
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	RunImagesParserTests(config);
	spConfigDestroy(config);
	return 0;
}*/

