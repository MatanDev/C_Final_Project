#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "SPConfig.h"

#define DEFAULT_PCA_DIMENSION	20
#define DEFAULT_PCA_FILENAME	"pca.yml"
#define DEFAULT_NUM_OF_FEATURES	100
#define DEFAULT_NUM_OF_SIM_IMGS	1
#define DEFAULT_KNN				1
#define DEFAULT_LOGGER_LEVEL	3
#define DEFAULT_LOGGER_FILENAME	"stdout"
#define FILE_TEMPLATE_FOR_ERROR	"File: %s\n"
#define LINE_TEMPLATE_FOR_ERROR	"Line: %d\n"
#define INVALID_CONF_MSG		"Message: Invalid configuration line\n"
#define INVALID_VALUE_MSG		"Message: Invalid value - constraint not met\n"
#define PARAM_NOT_SET_MSG		"Message: Parameter %s is not set\n"
#define COMMENT_SPECIFIER		'#'
#define ASSIGNMENT_SPECIFIER	'='
//#define NULL_CHARACTER			'\0'
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
#define SP_KDTREE_SPLIT_METHOD	"spKDTreeSplitMethod"
#define SP_KNN					"spKNN"
#define SP_MINIMAL_GUI			"spMinimalGUI"
#define SP_LOGGER_LEVEL			"spLoggerLevel"
#define SP_LOGGER_FILENAME		"spLoggerFilename"
#define MAX_LINE_LENGTH			1024
//#define FIRST_LINE_INDEX		0
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
	int spLoggerLevel;
	char* spLoggerFilename;
};*/

typedef enum error_message_type_t {
	INVALID_CONF_FILE,
	INVALID_VALUE,
	PARAMETER_NOT_SET
} ERROR_MSG_TYPE;

char* mallocAndCopy(const char* str) {
	char* ret = (char*)malloc(strlen(str) + 1);
	if (ret != NULL)
		strcpy(ret, str);
	return ret;
}

bool checkAndSetDefIfNeeded(char** field, const char* def, SP_CONFIG_MSG* msg) {
	if (*field == NULL) {
		*field = mallocAndCopy(def);
		if (*field == NULL) {
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
		break;
	default:
		break;
	}
}

//TODO - in case of error - what should msg be? - right now: invalid string - ask in forum
/*
 * basically it's like getVarNameAndValueFromLine
 */
bool parseLine(const char* filename, int lineNum, char* line,
		char** varName, char** value, bool* isCommentOrEmpty,
		SP_CONFIG_MSG* msg) {
	int startIndex, i;
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
	tmpPtr[0] = '\0';
	*varName = line + startIndex;

	// clear spaces from end of varName
	for (i = strlen(*varName) - 1; i > 0 && isspace((*varName)[i]); i--);
	(*varName)[i+1] = '\0';

	// clear spaces from beginning of value
	for (startIndex = 0; startIndex < strlen(*value) &&
	isspace((*value)[startIndex]); startIndex++);
	*value = (*value) + startIndex;

	// clear spaces from end of value (TODO - should clear only \n? - ask in forum)
	for (i = strlen(*value) - 1; i > 0 && isspace((*value)[i]); i--);
	(*value)[i+1] = '\0';

	return true;
}

bool handleStringField(char** strField, const char* filename, int lineNum,
		const char* value, SP_CONFIG_MSG* msg, bool isImagesSuffix) {
	int i;

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

	if ((*strField = mallocAndCopy(value)) == NULL)
		*msg = SP_CONFIG_ALLOC_FAIL;

	return *strField != NULL;
}

bool handlePositiveIntField(int* posIntField, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg) {
	int tmpInt;
	if ((tmpInt = atoi(value)) <= 0) {
		*msg = SP_CONFIG_INVALID_INTEGER;
		printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
		return false;
	}

	*posIntField = tmpInt;
	return true;
}

// TODO - maybe unite with previous one
bool handleBoundedPosIntField(int* posIntField, const char* filename,
		int lineNum, char* value, SP_CONFIG_MSG* msg, int from, int to) {
	int tmpInt;
	if ((tmpInt = atoi(value)) < from || tmpInt > to) {
		*msg = SP_CONFIG_INVALID_INTEGER;
		printErrorMessage(filename, lineNum, INVALID_VALUE, NULL);
		return false;
	}

	*posIntField = tmpInt;
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
		return handleBoundedPosIntField(&(config->spPCADimension), filename,
				lineNum, value, msg, 10, 28);

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

	if (!strcmp(varName, SP_KDTREE_SPLIT_METHOD))
		return handleKDTreeSplitMethod(config, filename, lineNum, value, msg);

	if (!strcmp(varName, SP_KNN))
		return handlePositiveIntField(&(config->spKNN), filename, lineNum,
				value, msg);

	if (!strcmp(varName, SP_MINIMAL_GUI))
		return handleBoolField(&(config->spMinimalGUI), filename, lineNum,
				value, msg);

	if (!strcmp(varName, SP_LOGGER_LEVEL))
		return handleBoundedPosIntField(&(config->spLoggerLevel), filename,
				lineNum, value, msg, 1, 4);

	if (!strcmp(varName, SP_LOGGER_FILENAME))
		return handleStringField(&(config->spLoggerFilename), filename,
				lineNum, value, msg, false);

	*msg = SP_CONFIG_INVALID_STRING;
	printErrorMessage(filename, lineNum, INVALID_CONF_FILE, NULL);
	return false;
}

SPConfig onError(SPConfig config) {
	spConfigDestroy(config);
	return NULL;
}

SPConfig parameterSetCheck(SPConfig config, SP_CONFIG_MSG* msg,
		const char* filename, int lineNum) {
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
		return onError(config);
	}

	return config;
}

SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	SPConfig config;
	FILE* configFile;
	char line[MAX_LINE_LENGTH];
	int lineNum = 0; // TODO - should lines start from 0 or 1?
	char* varName, *value;
	bool isCommentOrEmpty = false;

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

	config = (SPConfig)malloc(sizeof(struct sp_config_t));
	if (config == NULL) {
		*msg = SP_CONFIG_ALLOC_FAIL;
		return NULL;
	}

	// TODO - maybe instead of returning bool in the functions we should
	// check if msg is not success?
	initConfigToDefault(config);

	while(fgets(line, 1024, configFile) != NULL) {
		if (!parseLine(filename, lineNum, line, &varName, &value,
				&isCommentOrEmpty, msg)) {
			return onError(config);
		}
		if (!isCommentOrEmpty && !handleVariable(config, filename, lineNum,
				varName, value, msg)) {
			return onError(config);
		}
		lineNum++;
		isCommentOrEmpty = false;
	}

	// TODO - if we fail in initialize to default should we fail the whole
	// opertation? (meaning just assign instead of malloc and copy)
	if (!checkAndSetDefIfNeeded(&config->spPCAFilename, DEFAULT_PCA_FILENAME, msg) ||
		!checkAndSetDefIfNeeded(&config->spLoggerFilename, DEFAULT_LOGGER_FILENAME, msg)) {
		return onError(config);
	}

	return parameterSetCheck(config, msg, filename, lineNum);
}

bool isValid(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return false;
	}
	*msg = SP_CONFIG_SUCCESS;
	return true;
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spExtractionMode : false;
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spMinimalGUI : false;
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spNumOfImages : -1;
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spNumOfFeatures : -1;
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spPCADimension : -1;
}

int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	return isValid(config, msg) ? config->spNumOfSimilarImages : -1;
}

SP_CONFIG_MSG spConfigGetImagePathFeats(char* imagePath, const SPConfig config,
		int index, bool isFeats) {
	if (imagePath == NULL || config == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;
	if (index >= config->spNumOfImages)
		return SP_CONFIG_INDEX_OUT_OF_RANGE;

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

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
		int index) {
	return spConfigGetImagePathFeats(imagePath, config, index, false);
}

SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config) {
	if (pcaPath == NULL || config == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;

	// if config is valid, then so are config->spImagesDirectory and config->spPCAFilename
	sprintf(pcaPath, PCA_PATH_FORMAT, config->spImagesDirectory,
			config->spPCAFilename);
	return SP_CONFIG_SUCCESS;
}

void spConfigDestroy(SPConfig config) {
	if (config != NULL) {
		if (config->spImagesDirectory != NULL) {
			free(config->spImagesDirectory);
			config->spImagesDirectory = NULL;
		}
		if (config->spImagesPrefix != NULL) {
			free(config->spImagesPrefix);
			config->spImagesPrefix = NULL;
		}
		if (config->spImagesSuffix != NULL) {
			free(config->spImagesSuffix);
			config->spImagesSuffix = NULL;
		}
		if (config->spPCAFilename != NULL) {
			free(config->spPCAFilename);
			config->spPCAFilename = NULL;
		}
		if (config->spLoggerFilename != NULL) {
			free(config->spLoggerFilename);
			config->spLoggerFilename = NULL;
		}
		free(config);
	}
}

char* configMsgToStr(SP_CONFIG_MSG msg) {
	switch(msg) {
	case SP_CONFIG_MISSING_DIR:
		return mallocAndCopy(MISSING_DIR_MSG);
	case SP_CONFIG_MISSING_PREFIX:
		return mallocAndCopy(MISSING_PREFIX_MSG);
	case SP_CONFIG_MISSING_SUFFIX:
		return mallocAndCopy(MISSING_SUFFIX_MSG);
	case SP_CONFIG_MISSING_NUM_IMAGES:
		return mallocAndCopy(MISSING_IMAGES_NUM_MSG);
	case SP_CONFIG_CANNOT_OPEN_FILE:
		return mallocAndCopy(CANNOT_OPEN_FILE_MSG);
	case SP_CONFIG_ALLOC_FAIL:
		return mallocAndCopy(ALLOCATION_FAILED_MSG);
	case SP_CONFIG_INVALID_INTEGER:
		return mallocAndCopy(INVALID_INT_MSG);
	case SP_CONFIG_INVALID_STRING:
		return mallocAndCopy(INVALID_STR_MSG);
	case SP_CONFIG_INVALID_ARGUMENT:
		return mallocAndCopy(INVALID_ARG_MSG);
	case SP_CONFIG_INDEX_OUT_OF_RANGE:
		return mallocAndCopy(INDEX_OUT_OF_RANGE_MSG);
	case SP_CONFIG_SUCCESS:
		return mallocAndCopy(SUCCESS_MSG);
	}
	return NULL;
}

