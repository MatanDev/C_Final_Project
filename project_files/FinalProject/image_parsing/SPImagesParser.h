#ifndef SPIMAGESPARSER_H_
#define SPIMAGESPARSER_H_

#include <stdbool.h>

#include "../SPPoint.h"
#include "../SPConfig.h"
#include "SPImageData.h"


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
 * The method gets a pre-allocated char array and returns
 * true if it represents a line i.e ends with '\n'
 *
 * @param line - the char array
 *
 * @returns true iff line[line length -1] == '\n'
 */
bool isAFullLine(char* line);

/*
 * A helper method used to free the allocated data for 2 char arrays,
 * if they do not point to NULL
 *
 * @param s1 - first item
 * @param s2 - second item
 */
void onGetLineError(char* s1, char* s2);

/*
 * A method that is used to get a line (at an unknown length)
 * from a pre-allocated file pointer and a minimum estimation for the line length
 * the file pointer will point to the beginning of the next line after the
 * method is invoked
 *
 * pre assumptions - fp != NULL and min_buffer_size > 0 and min_buffer_size must be even
 *
 * @param fp - a pointer to a pre-allocated file at read mode
 * @param min_buffer_size - a positive integer used for the
 * 								minimum guess of the line size
 *
 * @returns :
 * NULL in case of memory allocation error
 * otherwise returns a char array that represent the current line in the file
 * an empty string will be returned if the fp already pointer to eof
 *
 * @logger - the method logs a memory allocation error if needed
 */
char* getLineByMinBufferSize(FILE* fp, int min_buffer_size);

/*
 * A method that is used to get a line (at an unknown length)
 * from a pre-allocated file pointer
 * the file pointer will point to the beginning of the next line after the
 * method is invoked
 *
 * pre assumptions - fp != NULL && minBufferSize > 0 && minBufferSize % 2 == 0
 *
 * @param fp - a pointer to a pre-allocated file at read mode
 *
 * @returns :
 * NULL in case of memory allocation error
 * otherwise returns a char array that represent the current line in the file
 * an empty string will be returned if the fp already pointer to eof
 *
 * @logger - the method logs a memory allocation error if needed
 */
char* getLine(FILE* fp);


