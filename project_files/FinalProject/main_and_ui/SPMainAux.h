#ifndef SPMAINAUX_H_
#define SPMAINAUX_H_

#include "../SPConfig.h"
#include "../image_parsing/SPImagesParser.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"
#include "../general_utils/SPUtils.h"


/*
 * Extracts the configuration filename from the command line arguments of the program
 *
 * @param argc - the number of arguments the program received in the command line,
 * including the name of the program
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
 * The method prints exiting string (inform in case of error at program run)
 *  and free's all relevant allocated memory
 *
 * @param config - the config item to be freed
 * @param image - an image to be freed
 * @param oneImageWasSet - indicates that image->features is not NULL
 * @param kdTree - the KDTree item to be freed
 * @param bpq - the priority queue item to be freed
 * @param returnValue - an integer that indicates if the program finished its work
 * 						Successfully
 *
 */
void endControlFlow(SPConfig config, SPImageData image,
		bool oneImageWasSet, SPKDTreeNode kdTree, SPBPQueue bpq, int returnValue);

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
 * The method prints the result to the user in non-minimal GUI mode in the requested format
 *
 * @param queryImagePath - the path of the given query image
 * @param config - the configuration structure instance
 * @param imagesIndexesArray - the chosen indices that should be printed
 * @param imagesCount - the size of "imagesIndicesArray"
 */
void presentSimilarImagesNoGUI(char* queryImagePath, SPConfig config,
		int* imagesIndicesArray, int imagesCount);

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

/*
 * Initializes SPImageData addressed by given SPImageData pointer 'currentImageData',
 * creates KDTree according to the given SPImageData pointers list 'imagesDataList'
 * and initializes a priority queue with max size using given configuration structure
 * instance 'config'
 *
 * @param config - configuration structure instance
 * @param imagesDataList - list of SPImageData pointers according to which the function
 * creates the KDTree
 * @param currentImageData - pointer to address to initialize SPImageData in
 * @param kdTree - pointer to a SPKDTreeNode which will be the root of the KDTree to be
 * built in the function
 * @param bpq - pointer to SPBPQueue to be initialized in the function
 * @param numOfImages - the number of images in workingImagesDatabase (the size of the
 * SPImageData instances array)
 *
 * @returns false if failed in any stage during these all operations, otherwise returns
 * true
 */
bool initializeWorkingImageKDTreeAndBPQueue(const SPConfig config,
		SPImageData** imagesDataList, SPImageData* currentImageData, SPKDTreeNode* kdTree,
		SPBPQueue* bpq, int numOfImages);

/*
 * Initializes the configuration structure instance pointed by 'config' according to 'argc'
 * and 'argv', and the values of the settings pointed by: 'numOfImages',
 * 'numOfSimilarImages', 'extractFlag' and 'GUIFlag'
 *
 * @param argc - the number of arguments the program received in the command line,
 * including the name of the program
 * @param argv - an array containing all the arguments the program received in the command
 * line
 * @param config - pointer to a configuration structure instance to be initialized in the
 * function
 * @param numOfImages - pointer to an integer to contain the number of images that was set
 * in the configuration file
 * @param numOfSimilarImages - pointer to an integer to contain the number of similar
 * images that was set in the configuration file
 * @param extractFlag - pointer to a boolean to contain the value of spExtractionFlag that
 * was set in the configuration file
 * @param GUIFlag - pointer to a boolean to contain the value of spMinimalGUI that was set
 * in the configuration file
 *
 * @returns false if failed in any stage of the operation, otherwise returns true
 */
bool initConfigAndSettings(int argc, char** argv, SPConfig* config, int* numOfImages,
		int* numOfSimilarImages, bool* extractFlag, bool* GUIFlag);

/*
 * Verifies that the PCA file path and images files paths extracted from 'config' are valid
 * and available
 *
 * @param config - the given configuration structure instance
 * @param numOfImages - the number of images that was set in the configuration file
 * @param extractFlag - a boolean containing the value of spExtractionFlag that was set in
 * the configuration file
 *
 * @returns false if PCA file path or any of the images files paths are not valid or
 * available, otherwise returns true
 */
bool verifyImagesFiles(SPConfig config, int numOfImages, bool extractFlag);

#endif /* SPMAINAUX_H_ */

