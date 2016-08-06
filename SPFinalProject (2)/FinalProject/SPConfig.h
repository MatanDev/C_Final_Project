#ifndef SPCONFIG_H_
#define SPCONFIG_H_

#include <stdbool.h>
#include <stdio.h>
#include "SPLogger.h"

/*
 * An enum for the messages that can be returned
 * from operations regarding the system configuration
 */
typedef enum sp_config_msg_t {
	SP_CONFIG_MISSING_DIR,
	SP_CONFIG_MISSING_PREFIX,
	SP_CONFIG_MISSING_SUFFIX,
	SP_CONFIG_MISSING_NUM_IMAGES,
	SP_CONFIG_CANNOT_OPEN_FILE,
	SP_CONFIG_ALLOC_FAIL,
	SP_CONFIG_INVALID_INTEGER,
	SP_CONFIG_INVALID_STRING,
	SP_CONFIG_INVALID_ARGUMENT,
	SP_CONFIG_INDEX_OUT_OF_RANGE,
	SP_CONFIG_SUCCESS
} SP_CONFIG_MSG;

/*
 * An enum for the three types of error messages to be printed to the user
 */
typedef enum error_message_type_t {
	INVALID_CONF_FILE,
	INVALID_VALUE,
	PARAMETER_NOT_SET
} ERROR_MSG_TYPE;

/*
 * An enum for the three available KDTree split methods
 */
typedef enum sp_kdtree_split_method_t {
	RANDOM,
	MAX_SPREAD,
	INCREMENTAL
} SP_KDTREE_SPLIT_METHOD;

struct sp_config_t {
	char* spImagesDirectory;
	char* spImagesPrefix;
	char* spImagesSuffix;
	int spNumOfImages;
	int spPCADimension;
	char* spPCAFilename;
	int spNumOfFeatures;
	bool spExtractionMode;
	int spNumOfSimilarImages;
	SP_KDTREE_SPLIT_METHOD spKDTreeSplitMethod;
	int spKNN;
	bool spMinimalGUI;
	int spLoggerLevel;
	char* spLoggerFilename;
};

typedef struct sp_config_t* SPConfig;

/*
 * Returns true if copying def to *field was successful, false otherwise.
 *
 * @param field - pointer to the string field to copy the default value into
 * @param def - the default string to initialize *field with
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if *field != NULL after copying, false otherwise.
 *
 * - SP_CONFIG_ALLOC_FAIL - if *field allocation failed
 */
bool checkAndSetDefIfNeeded(char** field, const char* def, SP_CONFIG_MSG* msg);

/*
 * Initializes given configuration structure instance's fields to default values
 *
 * @param config - pointer to the configuration structure instance
 */
void initConfigToDefault(SPConfig config);

/*
 * Prints a formatted error message with respect to given parameter errorMsgType
 *
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param errorMsgType - the type of error which needs to be printed
 * @param parameterName - the name of the parameter which is not set in case of
 * parameter not set error
 */
void printErrorMessage(const char* filename, int lineNum,
		ERROR_MSG_TYPE errorMsgType, const char* parameterName);

/*
 * Checks if the given configuration line is valid
 * and extracts the variable name and its value if so
 *
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param line - a string representing the given line from the configuration file
 * @param varName - pointer to a string to store the variable name in
 * @param value - pointer to a string to store the value in
 * @param isCommentOrEmpty - parameter to a boolean which is set to true iff the given
 * line is a comment line or an empty line
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if the given line is valid, otherwise returns false
 */
bool parseLine(const char* filename, int lineNum, char* line,
		char** varName, char** value, bool* isCommentOrEmpty,
		SP_CONFIG_MSG* msg);

/*
 * Checks if the given value upholds all the constraints of the given string field
 * and if so sets the given string field value to the given value
 *
 * @param strField - pointer to string field to set the value to
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @param isImagesSuffix - a boolean indicating wether the given string field is
 * spImagesSuffix, since it includes another constraint
 * @return true if the given value upholds all the constraints of the given string field
 * and no error occurred in the process of setting it, otherwise returns false
 */
bool handleStringField(char** strField, const char* filename, int lineNum,
		const char* value, SP_CONFIG_MSG* msg, bool isImagesSuffix);

/*
 * Checks if the given value is a positive integer and if so sets the given integer field
 * value to the given value
 *
 * @param posIntField - pointer to integer field to set the value to
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if the given value is a positive integer, otherwise returns false
 */
bool handlePositiveIntField(int* posIntField, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg);

/*
 * Checks if the given value is a positive integer in the given domain
 * and if so sets the given integer field value to the given value
 *
 * @param posIntField - pointer to integer field to set the value to
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @param minVal - the minimum legal value that value can have
 * @param maxVal - the maximum legal value that value can have
 * @return true if the given value is a positive integer in the given domain,
 * otherwise returns false
 */
bool handleBoundedPosIntField(int* posIntField, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg, int minVal, int maxVal);

/*
 * Checks if the given value is "true" or "false"
 * and if so sets the given boolean field value to true or false respectively
 *
 * @param boolField - pointer to boolean field to set the value to
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if the given value is "true" or "false",
 * otherwise returns false
 */
bool handleBoolField(bool* boolField, const char* filename, int lineNum,
		char* value, SP_CONFIG_MSG* msg);

/*
 * Checks if the given value is a KDTree split method
 * and if so sets config->spKDTreeSplitMethod value to the given KDTree split method
 *
 * @param config - pointer to the configuration structure instance
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if the given value is a KDTree split method, otherwise returns false
 */
bool handleKDTreeSplitMethod(SPConfig config, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg);

