#ifndef SPIMAGESPARSER_H_
#define SPIMAGESPARSER_H_

#include <stdbool.h>

#include "SPPoint.h"
#include "SPConfig.h"


//TODO - logger !!!!!!!!!!!!!!!!!!!!!!!!!!


/** A type used for defining an ImageData item**/
struct sp_image_data {
	int index;
	int numOfFeatures;
	SPPoint* featuresArray;
};

/** A type used for defining an ImageData item as a pointer**/
typedef struct sp_image_data* SPImageData;

/** A type used to indicate errors in function calls **/
typedef enum sp_data_parse_messages {
	SP_DP_SUCCESS,
	SP_DP_MEMORY_FAILURE,
	SP_DP_INVALID_ARGUMENT,
	SP_DP_FILE_READ_ERROR,
	SP_DP_FILE_WRITE_ERROR,
	SP_DP_FORMAT_ERROR,
	SP_DP_FEATURE_EXTRACTION_ERROR
} SP_DP_MESSAGES;


/*
 * The method sets the value of the features matrix
 *
 * @param features - the input features
 */
void setFeaturesMatrix(SPImageData* features);

/*
 * The method returns the number of characters required to store the point's data as a CSV
 *
 * @param point - the relevant point
 * @return - the requested size
 */
int getPointCSVSize(SPPoint point);

/*
 * The method gets an integer and returns its digits count.
 *
 * @param x - the requested number
 *
 * @return - x's digits count
 */
int getNumOfDigits(int x);

/*
 * The method gets a char array and a start position and try to extract a double value from
 * that string starting from "start", at the end "start" should be at the end of that double value
 *
 * @assert myString != NULL && *start >= 0
 *
 * @param myString - the relavent char array
 * @param start - a pointer to the start position at the string
 *
 * @returns - a double value of the relevant substring
 */
double getFloatingNumberFromSubString(char* myString, int* start);

/*
 * Creates a string signature at a CSV format of the given Point, with the following order:
 * "dim,point[0],...,point[dim-1]BREAKLINE"
 *
 * @param point - the relevant points
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 *  - point is NULL [message = SP_DP_INVALID_ARGUMENT],
 *  - memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 *  otherwise returns a CSV representing the point [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger regarding NULL pointer or memory allocation failure.
 */
char* pointToString(SPPoint point, SP_DP_MESSAGES* message);

/*
 * Loads a new SPPoint from a given CSV format - "dim,point[0],...,point[dim-1]BREAKLINE"
 *
 * @param pointdata - the relevant string representing the points data
 * @param index - the image index
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 * 	- pointdata is NULL  [message = SP_DP_INVALID_ARGUMENT],,
 * 	- memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 * 	- could not parse the point due to wrong format issues [message = SP_DP_FORMAT_ERROR],
 * 	otherwise returns a SPPoint representing the point  [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger.
 */
SPPoint parsePointFromString(char* pointdata, int index, SP_DP_MESSAGES* message);

/*
 * Creates a string signature at a CSV format of the given image basic details, with the following order:
 * "index,num of features[BREAKLINE]"
 *
 * @param point - the relevant image
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 *  - imageData is NULL [message = SP_DP_INVALID_ARGUMENT],
 *  - memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 *  otherwise returns a CSV representing the image basic details  [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger regarding NULL pointer or memory allocation failure.
 */
char* getImageStringHeader(SPImageData imageData, SP_DP_MESSAGES* message);

/*
 * The method returns an image path by its index
 *
 * @param config - the configurations data
 * @param imageIndex - the index of the requested image
 * @param dataPath - a flag used to request the data file path
 * @param message - a pointer to a message enum
 *
 * * @return -
 * NULL if :
 * 	- config is NULL [message = SP_DP_INVALID_ARGUMENT],
 * 	- memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 * 	otherwise returns the requested path [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger.
 */
char* getImagePath(const SPConfig config,int index,bool dataPath, SP_DP_MESSAGES* message);

