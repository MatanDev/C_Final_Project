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

#define QUERY_EXIT_INPUT 							"<>"

#define ERROR_LOADING_IMAGE_PATH 					"Error creating image path"
#define ERROR_INIT_CONFIG 							"Error initializing configurations and settings"
#define ERROR_INIT_IMAGES 							"Error at initialize images data items process"
#define ERROR_INIT_KDTREE 							"Error building the kd-tree data structure"
#define ERROR_USER_QUERY 							"Error at user input. neither a valid image path, nor exit request"

#define CONFIG_AND_INIT_ERROR_RETURN_VALUE 			-1
#define IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE 		-2
#define QUERY_IMAGE_ERROR_RETURN_VALUE 				-3
#define LOADING_IMAGE_FAILED_RETURN_VALUE 			-4
#define SUCCESS_RETURN_VALUE 						0


/*
 * this macro is used to run the given 'action' and if it fails print the given
 * 'errorMessage' to the log, end the control flow and return -1
 */
#define spMainAction(action, returnValue) do { \
                if(!((action))) { \
					endControlFlow(config, currentImageData, oneImageWasSet, kdTree, bpq, returnValue);\
					delete imageProcObject;\
					return returnValue; \
                } \
        } while (0)

//TODO - remove asserts ? http://moodle.tau.ac.il/mod/forum/discuss.php?d=77675
//TODO - verify calloc args order
//TODO - forum: what to do with logger write return value
//TODO - expend to 3 methods - http://moodle.tau.ac.il/mod/forum/discuss.php?d=79730
//TODO - should write main.h ?


/*
 * TODO - doc this
 */
int spMainInitialize(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag, SPBPQueue* bpq,
		SPImageData* currentImageData, SPKDTreeNode* kdTree, sp::ImageProc** imageProcObject){
	int i;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	char tempPath[MAX_PATH_LEN];
	SPImageData *imagesDataList = NULL;

	spVal((initConfigAndSettings(argc, argv, config, numOfImages,
			numOfSimilarImages, extractFlag, GUIFlag)), ERROR_INIT_CONFIG, CONFIG_AND_INIT_ERROR_RETURN_VALUE );

	//TODO - maybe try catch?

	//build features database
	(*imageProcObject) = new sp::ImageProc(*config);

	if (extractFlag) {
		initializeImagesDataList(&imagesDataList, *numOfImages);
		spVal(imagesDataList, ERROR_INIT_IMAGES, IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);
		for (i = 0; i < *numOfImages; i++){
			msg = spConfigGetImagePath(tempPath, *config, i);
			spValWc((msg == SP_CONFIG_SUCCESS), ERROR_LOADING_IMAGE_PATH,
					freeAllImagesData(imagesDataList, *numOfImages,true),
					IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);
			imagesDataList[i]->featuresArray = (*imageProcObject)->getImageFeatures(tempPath,
					i, &(imagesDataList[i]->numOfFeatures));
		}
	}

	spValWc((initializeWorkingImageKDTreeAndBPQueue(*config, &imagesDataList,
		currentImageData, kdTree, bpq, *numOfImages)), ERROR_INIT_KDTREE,
			freeAllImagesData(imagesDataList, *numOfImages, true),
			IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);

	freeAllImagesData(imagesDataList, *numOfImages, true);

	return SUCCESS_RETURN_VALUE;
}

/*
 * TODO - doc this
 */
int spMainStartUserInteraction(SPConfig config,SPImageData currentImageData, SPKDTreeNode kdTree,int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq, bool GUIFlag, sp::ImageProc** imageProcObject, bool* oneImageWasSet){
	char workingImagePath[MAX_PATH_LEN], tempPath[MAX_PATH_LEN];
	int *similarImagesIndices = NULL;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	int i;

	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "<>"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT)) {
		*oneImageWasSet = true;
		//TODO - if not a good path ask again - http://moodle.tau.ac.il/mod/forum/discuss.php?d=77911
		spVal((verifyPathAndAvailableFile(workingImagePath)), ERROR_USER_QUERY, QUERY_IMAGE_ERROR_RETURN_VALUE);
		if (currentImageData->featuresArray != NULL) {
			free(currentImageData->featuresArray);
		}
		currentImageData->featuresArray = (*imageProcObject)->getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));
		similarImagesIndices = searchSimilarImages(currentImageData, kdTree, numOfImages,
				numOfSimilarImages, bpq);

		//TODO - check minimal gui at schrieber or at least at my linux vm
		//TODO - maybe extract to another method
		if (GUIFlag) {
			for (i=0;i<numOfSimilarImages;i++) {
				msg = spConfigGetImagePath(tempPath, config, similarImagesIndices[i]);
				spValWc((msg == SP_CONFIG_SUCCESS), ERROR_LOADING_IMAGE_PATH,
						free(similarImagesIndices), LOADING_IMAGE_FAILED_RETURN_VALUE);
				(*imageProcObject)->showImage(tempPath);
			}
		} else {
			presentSimilarImagesNoGUI(workingImagePath, config, similarImagesIndices,
					numOfSimilarImages);
		}

		free(similarImagesIndices);
		similarImagesIndices = NULL;

		getQuery(workingImagePath);
	}
	return SUCCESS_RETURN_VALUE;
}


/*
 * The main function of the project, loads all the relevant data according
 * to the settings file, afterwards interacts with the user to get his queries
 * the interaction ends in case of a failure or the use of '<>'
 *
 * All errors data that can, will be written to the log file
 *
 * specific settings file will be loaded using the parameter '-c'
 *
 * @param argc - the arguments count
 * @param argv - the main arguments
 *
 * @returns :
 * '-1' - configuration and setting initialization failed
 * '-2' - extracting images data, and loading images logic failed
 * '-3' - failed to load query image
 * '-4' - failed to load image for GUI presentation
 * '0'  - success
 */
int main(int argc, char** argv) {
	int flowFlag;
	SPConfig config = NULL;
	int numOfSimilarImages, numOfImages = 0;
	SPImageData currentImageData = NULL;
	bool extractFlag, GUIFlag, oneImageWasSet = false;
	SPKDTreeNode kdTree = NULL;
	SPBPQueue bpq = NULL;
	sp::ImageProc* imageProcObject = NULL;

	spMainAction(((flowFlag = spMainInitialize(argc, argv, &config, &numOfImages,
			&numOfSimilarImages, &extractFlag, &GUIFlag, &bpq,
			&currentImageData, &kdTree, &imageProcObject)) >= 0), flowFlag);

	spMainAction(((flowFlag = spMainStartUserInteraction(config,currentImageData, kdTree,numOfImages,
			 numOfSimilarImages,  bpq,  GUIFlag,  &imageProcObject, &oneImageWasSet)) >= 0), flowFlag);


	// end control flow
	endControlFlow(config, currentImageData, oneImageWasSet, kdTree, bpq, SUCCESS_RETURN_VALUE);
	delete imageProcObject;
	return SUCCESS_RETURN_VALUE;
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
