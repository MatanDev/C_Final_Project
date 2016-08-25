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

#define STDOUT	"stdout" //TODO - remove at production
#define ERROR_LOADING_IMAGE_PATH "Error creating image path"
#define ERROR_INIT_CONFIG "Error initializing configurations and settings"
#define ERROR_INIT_IMAGES "Error at initialize images data items process"
#define ERROR_INIT_KDTREE "Error building the kd-tree data structure"
#define ERROR_USER_QUERY "Error at user input. neither a valid image path, nor exit request"

//TODO - verify calloc args order

int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = NULL;
	int *similarImagesIndices = NULL, numOfSimilarImages, i, numOfImages = 0;
	SPImageData currentImageData = NULL, *imagesDataList = NULL;
	char workingImagePath[MAXLINE_LEN], tempPath[MAXLINE_LEN];
	bool extractFlag, GUIFlag, oneImageWasSet = false;
	SPKDTreeNode kdTree = NULL;
	SPBPQueue bpq = NULL;

	verifyAction((initConfigAndSettings(argc, argv, &config, &numOfImages,
		&numOfSimilarImages,&extractFlag, &GUIFlag)), ERROR_INIT_CONFIG);

	//TODO - in case the directory is invalid or pca file is invalid we fail here
	//we should check if the path is valid in the previous function
	//we have other cases of possible failures - think how to deal with them (try catch?)

	//build features database
	sp::ImageProc imageProcObject(config);

	if (extractFlag) {
		initializeImagesDataList(&imagesDataList,numOfImages);
		verifyAction(imagesDataList, ERROR_INIT_IMAGES);
		for (i = 0; i < numOfImages; i++){
			msg = spConfigGetImagePath(tempPath, config, i);
			verifyAction((msg == SP_CONFIG_SUCCESS), ERROR_LOADING_IMAGE_PATH);
			imagesDataList[i]->featuresArray = imageProcObject.getImageFeatures(tempPath,
					i, &(imagesDataList[i]->numOfFeatures));
		}
	}

	verifyAction((initializeKDTreeAndBPQueue(config, &imagesDataList, &currentImageData,
			&kdTree, &bpq, numOfImages)), ERROR_INIT_KDTREE);

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "<>"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT)) {
		oneImageWasSet = true;
		verifyAction((verifyPathAndAvailableFile(workingImagePath)), ERROR_USER_QUERY);
		if (currentImageData->featuresArray != NULL) {
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndices = searchSimilarImages(currentImageData, kdTree, numOfImages,
				numOfSimilarImages, bpq);

		if (GUIFlag) {
			for (i=0;i<numOfSimilarImages;i++) {
				msg = spConfigGetImagePath(tempPath, config, similarImagesIndices[i]);
				verifyAction((msg == SP_CONFIG_SUCCESS), ERROR_LOADING_IMAGE_PATH);
				imageProcObject.showImage(tempPath);
			}
		} else {
			presentSimilarImagesNoGUI(similarImagesIndices, numOfSimilarImages);
		}

		free(similarImagesIndices);
		similarImagesIndices = NULL;

		getQuery(workingImagePath);
	}

	// end control flow
	endControlFlow(config, currentImageData, imagesDataList, numOfImages, oneImageWasSet,
			kdTree, bpq);
	return 0;
}

/*
int main() {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	char* loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return -1; // TODO - maybe report relevant error (log still not initialized)

	spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	RunImagesParserTests(config);
	//runConfigTests();
	//runKDArrayTests();
	//runKDTreeNodeTests();
	//runKDTreeNodeKNNTests();
	spConfigDestroy(config);
	return 0;
}
*/
