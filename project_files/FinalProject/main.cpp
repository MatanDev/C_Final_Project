#include <cstring>
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include "SPImageProc.h"

extern "C" {
#include "SPConfig.h"
#include "SPLogger.h"
#include "SPPoint.h"
#include "image_parsing/SPImagesParser.h"
#include "main_and_ui/SPImageQuery.h"
#include "main_and_ui/SPMainAux.h"
#include "data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "data_structures/kd_ds/SPKDTreeNode.h"
}

#define QUERY_EXIT_INPUT 							"<>"

#define ERROR_LOADING_IMAGE_PATH 					"Error creating image path"
#define ERROR_INIT_CONFIG 							"Error initializing configurations and settings"
#define ERROR_INIT_IMAGES 							"Error at initialize images data items process"
#define ERROR_INIT_KDTREE_OR_DATA 					"Error building the data structures"
#define REQUEST_QUERY_AGAIN							"Please enter a valid file path, or <> to exit.\n"
#define	FAIL_SEARCHING_IMAGES						"Failed during querying the database, thus similar images could not be found"
#define WRONG_USER_QUERY 							"Wrong user input. neither a valid image path, nor exit request"
#define DEBUG_IMAGES_PRESENTED_GUI					"Similar images are being presented - GUI mode"

#define RUN_ACTION									false

#define CONFIG_AND_INIT_ERROR_RETURN_VALUE 			-1
#define IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE 		-2
#define SUCCESS_RETURN_VALUE 						0

/*------------------------------------------------------------ logger info -------------------------------------------------------------------------*/
#define EXTRACTED_IMAGES_DATA  						"Extracted images data from images files"
#define IMAGED_DATABASE_INITIALIZATION_COMPLETED  	"Imaged database list basic initialization completed"
#define CONFIGURATIONS_AND_LOGGER_INITIALIZED  		"Configurations and logger initialized"
#define USER_INTERACTION_FINISHED_SUCCESSFULLY  	"User interaction finished successfully, releasing resources and exiting"
#define INITIALIZATION_FINISHED_SUCCESSFUL  		"Initialization finished successful"
#define SIMILAR_IMAGES_PRESENTED					"Similar images to the last query has been presented to the user"
#define QUERY_HAS_BEEN_INSERTED 					"A legal query has been inserted by the user : "
#define ILLEGAL_QUERY_HAS_BEEN_INSERTED 			"An illegal query has been inserted by the user : "
#define INTERNAL_DATA_AND_LOGIC_CREATED 			"Internal data and logic layer has been created successfully, the user can start querying now"
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * this macro is used to run the given 'action' and if it fails print the given
 * 'errorMessage' to the log, end the control flow and return a given return value
 */
#define spMainAction(action, returnValue) do { \
                if(!((action))) { \
					endControlFlow(config, currentImageData, isCurrentImageFeaturesArrayAllocated, kdTree, bpq, returnValue);\
					delete imageProcObject;\
					return returnValue; \
                } \
        } while (0)

//TODO - logger documentation in inner functions (convention)
//TODO - remove fflush(NULL) at production
//TODO - http://moodle.tau.ac.il/2015/mod/forum/discuss.php?d=80225
/*
 * The method initializes the project, loads the settings, the logger, the images data and build's
 * the KD data structure with the images data, it loads the data into the pointers that are
 * given as parameters
 *
 * @param argc - the count of arguments from the main method
 * @param argv - the arguments from the main method
 * @param config - a pointer to the config item
 * @param numOfImages - a pointer to the number of images integer
 * @param numOfSimilarImages - a pointer to the number of similar images integer
 * @param extractFlag - a pointer to the extraction flag
 * @param GUIFlag - a pointer to the GUI flag
 * @param bpq - a pointer for the priority queue
 * @param currentImageData - a pointer for an image data that needs to be allocated
 * @param kdTree - a pointer to the kd-tree
 * @param imageProbObject - a pointer to the image proc object pointer
 *
 * @returns :
 * '-1' - configuration and setting initialization failed
 * '-2' - extracting images data, and loading images logic failed
 * '0'  - success
 */