/*
 * Loads to a given SPImageData the basic data from a given CSV format string - "index,num of features[BREAKLINE]"
 *
 * @param header - the relevant string representing the image header data
 * @param image - the relevant image data item
 *
 * @return -
 * SP_DP_INVALID_ARGUMENT - header or image are NULL
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FORMAT_ERROR - could not parse the image due to wrong format issues
 * SP_DP_SUCCESS - Successfully written data to the image item
 *
 * @logger - Prints relevant errors to the logger.
 */
SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image);

/*
 * The method loads the images data, and creates an ImageData array containing the features data.
 * in case of failure no data will be created,
 * and all the images data that has been created so far would be destroyed.
 * the createFlag flag indicates if the image data should be processed or not
 *
 * @param config - the configurations data
 * @param createFlag - true iff the image data should be created from the image file (processed).
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 * 	- config is NULL [message = SP_DP_INVALID_ARGUMENT],
 * 	- memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 * 	- could not parse the image due to wrong format issues [message = SP_DP_FORMAT_ERROR],
 * 	- could not read from file [message = SP_DP_FILE_READ_ERROR]
 *  - the features extraction failed [message = SP_DP_FEATURE_EXTRACTION_ERROR]
 * 	otherwise returns an array of SPImageData representing the images data [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger.
 */
SPImageData* loadAllImagesData(const SPConfig config, bool createFlag, SP_DP_MESSAGES* message);

/*
 * The method loads image data into an allocated SPImageData structure by an opened file
 *
 * @assert - imageFile != NULL && imageData!= NULL && imageData->featuresArray != NULL && imageData->numOfFeatures>0
 * @param imageFile - a pointer to the file that contains the data regarding the image (the file points to the second line)
 * @param imageData - an allocated image data item
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FORMAT_ERROR - the file is not in the correct format
 * SP_DP_SUCCESS - image data created successfully
 */
SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData);

/*
 * The method loads image data given the image path
 * into an allocated SPImageData structure -  processing the data
 *
 * @param imageDataPath - the file path
 * @param imageData - an allocated SPImageData item to which the data will be written
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FEATURE_EXTRACTION_ERROR - the features extraction failed
 * SP_DP_SUCCESS - image data created successfully
 */
SP_DP_MESSAGES createImageDataByPath(char* imagePath, SPImageData imageData);

/*
 * The method loads image data given the image path
 * into an allocated SPImageData structure (not processing the data again)
 *
 * @param imageDataPath - the file path
 * @param imageData - an allocated SPImageData item to which the data will be written
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FORMAT_ERROR - the file is not in the correct format
 * SP_DP_SUCCESS - image data created successfully
 */
SP_DP_MESSAGES loadKnownImageData(char* imageDataPath, SPImageData imageData);

/*
 * The method loads image data into an allocated SPImageData structure by an opened file (not processing the data again)
 *
 * @assert - imageFile and imageData != NULL
 * @param imageFile - a pointer to the file that contains the data regarding the image
 * @param imageData - an allocated SPImageData item to which the data will be written
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FORMAT_ERROR - the file is not in the correct format
 * SP_DP_SUCCESS - image data created successfully
 */
SP_DP_MESSAGES loadImageDataFromFile(FILE* imageFile, SPImageData imageData);

/*
 * The method loads a specific image and creates a SPImageData item containing its features data.
 * the createFlag flag indicates if the image data should be processed or not
 *
 * @param imageDataPath - the path of the requested image
 * @param imageIndex - the index of the requested image
 * @param createFlag - true iff the image data should be created from the image file (processed).
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 * 	- config is NULL [message = SP_DP_INVALID_ARGUMENT],
 * 	- imageIndex out of range [message = SP_DP_INVALID_ARGUMENT],
 * 	- memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 * 	- could not parse the image due to wrong format issues [message = SP_DP_FORMAT_ERROR],
 * 	- could not read from file [message = SP_DP_FILE_READ_ERROR]
 *  - the features extraction failed [message = SP_DP_FEATURE_EXTRACTION_ERROR]
 * 	otherwise returns a SPImageData representing the image data [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger.
 */
SPImageData loadImageDataByPath(char* imageDataPath,int imageIndex, bool createFlag, SP_DP_MESSAGES* message);

