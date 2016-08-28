#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "SPConfig.h"
#include "general_utils\SPUtils.h"

#define DEFAULT_PCA_DIMENSION	20
#define DEFAULT_PCA_FILENAME	"pca.yml"
#define DEFAULT_NUM_OF_FEATURES	100
#define DEFAULT_NUM_OF_SIM_IMGS	1
#define DEFAULT_KNN				1
#define DEFAULT_LOGGER_LEVEL	SP_LOGGER_INFO_WARNING_ERROR_LEVEL
#define DEFAULT_LOGGER_FILENAME	"stdout"
#define FILE_TEMPLATE_FOR_ERROR	"File: %s\n"
#define LINE_TEMPLATE_FOR_ERROR	"Line: %d\n"
#define INVALID_CONF_MSG		"Message: Invalid configuration line\n"
#define INVALID_VALUE_MSG		"Message: Invalid value - constraint not met\n"
#define PARAM_NOT_SET_MSG		"Message: Parameter %s is not set\n"
#define COMMENT_SPECIFIER		'#'
#define ASSIGNMENT_SPECIFIER	'='
#define NULL_CHARACTER			'\0'
#define JPG_FILE_EXTENSION		".jpg"
#define PNG_FILE_EXTENSION		".png"
#define BMP_FILE_EXTENSION		".bmp"
#define GIF_FILE_EXTENSION		".gif"
#define FEATS_FILE_EXTENSION	".feats"
#define TRUE_AS_STR				"true"
#define FALSE_AS_STR			"false"
#define RAND_SPLIT_METHOD		"RANDOM"
#define MAX_SPREAD_SPLIT_METHOD	"MAX_SPREAD"
#define INC_SPLIT_METHOD		"INCREMENTAL"
#define SP_IMAGES_DIRECTORY		"spImagesDirectory"
#define SP_IMAGES_PREFIX		"spImagesPrefix"
#define SP_IMAGES_SUFFIX		"spImagesSuffix"
#define SP_NUM_OF_IMAGES		"spNumOfImages"
#define SP_PCA_DIMENSION		"spPCADimension"
#define SP_PCA_FILENAME			"spPCAFilename"
#define SP_NUM_OF_FEATURES		"spNumOfFeatures"
#define SP_EXTRACION_MODE		"spExtractionMode"
#define SP_NUM_OF_SIM_IMAGES	"spNumOfSimilarImages"
#define SP_KDTREE_SPLIT_MTD		"spKDTreeSplitMethod"
#define SP_KNN					"spKNN"
#define SP_MINIMAL_GUI			"spMinimalGUI"
#define SP_LOGGER_LVL			"spLoggerLevel"
#define SP_LOGGER_FILENAME		"spLoggerFilename"
#define MAX_LINE_LENGTH			1025 // 1024 from project specs + 1 for '\0'
//#define OPEN_FILE_READ_MODE		"r"
#define IMAGE_PATH_FORMAT		"%s%s%d%s"
#define PCA_PATH_FORMAT			"%s%s"
#define MISSING_DIR_MSG			"SP_CONFIG_MISSING_DIR"
#define MISSING_PREFIX_MSG		"SP_CONFIG_MISSING_PREFIX"
#define MISSING_SUFFIX_MSG		"SP_CONFIG_MISSING_SUFFIX"
#define MISSING_IMAGES_NUM_MSG	"SP_CONFIG_MISSING_NUM_IMAGES"
#define CANNOT_OPEN_FILE_MSG	"SP_CONFIG_CANNOT_OPEN_FILE"
#define ALLOCATION_FAILED_MSG	"SP_CONFIG_ALLOC_FAIL"
#define INVALID_INT_MSG			"SP_CONFIG_INVALID_INTEGER"
#define INVALID_STR_MSG			"SP_CONFIG_INVALID_STRING"
#define INVALID_ARG_MSG			"SP_CONFIG_INVALID_ARGUMENT"
#define INDEX_OUT_OF_RANGE_MSG	"SP_CONFIG_INDEX_OUT_OF_RANGE"
#define SUCCESS_MSG				"SP_CONFIG_SUCCESS"
#define SIGNATURE_FORMAT		"==[%s][%d][%d][%d]==\n"
#define ERROR_CREATING_SIGN     "Error creating config signature"
#define ERROR_INVALID_CONF_ARG	"The given configuration instance is not valid"
#define PCA_DIM_MIN_VALID_VAL	10
#define PCA_DIM_MAX_VALID_VAL	28
#define LOG_LVL_MIN_VALID_VAL	1
#define LOG_LVL_MAX_VALID_VAL	4

