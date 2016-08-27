#ifndef SPMAINAUX_H_
#define SPMAINAUX_H_

#include "../SPConfig.h"
#include "../image_parsing/SPImagesParser.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"

//TODO - validate and comment
#define verifyAction(action, errorMessage) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					endControlFlow(config, currentImageData, imagesDataList, numOfImages, \
									oneImageWasSet, kdTree, bpq); \
					return -1; \
                } \
        } while (0)

/*
 * Extracts the configuration filename from the command line arguments of the program
 *
 * @param argc - the number of arguments the program received in the command line, including
 * the name of the program
 * @param argv - an array containing all the arguments the program received in the command
 * line
 * @return the configuration filename extracted from the command line arguments if they
 * were given in a valid way, NULL otherwise.
 */
char* getConfigFilename(int argc, char** argv);

/*
 * Builds a configuration structure instance based on the given configuration filename
 * and returns it
 *
 * @param configFilename - the filename of the given configuration file
 * @param msg - pointer in which the msg returned by the function is stored
 * @return a configuration structure instance based on the given configuration filename.
 */
SPConfig getConfigFromFile(const char* configFilename, SP_CONFIG_MSG* msg);

/*
 * The method prints exiting string and free's all relevant allocated memory
 *
 * @param config - the config item to be freed
 * @param image - an image to be freed
 * @param imagesList - images array to be freed
 * @param numOfImages - number of images to free at images list
 * @param oneImageWasSet - indicates that image->features is not NULL
 * @param kdTree - the KDTree item to be freed
 * @param bpq - the priority queue item to be freed
 *
 */
void endControlFlow(SPConfig config, SPImageData image, SPImageData* imagesList,
		int numOfImages, bool oneImageWasSet, SPKDTreeNode kdTree, SPBPQueue bpq);

/*
 * The method prints a message to the console and gets an input from the user
 * The method inserts the input into "destination" that is presumed to be allocated
 *
 * @param message - the message to show to the user
 * @param destination - an allocated string that the input will be inserted to
 */
void getAsString(const char* message, char* destination);

/*
 * The method request a query from the user and gets an input from the user
 * The method inserts the input into "destination" that is presumed to be allocated
 *
 * @param destination - an allocated string that the input will be inserted to
 */
void getQuery(char* destination);

/*
 * The method search the database for similar images and returns an array of integers
 * representing the closest images found to the query image
 *
 * @param workingImage - image item to query
 * @param kdTree - a KDTree instance representing the KDTree created from all the features
 * of all the images whose paths were given in the configuration file
 * @param numOfImages - the total number of images in the database
 * @param numOfSimilarImages - the size of the returned array
 * @param bpq - a priority queue used to store the nearest features to each feature of the
 * working image
 *
 * @returns
 * NULL on memory allocation error, or error in an internal function
 * otherwise returns an integer array with "numOfSimilarImages" size that contains the
 * indices of the matched images
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* searchSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq);

/*
 * The method gets indexes list and prints them to the user
 *
 * @param imagesIndexesArray - the chosen indexes that should be printed
 * @param imagesCount - the size of "imagesIndexesArray"
 */
void presentSimilarImagesNoGUI(int* imagesIndexesArray, int imagesCount);

/*
 * The method load some settings from the config item into given pointers.
 *
 * @param config - the configurations item
 * @param numOfImages - pointer to an integer representing the count if images
 * @param numOfSimilar - pointer to an integer representing the count if similar images
 * @param extractFlag - pointer to a boolean representing the extraction flag
 * @param GUIFlag - pointer to a boolean representing the GUI flag
 *
 * @returns
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG loadRelevantSettingsData(const SPConfig config, int* numOfImages,
		int* numOfSimilar, bool* extractFlag, bool* GUIFlag);

/*
 * The method gets a pointer to images data array and initialize it, allocated memory and indexes values.
 *
 * @param imagesDataList - pointer to images data array
 * @param numOfImages - the count of images
 *
 * in case of error, it would be logged and *imagesDataList will be NULL
 */
void initializeImagesDataList(SPImageData** imagesDataList, int numOfImages);

/*
 * The method initialize an image data item.
 *
 * @return - NULL if memory allocation error, and logs the error.
 * otherwise returns a new image data item initialized with default values for the index.
 */
SPImageData initializeWorkingImage();

/*
 * The method gets a path to a file and returns true iff its a correct path to an available file
 *
 * @param path - the path to the file
 *
 * @returns true iff 'path' is a correct path to an available file
 */
bool verifyPathAndAvailableFile(char* path);

/*
 * Calculates the sum of all features of all images in 'workingImagesDatabase' SPImageData
 * instances array and returns it
 *
 * @param workingImagesDatabase - an array of SPImageData instances
 * @param numOfImages - the number of images in workingImagesDatabase (the size of the
 * array)
 *
 * @returns the sum of all features of all images in 'workingImagesDatabase'
 */
int calculateTotalNumOfFeatures(SPImageData* workingImagesDatabase, int numOfImages);

/*
 * Creates and fills an SPPoint array of all the features of all the images in
 * 'workingImagesDatabase' SPImageData instances array
 *
 * @param workingImagesDatabase - an array of SPImageData instances
 * @param numOfImages - the number of images in workingImagesDatabase (the size of the
 * SPImageData instances array)
 * @param totalNumOfFeatures - the size of the return array
 *
 * @returns an SPPoint array of all the features of all the images in
 * 'workingImagesDatabase'
 */
SPPoint* initializeAllFeaturesArray(SPImageData* workingImagesDatabase, int numOfImages,
		int totalNumOfFeatures);

// TODO - doc
bool initializeKDTreeAndBPQueue(const SPConfig config, SPImageData** imagesDataList,
		SPImageData* currentImageData, SPKDTreeNode* kdTree, SPBPQueue* bpq,
		int numOfImages);

bool initConfigAndSettings(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag);

//TODO - doc
bool verifyImagesFiles(SPConfig config, int numOfImages, bool extractFlag);

#endif /* SPMAINAUX_H_ */