int spMainInitialize(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag, SPBPQueue* bpq,
		SPImageData* currentImageData, SPKDTreeNode* kdTree, sp::ImageProc** imageProcObject){
	int i;
	char tempPath[MAX_PATH_LEN];
	SPImageData* imagesDataList = NULL;

	spVal((initConfigAndSettings(argc, argv, config, numOfImages,
			numOfSimilarImages, extractFlag, GUIFlag)), ERROR_INIT_CONFIG, CONFIG_AND_INIT_ERROR_RETURN_VALUE );
	spLoggerSafePrintInfo(CONFIGURATIONS_AND_LOGGER_INITIALIZED);

	spVal(imagesDataList = initializeImagesDataList(*numOfImages),
			ERROR_INIT_IMAGES, IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);

	spLoggerSafePrintInfo(IMAGED_DATABASE_INITIALIZATION_COMPLETED);

	//build features database
	(*imageProcObject) = new sp::ImageProc(*config);
	if (*extractFlag) {
		for (i = 0; i < *numOfImages; i++){
			spValWc((spConfigGetImagePath(tempPath, *config, i) == SP_CONFIG_SUCCESS),
					ERROR_LOADING_IMAGE_PATH,
					freeAllImagesData(imagesDataList, *numOfImages,true),
					IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);
			imagesDataList[i]->featuresArray = (*imageProcObject)->getImageFeatures(tempPath,
					i, &(imagesDataList[i]->numOfFeatures));
		}
		spLoggerSafePrintInfo(EXTRACTED_IMAGES_DATA);
	}

	spValWc((initializeWorkingImageKDTreeAndBPQueue(*config, imagesDataList,
		currentImageData, kdTree, bpq, *numOfImages)), ERROR_INIT_KDTREE_OR_DATA,
			freeAllImagesData(imagesDataList, *numOfImages, true),
			IMAGE_DATA_LOGIC_ERROR_RETURN_VALUE);

	freeAllImagesData(imagesDataList, *numOfImages, false);

	spLoggerSafePrintInfo(INTERNAL_DATA_AND_LOGIC_CREATED);


	return SUCCESS_RETURN_VALUE;
}


/*
 * The method gets a pre-validated query for an image path, requests the data layer i.e. the KD data-structure
 * for the similar images and presents them.
 * In case of a problem at presenting a specific image, a warning will be logged and a relevant message will
 * be shown, yet the process will keep running and try to present the next image.
 *
 * @param config - the configuration data
 * @param currentImageData - a pre-allocated image data to work with
 * @param imageProbObject - a pointer to the image proc object pointer
 * @param kdTree - the KD tree of the current images database
 * @param numOfImages - the number of images in the database
 * @param numOfSimilarImages - the number of similar images to present for each user query
 * @param bpq - a pre-allocated priority queue
 * @param GUIFlag - a flag that indicates if the program runs at minimal gui mode
 *
 */
void proccessQueryAndPresentImages(SPConfig config, SPImageData currentImageData, sp::ImageProc** imageProcObject,
		SPKDTreeNode kdTree, int numOfImages, int numOfSimilarImages, SPBPQueue bpq, char* workingImagePath, bool GUIFlag){
	int *similarImagesIndices = NULL, i;
	char tempPath[MAX_PATH_LEN];

	spLoggerSafePrintInfo(QUERY_HAS_BEEN_INSERTED);
	spLoggerSafePrintInfo(workingImagePath);

	currentImageData->featuresArray = (*imageProcObject)->getImageFeatures(workingImagePath,0,&(currentImageData->numOfFeatures));

	spValNc((similarImagesIndices = searchSimilarImages(currentImageData, kdTree, numOfImages,
			numOfSimilarImages, bpq)) != NULL , FAIL_SEARCHING_IMAGES, ); //on error returns

	if (GUIFlag) {
		spLoggerSafePrintDebug(DEBUG_IMAGES_PRESENTED_GUI,
					__FILE__, __FUNCTION__, __LINE__);
		for (i = 0; i< numOfSimilarImages; i++) {
			spValWarning(spConfigGetImagePath(tempPath, config, similarImagesIndices[i]) == SP_CONFIG_SUCCESS,
									WARNING_COULD_NOT_LOAD_IMAGE_PATH,
									printf(RELEVANT_IMAGE_INDEX_IS, similarImagesIndices[i]),
									(*imageProcObject)->showImage(tempPath););

		}
	} else {
		presentSimilarImagesNoGUI(workingImagePath, config, similarImagesIndices,
				numOfSimilarImages);
	}
	spLoggerSafePrintInfo(SIMILAR_IMAGES_PRESENTED);
	spFree(similarImagesIndices);
}