#define VALIDATE_INT(condition)	do { \
                if(!isValidInt(value, &tmpInt) || (condition)) { \
					*msg = SP_CONFIG_INVALID_INTEGER; \
					printErrorMessage(filename, lineNum, INVALID_VALUE, NULL); \
					return false; \
                } \
        } while (0)

#define VALID_MSG_IN_SIGN		do { \
                if (msg != SP_CONFIG_SUCCESS) { \
					spLoggerPrintError(ERROR_CREATING_SIGN, __FILE__, __FUNCTION__, \
							__LINE__); \
					return NULL; \
				} \
        } while (0)

/**
 * A data-structure which is used for configuring the system.
 */
/*struct sp_config_t {
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
	SP_LOGGER_LEVEL spLoggerLevel;
	char* spLoggerFilename;
};*/

bool checkAndSetDefIfNeeded(char** field, const char* def, SP_CONFIG_MSG* msg) {
	if (*field == NULL) {
		if ((*field = duplicateString(def)) == NULL) {
			*msg = SP_CONFIG_ALLOC_FAIL;
			return false;
		}
	}
	return true;
}

void initConfigToDefault(SPConfig config) {
	config->spImagesDirectory = NULL;
	config->spImagesPrefix = NULL;
	config->spImagesSuffix = NULL;
	config->spNumOfImages = 0;
	config->spPCADimension = DEFAULT_PCA_DIMENSION;
	config->spPCAFilename = NULL;
	config->spNumOfFeatures = DEFAULT_NUM_OF_FEATURES;
	config->spExtractionMode = true;
	config->spNumOfSimilarImages = DEFAULT_NUM_OF_SIM_IMGS;
	config->spKDTreeSplitMethod = MAX_SPREAD;
	config->spKNN = DEFAULT_KNN;
	config->spMinimalGUI = false;
	config->spLoggerLevel = DEFAULT_LOGGER_LEVEL;
	config->spLoggerFilename = NULL;
}

void printErrorMessage(const char* filename, int lineNum,
		ERROR_MSG_TYPE errorMsgType, const char* parameterName) {
	printf(FILE_TEMPLATE_FOR_ERROR, filename);
	printf(LINE_TEMPLATE_FOR_ERROR, lineNum);
	switch(errorMsgType) {
	case INVALID_CONF_FILE:
		printf(INVALID_CONF_MSG);
		break;
	case INVALID_VALUE:
		printf(INVALID_VALUE_MSG);
		break;
	case PARAMETER_NOT_SET:
		printf(PARAM_NOT_SET_MSG, parameterName);
	}
}