/*
 * The method loads a specific image and creates a SPImageData item containing its features data.
 * the createFlag flag indicates if the image data should be processed or not
 *
 * @param config - the configurations data
 * @param imageIndex - the index of the requested image
 * @param createFlag - true iff the image data should be created from the image file (processed).
 * @param message - a pointer to a message enum
 *
 * @return -
 * NULL if :
 * 	- config is NULL or index < 0 [message = SP_DP_INVALID_ARGUMENT],
 * 	- imageIndex out of range [message = SP_DP_INVALID_ARGUMENT],
 * 	- memory allocation failure [message = SP_DP_MEMORY_FAILURE],
 * 	- could not parse the image due to wrong format issues [message = SP_DP_FORMAT_ERROR],
 * 	- could not read from file [message = SP_DP_FILE_READ_ERROR]
 *  - the features extraction failed [message = SP_DP_FEATURE_EXTRACTION_ERROR]
 * 	otherwise returns a SPImageData representing the image data [message = SP_DP_SUCCESS]
 *
 * @logger - Prints relevant errors to the logger.
 */
SPImageData loadImageData(const SPConfig config, int imageIndex, bool createFlag, SP_DP_MESSAGES* message);

/*
 * The method gets an opened file pointer and an image data item and writes the image data to the file.
 * @assert - imageFile and imageData not NULL
 * @param imageFile - a pointer to the file that the data should be written to
 * @param imageData - an item that contains the image data
 *
 * @returns
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 */
SP_DP_MESSAGES writeImageDataToFile(FILE* imageFile, SPImageData imageData);

/*
 * The method saves to the disk an image data item at a CSV format.
 * The method will override an existing file with the same name.
 *
 * @param config - the configurations data
 * @param imageData - the image data to save
 *
 * @return -
 * SP_DP_INVALID_ARGUMENT - config or imageData is NULL,
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 *
 * @logger - Prints relevant errors to the logger.
 */
SP_DP_MESSAGES saveImageData(const SPConfig config, SPImageData imageData);

/*
 * The method saves to the disk a bulk of images data items at a CSV format.
 * The method will override existing files with the same name.
 * In case of failure the method will stop writing files, those who were writen would not be deleted
 *
 * @param config - the configurations data.
 * @param imagesData - the images data to save, as a SPImageData array.
 *
 * @return -
 * SP_DP_INVALID_ARGUMENT - config or imagesData or one of the items at imagesData is NULL,
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 *
 * @logger - Prints relevant errors to the logger.
 */
SP_DP_MESSAGES saveAllImagesData(const SPConfig config, SPImageData* imagesData);

/*
 * The main method that starts and loads all the images data according to the configurations
 *
 * @param config - the config file
 * @param msg - a message that represent the outcome of the process
 *
 * @returns :
 * NULL if:
 * 	    SP_DP_INVALID_ARGUMENT - config is NULL,
 * 		SP_DP_MEMORY_FAILURE - memory allocation failure
 * 		SP_DP_FILE_WRITE_ERROR - error writing to file
 *	    SP_DP_FILE_READ_ERROR - error reading from a file
 *	    SP_DP_FORMAT_ERROR - some file was in the wrong format
 *	    SP_DP_FEATURE_EXTRACTION_ERROR - could not extract features
 * SPImage data array of the images data and msg = SP_DP_SUCCESS if all actions done successfully
 *
 */
SPImageData* spImagesParserStartParsingProcess(const SPConfig config, SP_DP_MESSAGES* msg);

/*
 * Deallocates a number of features from a given features array
 *
 * @param features - the features array
 * @param numOfFeatures - the number of features
 *
 * @assert numOfFeatures >= 0
 *  @logger - prints a warning if images data is null
 */
void freeFeatures(SPPoint* features, int numOfFeatures);

/*
 * Deallocates an image data item
 *
 * @param imageData - the image data to destroy
 *
 * @logger - prints a warning if image data is null
 */
void freeImageData(SPImageData imageData);

/*
 * Deallocates an images data items array
 *
 * @param imagesData - the images data array that should be destryed
 * @param size - the size of the array
 *
 * @assert size >= 0
 *
 * @logger - prints a warning if images data is null
 */
void freeAllImagesData(SPImageData* imagesData, int size);

#endif /* SPIMAGESPARSER_H_ */