/*
 * The method returns the number of characters required to store the point's data as a CSV
 *
 * @param point - the relevant point
 * @return - the requested size
 *
 * @logger - the method reports odd internal calculations as warnings
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
 * pre assumptions - myString != NULL && *start >= 0
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
 * @logger - Prints relevant errors and warnings to the logger regarding NULL pointer or memory allocation failure.
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
 * @logger - Prints relevant errors and warnings to the logger.
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
 * @logger - Prints relevant errors and warnings to the logger regarding NULL pointer or memory allocation failure.
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
 * @logger - Prints relevant information to the logger.
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
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image);

/*
 * The method loads the images data into allImagesData, thus fills the ImageData array containing the features data.
 * in case of failure no data will be created,
 * and all the images data that has been created so far would be destroyed.
 *
 * @param config - the configurations data
 * @param configSignature - a string signature of the config file
 * @param allImagesData - a pre allocated images data array
 *
 * @return -
 *
 * 	- SP_DP_INVALID_ARGUMENT - config or configSignature or allImagesData is NULL
 * 	- SP_DP_MEMORY_FAILURE - memory allocation failure
 * 	- SP_DP_FORMAT_ERROR - could not parse the image due to wrong format issues
 * 	- SP_DP_FILE_READ_ERROR - could not read from file
 *  - SP_DP_FEATURE_EXTRACTION_ERROR - the features extraction failed
 * 	- SP_DP_SUCCESS - otherwise fully allocates allImagesData with the relevant data from the images
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES loadAllImagesData(const SPConfig config, char* configSignature, SPImageData* allImagesData);

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
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData);

/*
 * The method loads image data given the image path
 * into an allocated SPImageData structure (not processing the data again)
 *
 * @param configSignature - a string representing the config file relevant settings
 * @param imageDataPath - the file path
 * @param imageData - an allocated SPImageData item to which the data will be written
 *
 * pre assumptions - configSignature != NULL
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FORMAT_ERROR - the file is not in the correct format
 * SP_DP_SUCCESS - image data created successfully
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES loadKnownImageData(char* configSignature, char* imageDataPath, SPImageData imageData);

/*
 * The method loads image data into an allocated SPImageData structure by an opened file (not processing the data again)
 *
 * pre assumptions - imageFile and imageData and configSignature != NULL
 *
 * @param configSignature - a string representing the config file relevant settings
 * @param imageFile - a pointer to the file that contains the data regarding the image
 * @param imageData - an allocated SPImageData item to which the data will be written
 *
 * @return -
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_READ_ERROR - error reading from file
 * SP_DP_FORMAT_ERROR - the file is not in the correct format
 * SP_DP_SUCCESS - image data created successfully
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES loadImageDataFromFile(char* configSignature, FILE* imageFile, SPImageData imageData);


/*
 * The method loads a specific image and fills its SPImageData item containing its features data at allImagesData.
 *
 * @param config - the configurations data
 * @param configSignature - a string representing the config file relevant settings
 * @param imageIndex - the index of the requested image
 * @param allImagesData - a pointer to pre allocated images database
 *
 * @return -
 * 	- SP_DP_INVALID_ARGUMENT - config is NULL or index < 0 or configSignature or allImagesData is NULL
 * 	- SP_DP_INVALID_ARGUMENT - imageIndex out of range
 * 	- SP_DP_MEMORY_FAILURE - memory allocation failure
 * 	- SP_DP_FORMAT_ERROR - could not parse the image due to wrong format issues
 * 	- SP_DP_FILE_READ_ERROR - could not read from file
 *  - SP_DP_FEATURE_EXTRACTION_ERROR - the features extraction failed
 * 	- SP_DP_SUCCESS - otherwise fills the relevant image data
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES loadImageData(const SPConfig config, char* configSignature, int imageIndex, SPImageData* allImagesData);

/*
 * The method gets an opened file pointer and an image data item and writes the image data to the file.
 *
 * @param imageFile - a pointer to the file that the data should be written to
 * @param imageData - an item that contains the image data
 * @param configSignature - a string representing a signature of the config file
 *
 * pre assumptions  - imageFile and imageData and configSignature not NULL
 *
 * @returns
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES writeImageDataToFile(FILE* imageFile, SPImageData imageData, char* configSignature);

/*
 * The method saves to the disk an image data item at a CSV format.
 * The method will override an existing file with the same name.
 *
 * @param config - the configurations data
 * @param configSignature - a string representing a signature of the config file
 * @param imageData - the image data to save
 *
 * @return -
 * SP_DP_INVALID_ARGUMENT - config or imageData is NULL,
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES saveImageData(const SPConfig config,char* configSignature, SPImageData imageData);

/*
 * The method saves to the disk a bulk of images data items at a CSV format.
 * The method will override existing files with the same name.
 * The method will consider a success if it can write more than a given percentage of .feats file,
 * this percentage is defined with the macro 'MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS',
 * if more than MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS % of the images fails, the method will announce
 * a failure and return the last failure message reason
 *
 * In case of failure the method will stop writing files, those who were written would not be deleted
 *
 *
 * @param config - the configurations data.
 * @param configSignature - a string representing a signature of the config file
 * @param imagesData - the images data to save, as a SPImageData array.
 *
 * @return -
 * SP_DP_INVALID_ARGUMENT - config or configSignature or imagesData or one of the items at imagesData is NULL,
 * SP_DP_MEMORY_FAILURE - memory allocation failure
 * SP_DP_FILE_WRITE_ERROR - error writing to file
 * SP_DP_SUCCESS - file created and saved successfully
 *
 * @logger - Prints relevant errors to the logger, and warning in case of write failures that
 * 				did not reached the MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS limit.
 */
SP_DP_MESSAGES saveAllImagesData(const SPConfig config, char* configSignature, SPImageData* imagesData);

/*
 * The main method that starts and loads all the images data according to the configurations
 *
 * @param config - the config file
 * @imagesData - a list of images data, the method will fill all the relevant data into it
 *
 * @returns :
 * 	    SP_DP_INVALID_ARGUMENT - config is NULL,
 * 		SP_DP_MEMORY_FAILURE - memory allocation failure
 * 		SP_DP_FILE_WRITE_ERROR - error writing to file
 *	    SP_DP_FILE_READ_ERROR - error reading from a file
 *	    SP_DP_FORMAT_ERROR - some file was in the wrong format
 *	    SP_DP_FEATURE_EXTRACTION_ERROR - could not extract features
 *	    SP_DP_SUCCESS - if all actions done successfully
 *
 * @logger - Prints relevant errors and warnings to the logger.
 */
SP_DP_MESSAGES spImagesParserStartParsingProcess(const SPConfig config, SPImageData* imagesData);


#endif /* SPIMAGESPARSER_H_ */