//TODO - forum: what should msg be in case of error?
bool parseLine(const char* filename, int lineNum, char* line,
		char** varName, char** value, bool* isCommentOrEmpty,
		SP_CONFIG_MSG* msg) {
	unsigned int startIndex, i;
	char *tmpPtr;

	// run while space
	for (startIndex = 0; startIndex < strlen(line) &&
	isspace(line[startIndex]); startIndex++);

	// first non-space character is '#' - comment line,
	// or line includes only spaces - empty line
	if (line[startIndex] == COMMENT_SPECIFIER || startIndex == strlen(line))
		return *isCommentOrEmpty = true;

	// the second case can happen only in the last line
	if((tmpPtr = strchr(line + startIndex, ASSIGNMENT_SPECIFIER)) == NULL ||
			tmpPtr == line + strlen(line) - 1) {
		*msg = SP_CONFIG_INVALID_STRING;
		printErrorMessage(filename, lineNum, INVALID_CONF_FILE, NULL);
		return false;
	}

	*value = tmpPtr + 1;
	tmpPtr[0] = NULL_CHARACTER;
	*varName = line + startIndex;

	// clear spaces from end of varName
	for (i = strlen(*varName) - 1; i > 0 && isspace((*varName)[i]); i--);
	(*varName)[i+1] = NULL_CHARACTER;

	// clear spaces from beginning of value
	for (startIndex = 0; startIndex < strlen(*value) &&
	isspace((*value)[startIndex]); startIndex++);
	*value = (*value) + startIndex;

	// TODO - forum: should clear only \n?
	// clear spaces from end of value
	for (i = strlen(*value) - 1; i > 0 && isspace((*value)[i]); i--);
	(*value)[i+1] = NULL_CHARACTER;

	return true;
}

bool handleStringField(char** strField, const char* filename, int lineNum,
		const char* value, SP_CONFIG_MSG* msg, bool isImagesSuffix) {
	unsigned int i;

	if (*strField != NULL) {
		free(*strField);
		*strField = NULL;
	}

	for (i = 0; i < strlen(value); i++) {
		if (isspace(value[i])) {
			*msg = SP_CONFIG_INVALID_STRING;
			printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
			return false;
		}
	}

	if (strlen(value) == 0 ||
			(isImagesSuffix && strcmp(value, JPG_FILE_EXTENSION) &&
					strcmp(value, PNG_FILE_EXTENSION) && strcmp(value, BMP_FILE_EXTENSION)
					&& strcmp(value, GIF_FILE_EXTENSION))) {
		*msg = SP_CONFIG_INVALID_STRING;
		printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
		return false;
	}

	if ((*strField = duplicateString(value)) == NULL)
		*msg = SP_CONFIG_ALLOC_FAIL;

	return *strField != NULL;
}

bool isValidInt(char* strVal, int* intVal) {
	// we check if the last parsed char is '\0' to know if we reached the end of the
	// string in parsing (to avoid cases like floating point, white space after num, etc.)
	char* endOfParse;
	*intVal = strtol(strVal, &endOfParse, 10);
	return *endOfParse == NULL_CHARACTER;
}

bool handlePositiveIntField(int* posIntField, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg) {
	int tmpInt;
	VALIDATE_INT(tmpInt <= 0);
	*posIntField = tmpInt;
	return true;
}

bool handlePCADimension(SPConfig config, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg) {
	int tmpInt;
	VALIDATE_INT(tmpInt < PCA_DIM_MIN_VALID_VAL || tmpInt > PCA_DIM_MAX_VALID_VAL);
	config->spPCADimension = tmpInt;
	return true;
}

bool handleBoolField(bool* boolField, const char* filename, int lineNum,
		char* value, SP_CONFIG_MSG* msg) {
	if (!strcmp(value, TRUE_AS_STR))
		*boolField = true;

	else if (!strcmp(value, FALSE_AS_STR))
		*boolField = false;

	else {
		*msg = SP_CONFIG_INVALID_STRING;
		printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
		return false;
	}

	return true;
}

bool handleKDTreeSplitMethod(SPConfig config, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg) {
	if (!strcmp(value, RAND_SPLIT_METHOD))
		config->spKDTreeSplitMethod = RANDOM;

	else if (!strcmp(value, MAX_SPREAD_SPLIT_METHOD))
		config->spKDTreeSplitMethod = MAX_SPREAD;

	else if (!strcmp(value, INC_SPLIT_METHOD))
		config->spKDTreeSplitMethod = INCREMENTAL;

	else {
		*msg = SP_CONFIG_INVALID_STRING;
		printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
		return false;
	}

	return true;
}