/*
 * The method is used for the user interaction process, it receives the relevant data that is loaded
 * and asks the user for image queries and handles them accordingly.
 *
 *
 * @param config - the configuration data
 * @param currentImageData - a pre-allocated image data to work with
 * @param kdTree - the KD tree of the current images database
 * @param numOfImages - the number of images in the database
 * @param numOfSimilarImages - the number of similar images to present for each user query
 * @param bpq - a pre-allocated priority queue
 * @param GUIFlag - a flag that indicates if the program runs at minimal gui mode
 * @param imageProbObject - a pointer to the image proc object pointer
 * @param oneImageWasSet - a pointer to a flag that indicates that one image query was loaded,
 * 							this is needed for memory deallocation.
 */
void spMainStartUserInteraction(SPConfig config,SPImageData currentImageData, SPKDTreeNode kdTree,int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq, bool GUIFlag, sp::ImageProc** imageProcObject, bool* isCurrentImageFeaturesArrayAllocated){
	char workingImagePath[MAX_PATH_LEN];


	// first run must always happen
	getQuery(workingImagePath);

	// iterating until the user inputs "<>"
	while (strcmp(workingImagePath, QUERY_EXIT_INPUT)) {
		resetImageData(currentImageData);
		*isCurrentImageFeaturesArrayAllocated = false; //indicate we should not free currentImageData->features again

		while (strcmp(workingImagePath, QUERY_EXIT_INPUT) && !verifyPathAndAvailableFile(workingImagePath)){
		spLoggerSafePrintInfo(ILLEGAL_QUERY_HAS_BEEN_INSERTED);
			spLoggerSafePrintInfo(workingImagePath);

			spLoggerSafePrintWarning(WRONG_USER_QUERY, __FILE__, __FUNCTION__, __LINE__);
			printf(REQUEST_QUERY_AGAIN);
			fflush(NULL);
			getQuery(workingImagePath);
		}
		if (!strcmp(workingImagePath, QUERY_EXIT_INPUT)){ // query == '<>'
			return;
		}

		*isCurrentImageFeaturesArrayAllocated = true;
		proccessQueryAndPresentImages(config, currentImageData, imageProcObject, kdTree, numOfImages,
				numOfSimilarImages, bpq, workingImagePath, GUIFlag);

		getQuery(workingImagePath);
	}
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
 * '-3' - in case of error at the logger
 * '0'  - success
 */

int main(int argc, char** argv) {
	int flowFlag;
	SPConfig config = NULL;
	int numOfSimilarImages, numOfImages = 0;
	SPImageData currentImageData = NULL;
	bool extractFlag, GUIFlag, isCurrentImageFeaturesArrayAllocated = false;
	SPKDTreeNode kdTree = NULL;
	SPBPQueue bpq = NULL;
	sp::ImageProc* imageProcObject = NULL;

	spMainAction(((flowFlag = spMainInitialize(argc, argv, &config, &numOfImages,
			&numOfSimilarImages, &extractFlag, &GUIFlag, &bpq,
			&currentImageData, &kdTree, &imageProcObject)) >= 0), flowFlag);

	spLoggerSafePrintInfo(INITIALIZATION_FINISHED_SUCCESSFUL);

	spMainStartUserInteraction(config,currentImageData, kdTree,numOfImages, numOfSimilarImages,
			bpq, GUIFlag, &imageProcObject, &isCurrentImageFeaturesArrayAllocated);

	spLoggerSafePrintInfo(USER_INTERACTION_FINISHED_SUCCESSFULLY);
	// end control flow
	spMainAction(RUN_ACTION, SUCCESS_RETURN_VALUE); //returns success
}
