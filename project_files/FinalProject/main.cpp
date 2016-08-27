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
#include "image_parsing/SPImagesParser.h"
#include "SPPoint.h"
#include "unit_tests/SPImagesParserUnitTest.h"
#include "main_and_ui/SPImageQuery.h"
#include "main_and_ui/SPMainAux.h"
#include "unit_tests/SPKDArrayUnitTest.h"
#include "unit_tests/SPKDTreeNodeUnitTest.h"
#include "data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "unit_tests/SPKDTreeNodeKNNUnitTest.h"
#include "data_structures/kd_ds/SPKDTreeNode.h"
}
#define MAX_FILE_PATH_LENGTH 1024
#define QUERY_EXIT_INPUT "<>"

#define ERROR_LOADING_IMAGE_PATH "Error creating image path"
#define ERROR_INIT_CONFIG "Error initializing configurations and settings"
#define ERROR_INIT_IMAGES "Error at initialize images data items process"
#define ERROR_INIT_KDTREE "Error building the kd-tree data structure"
#define ERROR_USER_QUERY "Error at user input. neither a valid image path, nor exit request"

//TODO - remove asserts ? http://moodle.tau.ac.il/mod/forum/discuss.php?d=77675
//TODO - verify calloc args order
//TODO - forum: what to do with logger write return value
//TODO - expend to 3 methods - http://moodle.tau.ac.il/mod/forum/discuss.php?d=79730
int main(int argc, char** argv) {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = NULL;
	int *similarImagesIndices = NULL, numOfSimilarImages, i, numOfImages = 0;
	SPImageData currentImageData = NULL, *imagesDataList = NULL;
	char workingImagePath[MAX_FILE_PATH_LENGTH], tempPath[MAX_FILE_PATH_LENGTH];
	bool extractFlag, GUIFlag, oneImageWasSet = false;
	SPKDTreeNode kdTree = NULL;
	SPBPQueue bpq = NULL;

	verifyAction((initConfigAndSettings(argc, argv, &config, &numOfImages,
		&numOfSimilarImages,&extractFlag, &GUIFlag)), ERROR_INIT_CONFIG);

	//TODO - maybe try catch?

	//build features database
	sp::ImageProc imageProcObject(config);
	//TODO - verify we don't need to clean this object.. http://moodle.tau.ac.il/mod/forum/discuss.php?d=79258

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

	verifyAction((initializeWorkingImageKDTreeAndBPQueue(config, &imagesDataList,
		&currentImageData, &kdTree, &bpq, numOfImages)), ERROR_INIT_KDTREE);

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "<>"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT)) {
		oneImageWasSet = true;
		//TODO - if not a good path ask again - http://moodle.tau.ac.il/mod/forum/discuss.php?d=77911
		verifyAction((verifyPathAndAvailableFile(workingImagePath)), ERROR_USER_QUERY);
		if (currentImageData->featuresArray != NULL) {
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = imageProcObject.getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndices = searchSimilarImages(currentImageData, kdTree, numOfImages,
				numOfSimilarImages, bpq);

		//TODO - check minimal gui at schrieber or at least at my linux vm
		if (GUIFlag) {
			for (i=0;i<numOfSimilarImages;i++) {
				msg = spConfigGetImagePath(tempPath, config, similarImagesIndices[i]);
				verifyAction((msg == SP_CONFIG_SUCCESS), ERROR_LOADING_IMAGE_PATH);
				imageProcObject.showImage(tempPath);
			}
		} else {
			presentSimilarImagesNoGUI(workingImagePath, config, similarImagesIndices,
					numOfSimilarImages);
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
	SPConfig config = spConfigCreate("./unit_tests/spcbir.config", &msg);
	char* loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return -1;

	spLoggerCreate(!strcmp(loggerFilename, "stdout") ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	//RunImagesParserTests(config);
	//runConfigTests();
	runKDArrayTests();
	//runKDTreeNodeTests();
	//runKDTreeNodeKNNTests();
	spConfigDestroy(config);
	spLoggerDestroy();
	return 0;
}
*/