bool handleLoggerLevel(SPConfig config, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg) {
	int tmpInt;
	VALIDATE_INT(tmpInt < LOG_LVL_MIN_VALID_VAL || tmpInt > LOG_LVL_MAX_VALID_VAL);

	switch(tmpInt) {
	case 1:
		config->spLoggerLevel = SP_LOGGER_ERROR_LEVEL;
		break;
	case 2:
		config->spLoggerLevel = SP_LOGGER_WARNING_ERROR_LEVEL;
		break;
	case 3:
		config->spLoggerLevel = SP_LOGGER_INFO_WARNING_ERROR_LEVEL;
		break;
	case 4:
		config->spLoggerLevel = SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL;
	}

	return true;
}

bool handleVariable(SPConfig config, const char* filename, int lineNum,
		char *varName, char *value, SP_CONFIG_MSG* msg) {
	if (!strcmp(varName, SP_IMAGES_DIRECTORY))
		return handleStringField(&(config->spImagesDirectory), filename,
				lineNum, value, msg, false);

	if (!strcmp(varName, SP_IMAGES_PREFIX))
		return handleStringField(&(config->spImagesPrefix), filename, lineNum,
				value, msg, false);

	if (!strcmp(varName, SP_IMAGES_SUFFIX))
		return handleStringField(&(config->spImagesSuffix), filename, lineNum,
				value, msg, true);

	if (!strcmp(varName, SP_NUM_OF_IMAGES))
		return handlePositiveIntField(&(config->spNumOfImages), filename,
				lineNum, value, msg);

	if (!strcmp(varName, SP_PCA_DIMENSION))
		return handlePCADimension(config, filename, lineNum, value, msg);

	if (!strcmp(varName, SP_PCA_FILENAME))
		return handleStringField(&(config->spPCAFilename), filename, lineNum,
				value, msg, false);

	if (!strcmp(varName, SP_NUM_OF_FEATURES))
		return handlePositiveIntField(&(config->spNumOfFeatures), filename,
				lineNum, value, msg);

	if (!strcmp(varName, SP_EXTRACION_MODE))
		return handleBoolField(&(config->spExtractionMode), filename,
				lineNum, value, msg);

	if (!strcmp(varName, SP_NUM_OF_SIM_IMAGES))
		return handlePositiveIntField(&(config->spNumOfSimilarImages),
				filename, lineNum, value, msg);

	if (!strcmp(varName, SP_KDTREE_SPLIT_MTD))
		return handleKDTreeSplitMethod(config, filename, lineNum, value, msg);

	if (!strcmp(varName, SP_KNN))
		return handlePositiveIntField(&(config->spKNN), filename, lineNum,
				value, msg);

	if (!strcmp(varName, SP_MINIMAL_GUI))
		return handleBoolField(&(config->spMinimalGUI), filename, lineNum,
				value, msg);

	if (!strcmp(varName, SP_LOGGER_LVL))
		return handleLoggerLevel(config, filename, lineNum, value, msg);

	if (!strcmp(varName, SP_LOGGER_FILENAME))
		return handleStringField(&(config->spLoggerFilename), filename,
				lineNum, value, msg, false);

	*msg = SP_CONFIG_INVALID_STRING;
	printErrorMessage(filename, lineNum, INVALID_CONF_FILE, NULL);
	return false;
}

SPConfig onError(SPConfig config, FILE* configFile) {
	if (configFile)
		fclose(configFile);
	spConfigDestroy(config);
	return NULL;
}

SPConfig parameterSetCheck(SPConfig config, SP_CONFIG_MSG* msg,
		const char* filename, int lineNum, FILE* configFile) {
	const char* parameterName;

	if (!config->spImagesDirectory) {
		parameterName = SP_IMAGES_DIRECTORY;
		*msg = SP_CONFIG_MISSING_DIR;

	} else if (!config->spImagesPrefix) {
		parameterName = SP_IMAGES_PREFIX;
		*msg = SP_CONFIG_MISSING_PREFIX;

	} else if (!config->spImagesSuffix) {
		parameterName = SP_IMAGES_SUFFIX;
		*msg = SP_CONFIG_MISSING_SUFFIX;

	} else if (!config->spNumOfImages) {
		parameterName = SP_NUM_OF_IMAGES;
		*msg = SP_CONFIG_MISSING_NUM_IMAGES;
	}

	if (*msg != SP_CONFIG_SUCCESS) {
		printErrorMessage(filename, lineNum, PARAMETER_NOT_SET, parameterName);
		return onError(config, configFile);
	}

	if (configFile)
		fclose(configFile);

	return config;
}

SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	SPConfig config;
	FILE* configFile = NULL;
	char line[MAX_LINE_LENGTH];
	int lineNum = 0;
	char* varName, *value;
	bool isCommentOrEmpty;

	*msg = SP_CONFIG_SUCCESS;
	if (filename == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return NULL;
	}

	configFile = fopen(filename, "r");
	if (configFile == NULL) {
		*msg = SP_CONFIG_CANNOT_OPEN_FILE;
		return NULL;
	}

	config = (SPConfig)calloc(1, sizeof(struct sp_config_t));
	if (config == NULL) {
		*msg = SP_CONFIG_ALLOC_FAIL;
		return NULL;
	}

	initConfigToDefault(config);

	while(fgets(line, MAX_LINE_LENGTH, configFile) != NULL) {
		isCommentOrEmpty = false;
		if (!parseLine(filename, ++lineNum, line, &varName, &value,
				&isCommentOrEmpty, msg)) {
			return onError(config, configFile);
		}
		if (!isCommentOrEmpty && !handleVariable(config, filename, lineNum,
				varName, value, msg)) {
			return onError(config, configFile);
		}
	}

	if (!checkAndSetDefIfNeeded(&(config->spPCAFilename), DEFAULT_PCA_FILENAME, msg) ||
		!checkAndSetDefIfNeeded(&(config->spLoggerFilename), DEFAULT_LOGGER_FILENAME, msg)) {
		return onError(config, configFile);
	}

	return parameterSetCheck(config, msg, filename, lineNum, configFile);
}

bool isValid(const SPConfig config, SP_CONFIG_MSG* msg, const char* function, int line) {
	//TODO - forum: what to do in case msg in NULL?
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		spLoggerPrintError(ERROR_INVALID_CONF_ARG, __FILE__, function, line);
		return false;
	}
	*msg = SP_CONFIG_SUCCESS;
	return true;
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spExtractionMode : false;
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spMinimalGUI : false;
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spNumOfImages : -1;
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spNumOfFeatures : -1;
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spPCADimension : -1;
}

int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spNumOfSimilarImages :
			-1;
}

int spConfigGetKNN(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spKNN : -1;
}

SP_KDTREE_SPLIT_METHOD spConfigGetSplitMethod(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spKDTreeSplitMethod :
			MAX_SPREAD;
}

SP_LOGGER_LEVEL spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spLoggerLevel :
			SP_LOGGER_INFO_WARNING_ERROR_LEVEL;
}

char* spConfigGetLoggerFilename(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg, __FUNCTION__, __LINE__) ? config->spLoggerFilename : NULL;
}