/*
 * Checks if the given variable name is a name of a field in config and if so sets the
 * field's value to the given value, in case the value upholds all the constraints regarding
 * the field
 *
 * @param config - pointer to the configuration structure instance
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * configuration file in case of parameter not set error
 * @param varName - a string which contains the name of the variable to set the value to
 * @param value - a string which contains the value to set
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if the given value was set successfully to the given variable
 */
bool handleVariable(SPConfig config, const char* filename, int lineNum,
		char *varName, char *value, SP_CONFIG_MSG* msg);

/*
 * Destroys the given configuration structure instance and returns NULL
 *
 * @param config - pointer to the configuration structure instance
 * @return NULL
 */
SPConfig onError(SPConfig config);

/*
 * Check if the parameters that don't have a default value were set in the configuration file
 *
 * @param config - pointer to the configuration structure instance
 * @param msg - pointer in which the msg returned by the function is stored
 * @param filename - the configuration filename
 * @param lineNum - the number of the invalid line or the number of lines in the
 * @return config if all parameters that don't have a default value were set in the
 * configuration file, otherwise return NULL
 */
SPConfig parameterSetCheck(SPConfig config, SP_CONFIG_MSG* msg,
		const char* filename, int lineNum);

/*
 * Returns true if both config is not null, otherwise false
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if both config is not null, otherwise false
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool isValid(const SPConfig config, SP_CONFIG_MSG* msg);

/**
 * Creates a new system configuration struct. The configuration struct
 * is initialized based on the configuration file given by 'filename'.
 * 
 * @param filename - the name of the configuration file
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return NULL in case an error occurs. Otherwise, a pointer to a struct which
 * 		   contains all system configuration.
 * 
 * The resulting value stored in msg is as follow:
 * - SP_CONFIG_INVALID_ARGUMENT - if filename == NULL
 * - SP_CONFIG_CANNOT_OPEN_FILE - if the configuration file given by filename cannot be open
 * - SP_CONFIG_ALLOC_FAIL - if an allocation failure occurred
 * - SP_CONFIG_INVALID_INTEGER - if a line in the config file contains invalid integer
 * - SP_CONFIG_INVALID_STRING - if a line in the config file contains invalid string
 * - SP_CONFIG_MISSING_DIR - if spImagesDirectory is missing
 * - SP_CONFIG_MISSING_PREFIX - if spImagesPrefix is missing
 * - SP_CONFIG_MISSING_SUFFIX - if spImagesSuffix is missing 
 * - SP_CONFIG_MISSING_NUM_IMAGES - if spNumOfImages is missing
 * - SP_CONFIG_SUCCESS - in case of success
 *
 *
 */
SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg);

/*
 * Returns true if spExtractionMode = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns true if spMinimalGUI = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the number of images set in the configuration file, i.e the value
 * of spNumOfImages.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the number of features to be extracted. i.e the value
 * of spNumOfFeatures.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg);

/**
 * Returns the dimension of the PCA. i.e the value of spPCADimension.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the number of similar images set in the configuration file, i.e the value
 * of spNumOfSimilarImages.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the logger level as configured in the configuration file,
 * i.e the SP_LOGGER_LEVEL represented by the value of spLoggerLevel.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return the SP_LOGGER_LEVEL represented by the value of spLoggerLevel in success,
 * SP_LOGGER_INFO_WARNING_ERROR_LEVEL otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_LOGGER_LEVEL spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the logger filename as configured in the configuration file
 * i.e the value of spLoggerFilename.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return spLoggerFilename in success, NULL otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
char* spConfigGetLoggerFilename(const SPConfig config, SP_CONFIG_MSG* msg);

/**
 * Given an index 'index' the function stores in imagePath the full path of the
 * ith image if 'isFeats' is false, and the full path of the ith image with ".feats"
 * extension if 'isFeats' is true.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @param index - the index of the image.
 * @param isFeats - the boolean used to determine whether to return the full path of the
 * image with ".feats" extension or not
 *
 * @return
 * - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 * - SP_CONFIG_INDEX_OUT_OF_RANGE - if index >= spNumOfImages
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetImagePathFeats(char* imagePath, const SPConfig config,
		int index, bool isFeats);

/**
 * Given an index 'index' the function stores in imagePath the full path of the
 * ith image.
 *
 * For example:
 * Given that the value of:
 *  spImagesDirectory = "./images/"
 *  spImagesPrefix = "img"
 *  spImagesSuffix = ".png"
 *  spNumOfImages = 17
 *  index = 10
 *
 * The functions stores "./images/img10.png" to the address given by imagePath.
 * Thus the address given by imagePath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @param index - the index of the image.
 *
 * @return
 * - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 * - SP_CONFIG_INDEX_OUT_OF_RANGE - if index >= spNumOfImages
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
		int index);

/**
 * The function stores in pcaPath the full path of the pca file.
 * For example given the values of:
 *  spImagesDirectory = "./images/"
 *  spPcaFilename = "pca.yml"
 *
 * The functions stores "./images/pca.yml" to the address given by pcaPath.
 * Thus the address given by pcaPath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @return
 *  - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 *  - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config);

/**
 * Frees the memory of *field and sets it to null in the end.
 * If *field == NULL nothing is done.
 */
void freeAndSetToNull(char** field);

/**
 * Frees all memory resources associate with config.
 * If config == NULL nothing is done.
 */
void spConfigDestroy(SPConfig config);

/**
 * Returns the given SP_CONFIG_MSG instance as string
 *
 * @param msg - the SP_CONFIG_MSG instance
 * @return the given SP_CONFIG_MSG instance as string
 */
char* configMsgToStr(SP_CONFIG_MSG msg);

#endif /* SPCONFIG_H_ */
