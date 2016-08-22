#include <cstring>
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include "SPImageProc.h"

extern "C" {
#include "SPConfig.h"
#include "SPLogger.h"
#include "unit_tests/SPConfigUnitTest.h"
#include "SPImagesParser.h"
#include "SPPoint.h"
#include "unit_tests/SPImagesParserUnitTest.h"
#include "SPImageQuery.h"
#include "SPMainAux.h"
#include "unit_tests/SPKDArrayUnitTest.h"
#include "unit_tests/SPKDTreeNodeUnitTest.h"
#include "SPBPriorityQueue.h"
#include "unit_tests/SPKDTreeNodeKNNUnitTest.h"
#include "SPKDTreeNode.h"
}
#define MAXLINE_LEN 1024 //TODO - verify what this should be
#define QUERY_EXIT_INPUT "<>"

#define ERROR_LOADING_IMAGE_PATH "Error creating image path"
//TODO - verify calloc args order

int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config;
	int* similarImagesIndexes, numOfSimilarImages, i, numOfImages = 0;
	SPImageData currentImageData = NULL, *imagesDataList = NULL;
	char workingImagePath[MAXLINE_LEN], tempPath[MAXLINE_LEN] ;
	bool extractFlag, GUIFlag, oneImageWasSet = false;
	SPKDTreeNode kdTree = NULL;
	SPBPQueue bpq = NULL;

	if (!initConfigAndSettings(argc, argv, &config, &numOfImages, &numOfSimilarImages,
			&extractFlag, &GUIFlag)) {
		endControlFlow(config, currentImageData, imagesDataList, numOfImages,
				oneImageWasSet, kdTree, bpq);
		return -1;
	}

	//build features database
	sp::ImageProc imageProcObject(config);

	if (extractFlag) {
		initializeImagesDataList(&imagesDataList,numOfImages);
		if (imagesDataList == NULL) {
			endControlFlow(config, currentImageData, imagesDataList, numOfImages,
							oneImageWasSet, kdTree, bpq);
			return -1;
		}
		for (i = 0; i < numOfImages; i++){
			msg = spConfigGetImagePath(tempPath, config, i);
			if (msg != SP_CONFIG_SUCCESS) {
				spLoggerPrintError(ERROR_LOADING_IMAGE_PATH, __FILE__,__FUNCTION__, __LINE__);
				endControlFlow(config, currentImageData, imagesDataList, numOfImages,
								oneImageWasSet, kdTree, bpq);
				return -1;
			}
			imagesDataList[i]->featuresArray = imageProcObject.getImageFeatures(tempPath, i, &(imagesDataList[i]->numOfFeatures));
		}
	}

	if (!(initializeKDTreeAndBPQueue(config, &imagesDataList, &currentImageData, &kdTree,
			&bpq, numOfImages))) {
		endControlFlow(config, currentImageData, imagesDataList, numOfImages,
						oneImageWasSet, kdTree, bpq);
		return -1;
	}

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "<>"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT)) {
		oneImageWasSet = true;
		if (!verifyPathAndAvailableFile(workingImagePath)) {
			endControlFlow(config, currentImageData, imagesDataList, numOfImages,
							oneImageWasSet, kdTree, bpq);
			return -1;
		}
		if (currentImageData->featuresArray != NULL) {
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndexes = searchSimilarImages(currentImageData, kdTree, numOfImages,
				numOfSimilarImages, bpq);

		if (GUIFlag) {
			for (i=0;i<numOfSimilarImages;i++) {
				msg = spConfigGetImagePath(tempPath, config,similarImagesIndexes[i]);

				if (msg != SP_CONFIG_SUCCESS) {
					spLoggerPrintError(ERROR_LOADING_IMAGE_PATH, __FILE__,__FUNCTION__, __LINE__);
					endControlFlow(config, currentImageData, imagesDataList, numOfImages,
									oneImageWasSet, kdTree, bpq);
					return -1;
				}
				imageProcObject.showImage(tempPath);
			}
		} else {
			presentSimilarImagesNoGUI(similarImagesIndexes, numOfSimilarImages);
		}

		getQuery(workingImagePath);
	}

	// end control flow
	endControlFlow(config, currentImageData, imagesDataList, numOfImages, oneImageWasSet,
			kdTree, bpq);
	return 0;
}
/*
int main() {
	runConfigTests();
	return 0;
}


int main(){
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	RunImagesParserTests(config);
	spConfigDestroy(config);
	return 0;
}*/
/*
int main() {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	char* loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return -1; // TODO - maybe report relevant error (log still not initialized)

	spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	//runKDArrayTests();
	//runKDTreeNodeTests();
	runKDTreeNodeKNNTests();
	spConfigDestroy(config);
	return 0;
}*/