SP_CONFIG_MSG spConfigGetImagePathFeats(char* imagePath, const SPConfig config,
		int index, bool isFeats) {
	//TODO - log this mothafucka
	if (imagePath == NULL || config == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;
	if (index >= config->spNumOfImages)
		return SP_CONFIG_INDEX_OUT_OF_RANGE;

	// TODO - forum: what should we return in case sprintf fails?
	// if config is valid, then so are config->spImagesDirectory, config->spImagesPrefix
	// and config->spImagesSuffix
	if (isFeats)
		sprintf(imagePath, IMAGE_PATH_FORMAT, config->spImagesDirectory,
				config->spImagesPrefix, index, FEATS_FILE_EXTENSION);
	else
		sprintf(imagePath, IMAGE_PATH_FORMAT, config->spImagesDirectory,
				config->spImagesPrefix, index, config->spImagesSuffix);
	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config, int index) {
	return spConfigGetImagePathFeats(imagePath, config, index, false);
}

SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config) {
	//TODO - log this mothafucka
	if (pcaPath == NULL || config == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;

	// if config is valid, then so are config->spImagesDirectory and config->spPCAFilename
	sprintf(pcaPath, PCA_PATH_FORMAT, config->spImagesDirectory,
			config->spPCAFilename);
	return SP_CONFIG_SUCCESS;
}

char* getSignature(const SPConfig config){
	char lastImagePath[MAX_PATH_LEN],*signature = NULL;
	int PCADim,numOfImages,numOfFeatures, rsltFlag;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	numOfImages = spConfigGetNumOfImages(config, &msg);
	if (msg != SP_CONFIG_SUCCESS || numOfImages < 1) {
		spLoggerPrintError(ERROR_CREATING_SIGN, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}
	numOfFeatures = spConfigGetNumOfFeatures(config, &msg);
	VALID_MSG_IN_SIGN;

	PCADim = spConfigGetPCADim(config, &msg);
	VALID_MSG_IN_SIGN;

	msg = spConfigGetImagePath(lastImagePath,config,numOfImages-1);
	VALID_MSG_IN_SIGN;

	signature = (char*)calloc(MAX_PATH_LEN*2,sizeof(char));
	if (signature == NULL){
		spLoggerPrintError(ALLOCATION_FAILED_MSG, __FILE__,
				__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_CREATING_SIGN, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}
	rsltFlag = sprintf(signature, SIGNATURE_FORMAT, lastImagePath,
			numOfImages, numOfFeatures, PCADim);
	if (rsltFlag<0){
		free(signature);
		spLoggerPrintError(ERROR_CREATING_SIGN, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}
	return signature;
}

void freeAndSetToNull(char** field) {
	if (*field != NULL) {
		free(*field);
		*field = NULL;
	}
}

void spConfigDestroy(SPConfig config) {
	if (config != NULL) {
		freeAndSetToNull(&(config->spImagesDirectory));
		freeAndSetToNull(&(config->spImagesPrefix));
		freeAndSetToNull(&(config->spImagesSuffix));
		freeAndSetToNull(&(config->spPCAFilename));
		freeAndSetToNull(&(config->spLoggerFilename));
		free(config);
	}
}

const char* configMsgToStr(SP_CONFIG_MSG msg) {
	switch(msg) {
	case SP_CONFIG_MISSING_DIR:
		return MISSING_DIR_MSG;
	case SP_CONFIG_MISSING_PREFIX:
		return MISSING_PREFIX_MSG;
	case SP_CONFIG_MISSING_SUFFIX:
		return MISSING_SUFFIX_MSG;
	case SP_CONFIG_MISSING_NUM_IMAGES:
		return MISSING_IMAGES_NUM_MSG;
	case SP_CONFIG_CANNOT_OPEN_FILE:
		return CANNOT_OPEN_FILE_MSG;
	case SP_CONFIG_ALLOC_FAIL:
		return ALLOCATION_FAILED_MSG;
	case SP_CONFIG_INVALID_INTEGER:
		return INVALID_INT_MSG;
	case SP_CONFIG_INVALID_STRING:
		return INVALID_STR_MSG;
	case SP_CONFIG_INVALID_ARGUMENT:
		return INVALID_ARG_MSG;
	case SP_CONFIG_INDEX_OUT_OF_RANGE:
		return INDEX_OUT_OF_RANGE_MSG;
	case SP_CONFIG_SUCCESS:
		return SUCCESS_MSG;
	}
	return NULL;
}

char* duplicateString(const char *str)
{
	char* duplicated;
    int len = 1; // for '\0'
    len += strlen(str);
    duplicated = (char*)calloc(len, sizeof(char));
    if (duplicated != NULL) {
        strcpy(duplicated, str);
    }
    return duplicated;
}
