#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SPImagesParser.h"
#include "../general_utils/SPUtils.h"

#define DOUBLE_PRECISION 						   6

#define BREAKLINE_NO_CR							   '\n'
#define END_OF_STRING							   '\0'
#define DEF_LINE_LEN                               512

//these macros are used to define how many images do we allow to fail at the
//save to .feats or load from .feats process
#define MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS	   20
#define MAX_ERRORS_PERCENTAGE_AT_LOAD_PROCESS	   20

#define HEADER_STRING_FORMAT                       "%d,%d\n"
#define POINT_STRING_FORMAT                        "%d%s\n"
#define INTERNAL_POINT_DATA_STRING_FORMAT          ",%f%s"
#define CREATE_IF_NOT_EXISTS_FILE_MODE		       "ab+"
#define READ_FILE_MODE		       				   "r"

#define FAILED_OPEN_FILE                            "Could not open file"
#define FAILED_WRITING_FILE                         "Failed writing to file"
#define FAILED_READING_FILE                         "Failed while reading from file"
#define FAILED_GETTING_HEADER                       "Failed creating header"
#define FAILED_TRANSLATING_POINT                    "Failed translating point to CSV string"
#define FAILED_STRING_PARSING_WRONG_FORMAT          "Wrong format in image2string parsing"
#define FAILED_GETTING_PATH                         "Failed creating file path"
#define FAILED_ANALYZING_FEATURES                   "Failed while analyzing features"
#define FAILED_CREATING_LOG_MESSAGE                 "Failed creating log message"
#define FAILED_CREATING_IMAGE_PATH                  "Failed creating image or image data path"
#define FAILED_CONVERTING_POINT_TO_STRING           "Failed converting point to string"
#define FAILED_PARSING_STRING_TO_POINT              "Failed parsing point from string"
#define FAILED_CREATING_IMAGE_STRING_HEADER         "Failed converting image to string - error in creating header"
#define FAILED_PARSING_IMAGE_DATA_FROM_HEADER       "Failed parsing image data from header"
#define FAILED_LOADING_IMAGES_DATA                  "Failed at loading images data\n either encountered critical error or max limit of errors has been reached."
#define FAILED_LOADING_IMAGE_DATA                   "Failed at loading an image data"
#define FAILED_AT_READING_FEATURES_FROM_FILE        "Problem at reading features from file"
#define FAILED_WRITING_IMAGES_DATA                  "Failed at writing images data\n either encountered critical error or max limit of errors has been reached."
#define FAILED_WRITING_IMAGE_DATA                   "Failed at writing an image data"
#define FAILED_AT_IMAGE_PARSING_PROCESS             "Failed at image parsing process"
#define IMAGE_FAILED_DETAILS_FORMAT                 "Problem: Image index : %d \n Description : %s"
#define FAILED_NOT_MATCHING_CONFIG				   "Failed loading image data, configuration data does not match"
#define FAILED_READING_A_LINE_FROM_FILE		   	   "Failed reading a line from file"

#define WARNING_WRONG_POINT_SIZE_CALC              "Wrong point CSV size calculation"
#define WARNING_WRONG_DIGITS_CALC                  "Wrong digits calculation"
#define WARNING_TRY_TO_SET_NULL_FEATURES           "Features data is set to NULL"
#define WARNING_CONFIG_SHOULD_NOT_BE_NULL		   "Warning, could not extract config data, when config should not be null"
#define WARNING_FEATURES_NULL_PRE_DATABASE		   "Warning, features matrix is null pre database creation"
#define WARNING_VERY_LONG_LINE 			   	   	   "Warning : A very long line is being read from a features file\n"
#define WARNING_SAVE_IMAGE_FEAT_LIMIT_NOT_REACHED  "Could not save image .feat file\n max limit of saves errors has not yet been reached."
#define WARNING_LOAD_IMAGE_FEAT_LIMIT_NOT_REACHED  "Could not load image .feat file\n max limit of load errors has not yet been reached."

#define DEBUG_GET_LINE_BUFFER_DOUBLED  			   "Get line buffer doubled"
#define DEBUG_LOADING_IMAGE_FROM_FEAT_INDEX 	   "Loading image from .feat file at index - "
#define DEBUG_SAVING_IMAGE_TO_FEAT_INDEX 		   "Saving image to .feat file at index - "
#define DEBUG_LOADING_IMAGES_DATA 				   "Loading images data from .feat files"
#define DEBUG_DONE_LOADING_IMAGES_DATA 			   "Done loading images data from .feat files"
#define DEBUG_SAVING_IMAGES_DATA 				   "Saving images data to .feat files"
#define DEBUG_DONE_SAVING_IMAGES_DATA      		   "Done saving images data to .feat files"

bool isAFullLine(char* line){
	return strlen(line) > 0 && line[strlen(line)-1] == BREAKLINE_NO_CR;
}

void onGetLineError(char* s1, char* s2){
	if (s1) free(s1);
	if (s2) free(s2);
}

char* getLineByMinBufferSize(FILE* fp, int minBufferSize){
	int bufferSize = minBufferSize;
	char *line = NULL, *tempLine = NULL, *rslt = NULL;

	spSafeCalloc(line, char, bufferSize, onGetLineError(line,tempLine));

	rslt = fgets(line,bufferSize,fp);

	if (!feof(fp) && rslt == NULL){
		onGetLineError(line,tempLine);
		return NULL;
	}

	while (!feof(fp) && ! isAFullLine(line)){
		spSafeCalloc(tempLine, char, bufferSize,onGetLineError(line,tempLine));
		strcpy(tempLine,line);

		bufferSize <<= 1; // buffer *= 2
		spLoggerSafePrintDebug(DEBUG_GET_LINE_BUFFER_DOUBLED,
					__FILE__, __FUNCTION__, __LINE__);
		if (bufferSize > 2048){
			spLoggerSafePrintWarning(WARNING_VERY_LONG_LINE,
					__FILE__, __FUNCTION__, __LINE__);
		}

		spSafeRealloc(line, char, bufferSize , onGetLineError(line,tempLine));

		//this is why min_buffer_size must be even
		rslt = fgets(tempLine,bufferSize >> 1,fp);

		if (!feof(fp) && rslt == NULL){
			onGetLineError(line,tempLine);
			return NULL;
		}
		strcat(line,tempLine);
		free(tempLine);
	}
	return line;
}

char* getLine(FILE* fp){
	return getLineByMinBufferSize(fp, DEF_LINE_LEN);
}

char* getImagePath(const SPConfig config, int index, bool dataPath, SP_DP_MESSAGES* message){
	SP_CONFIG_MSG  configMessage;
	char *path  = NULL;

	spVerifyArgumentsWcRnNc(config != NULL, FAILED_CREATING_IMAGE_PATH,
			*message =  SP_DP_INVALID_ARGUMENT);

	spCallocErWc(path, char, MAX_PATH_LEN, FAILED_CREATING_IMAGE_PATH,
			*message =  SP_DP_MEMORY_FAILURE);

	configMessage = spConfigGetImagePathFeats(path, config, index, dataPath);
	spValWcRnNc(configMessage == SP_CONFIG_SUCCESS, FAILED_CREATING_IMAGE_PATH,
			*message =  SP_DP_INVALID_ARGUMENT; free(path) );

	*message = SP_DP_SUCCESS;
	return path;
}

double getFloatingNumberFromSubString(char* myString, int* start) {
	assert(myString != NULL && *start >= 0);
	char rsltAsString[strlen(myString)];
	int i = 0;

	while (isdigit(myString[*start]) || myString[*start] == '.' || myString[*start] == '-'){
		rsltAsString[i] = myString[*start];
		i++;
		(*start)++;
	}
	rsltAsString[i] = END_OF_STRING;
	return atof(rsltAsString);
}

int getNumOfDigits(int x){
	int digits = 0;
	if (x == 0) return 1;

	while (x != 0){
		digits++;
		x/=10;
	}
	return digits;
}

int getPointCSVSize(SPPoint point){
	int rslt, digits, i;
	digits =  getNumOfDigits(spPointGetDimension(point));

	if (digits == 0 && spPointGetDimension(point) != 0){
		spLoggerSafePrintWarning(WARNING_WRONG_DIGITS_CALC,
				__FILE__,__FUNCTION__, __LINE__);
	}

	rslt = digits + 3; // +4 stands for "," + "\n" + "\0"
	for (i = 0 ; i < spPointGetDimension(point) ; i++){
		rslt += DOUBLE_PRECISION + 3; // +2 stands for the ".", "," and possible "-"
		rslt += getNumOfDigits((int)spPointGetAxisCoor(point,i));
	}
	return rslt;
}

char* pointToString(SPPoint point, SP_DP_MESSAGES* message){
	char *rsltString = NULL , *tempString = NULL, *secondTempString;
	int size,i;

	spVerifyArgumentsWcRnNc(point != NULL, FAILED_CONVERTING_POINT_TO_STRING,
			*message = SP_DP_INVALID_ARGUMENT);

	size = getPointCSVSize(point);

	if (size < 4){
		spLoggerSafePrintWarning(WARNING_WRONG_POINT_SIZE_CALC,
				__FILE__,__FUNCTION__, __LINE__);
	}

	spCallocErWc(rsltString, char, size, FAILED_CONVERTING_POINT_TO_STRING,
			*message = SP_DP_MEMORY_FAILURE);
	spCallocErWc(tempString, char, size, FAILED_CONVERTING_POINT_TO_STRING,
			*message = SP_DP_MEMORY_FAILURE);
	spCallocErWc(secondTempString, char, size, FAILED_CONVERTING_POINT_TO_STRING,
			*message = SP_DP_MEMORY_FAILURE);

	tempString[0] = END_OF_STRING;

	//create internal string data
	for (i = spPointGetDimension(point) -1 ; i >= 0 ; i--){
		strcpy(secondTempString, tempString);
		spValWcRnNc(sprintf(tempString, INTERNAL_POINT_DATA_STRING_FORMAT,
				spPointGetAxisCoor(point,i),secondTempString) >= 0,
				FAILED_CONVERTING_POINT_TO_STRING,
				free(tempString); free(rsltString); *message = SP_DP_FORMAT_ERROR );
	}
	spValWcRnNc(sprintf(rsltString, POINT_STRING_FORMAT,
			spPointGetDimension(point),tempString) >= 0,
			FAILED_CONVERTING_POINT_TO_STRING,
			free(tempString); free(rsltString); free(secondTempString); *message = SP_DP_FORMAT_ERROR);

	free(secondTempString);
	free(tempString);
	return rsltString;
}

SPPoint parsePointFromString(char* pointdata, int index, SP_DP_MESSAGES* message){
	SPPoint rsltPoint = NULL;
	int i = 0, j, dim;
	double* data = NULL;

	spMinimalVerifyArgumentsRnNc(pointdata != NULL);

	//extract dimension
	dim = (int)getFloatingNumberFromSubString(pointdata, &i);
	i++;

	if (dim < 0){
		spLoggerSafePrintWarning(FAILED_PARSING_STRING_TO_POINT,
				__FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}

	//allocate main data for the point
	spCallocErWc(data, double, dim, FAILED_PARSING_STRING_TO_POINT,
			*message = SP_DP_MEMORY_FAILURE);

	//extract main data
	for (j = 0 ; j < dim ; j++){
		data[j] = getFloatingNumberFromSubString(pointdata, &i);
		i++;
	}

	//allocate a new point
	spValWcRn((rsltPoint = spPointCreate(data, dim, index)) != NULL,
			FAILED_PARSING_STRING_TO_POINT,
			*message = SP_DP_MEMORY_FAILURE; free(data));

	free(data);
	return rsltPoint;
}

char* getImageStringHeader(SPImageData imageData, SP_DP_MESSAGES* message){
	char *rslt = NULL;
	int stringLen = 3; // for : ',' + '\n' + '\0\'

	spVerifyArgumentsRnNc(imageData != NULL, FAILED_CREATING_IMAGE_STRING_HEADER);

	stringLen += getNumOfDigits(imageData->index);
	stringLen += getNumOfDigits(imageData->numOfFeatures);

	spCallocErWc(rslt, char, stringLen,
			FAILED_CREATING_IMAGE_STRING_HEADER, *message = SP_DP_MEMORY_FAILURE);

	 spValWcRnNc((sprintf(rslt, HEADER_STRING_FORMAT, imageData->index, imageData->numOfFeatures)) >= 0,
			 FAILED_CREATING_IMAGE_STRING_HEADER,
			 free(rslt);
			*message = SP_DP_FORMAT_ERROR);

	return rslt;
}

SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image){
	int i = 0, index;
	spValNc((index = (int)getFloatingNumberFromSubString(header,&i))>= 0 && index == image->index,
			FAILED_STRING_PARSING_WRONG_FORMAT, SP_DP_FORMAT_ERROR);

	i++;
	spValNc((image->numOfFeatures = (int)getFloatingNumberFromSubString(header,&i))>= 0,
			FAILED_STRING_PARSING_WRONG_FORMAT, SP_DP_FORMAT_ERROR);

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES loadAllImagesData(const SPConfig config,char* configSignature, SPImageData* allImagesData){
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i, numOfImages, maxFailsAllowed;

	spVerifyArguments(allImagesData != NULL && config != NULL && configSignature != NULL,
			FAILED_LOADING_IMAGES_DATA, SP_DP_INVALID_ARGUMENT);

	numOfImages = spConfigGetNumOfImages(config, &configMessage);
	spVal(configMessage == SP_CONFIG_SUCCESS, FAILED_LOADING_IMAGES_DATA,
			SP_DP_INVALID_ARGUMENT);

	maxFailsAllowed = numOfImages * MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS / 100;

	for (i = 0 ; i < numOfImages ; i++){
		if ((message = loadImageData(config, configSignature, i , allImagesData)) != SP_DP_SUCCESS){
			if (maxFailsAllowed == 0 || message == SP_DP_MEMORY_FAILURE){ //reached limit - report error and return message
				spLoggerSafePrintError(FAILED_LOADING_IMAGES_DATA,
									__FILE__, __FUNCTION__, __LINE__);
				return message;

			} else { //report warning
				spLoggerSafePrintWarning(WARNING_LOAD_IMAGE_FEAT_LIMIT_NOT_REACHED,
										__FILE__, __FUNCTION__, __LINE__);
				resetImageData(allImagesData[i]); //treat as no features
				maxFailsAllowed--;
			}

		}
	}
	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *line = NULL;
	SPPoint currentPoint;

	spVerifyArgumentsNc(imageFile != NULL && imageData != NULL &&
			imageData->featuresArray != NULL && imageData->numOfFeatures>=0,
			FAILED_AT_READING_FEATURES_FROM_FILE, SP_DP_INVALID_ARGUMENT);

	for (i = 0 ; i < imageData->numOfFeatures ; i++){
		spValWc((line = getLine(imageFile)) != NULL, FAILED_AT_READING_FEATURES_FROM_FILE,
			spLoggerSafePrintError(FAILED_READING_FILE, __FILE__,__FUNCTION__, __LINE__);
			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray, i);
			free(line), SP_DP_MEMORY_FAILURE); // getLine fails only at memory problems, otherwise returns empty line

		currentPoint = parsePointFromString(line, imageData->index, &message);

		spValWcNc(message == SP_DP_SUCCESS, FAILED_AT_READING_FEATURES_FROM_FILE,
				//rollback - free the points that were allocated so far
				freeFeatures(imageData->featuresArray, i);
				spPointDestroy(currentPoint);
				free(line), message);

		free(line);
		imageData->featuresArray[i] = currentPoint;
	}

	return message;
}

SP_DP_MESSAGES loadKnownImageData(char* configSignature, char* imageDataPath, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	FILE* imageFile = NULL;
	spValWcNc((imageFile = fopen(imageDataPath, READ_FILE_MODE)) != NULL,
			FAILED_OPEN_FILE, spLoggerSafePrintWarning(FAILED_LOADING_IMAGE_DATA,
					__FILE__,__FUNCTION__, __LINE__),
			SP_DP_FILE_READ_ERROR);

	message = loadImageDataFromFile(configSignature, imageFile, imageData);

	fclose(imageFile);
	imageFile = NULL;
	return message;
}

SP_DP_MESSAGES loadImageDataFromFile(char* configSignature, FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	char *line = NULL;

	//verify config signature
	spValWc((line = getLine(imageFile))!= NULL, FAILED_READING_FILE,
			spLoggerSafePrintError(FAILED_LOADING_IMAGE_DATA,
					__FILE__,__FUNCTION__, __LINE__);free(line),
					SP_DP_MEMORY_FAILURE); // getLine fails only at memory problems, otherwise returns empty line

	spValWcNc(strcmp(line,configSignature) == 0,FAILED_LOADING_IMAGE_DATA,
			spLoggerSafePrintWarning(FAILED_NOT_MATCHING_CONFIG,
					__FILE__,__FUNCTION__, __LINE__);free(line),
			SP_DP_FORMAT_ERROR);

	free(line);

	//read headers
	spValWc((line = getLine(imageFile))!= NULL, FAILED_READING_FILE,
			spLoggerSafePrintError(FAILED_LOADING_IMAGE_DATA,
					__FILE__,__FUNCTION__, __LINE__);free(line),
					SP_DP_MEMORY_FAILURE); // getLine fails only at memory problems, otherwise returns empty line

	spValWcNc((message = loadImageDataFromHeader(line, imageData)) == SP_DP_SUCCESS,
			FAILED_LOADING_IMAGE_DATA, free(line), message);

	free(line);

	//read points
	spCallocEr(imageData->featuresArray, SPPoint, imageData->numOfFeatures,
			FAILED_LOADING_IMAGE_DATA, SP_DP_MEMORY_FAILURE);

	spValWcNc((message = readFeaturesFromFile(imageFile, imageData)) == SP_DP_SUCCESS,
			FAILED_LOADING_IMAGE_DATA, free(imageData->featuresArray), message);

	return message;
}

SP_DP_MESSAGES loadImageData(const SPConfig config, char* configSignature, int imageIndex,
		SPImageData* allImagesData){
	char* filePath;
	SP_DP_MESSAGES message = SP_DP_SUCCESS;

	spVerifyArgumentsNc( config != NULL && imageIndex >= 0 && configSignature != NULL
			&& allImagesData != NULL, FAILED_LOADING_IMAGE_DATA, SP_DP_INVALID_ARGUMENT);

	filePath = getImagePath(config, imageIndex, true, &message);

	if (message == SP_DP_SUCCESS){
		message = loadKnownImageData(configSignature, filePath, allImagesData[imageIndex]);
	}
	else{
		spLoggerSafePrintWarning(FAILED_LOADING_IMAGE_DATA,
				__FILE__,__FUNCTION__, __LINE__);
	}

	free(filePath);
	return message;
}

SP_DP_MESSAGES writeImageDataToFile(FILE* imageFile, SPImageData imageData, char* configSignature){
	assert(imageFile != NULL && imageData != NULL && configSignature != NULL);
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *line = NULL;
	//write config signature to file
	spValNc(fputs(configSignature,imageFile) >= 0, FAILED_WRITING_IMAGE_DATA, SP_DP_FILE_WRITE_ERROR);

	fflush(imageFile);

	//write header to file
	line = getImageStringHeader(imageData, &message);

	spValWcNc(line != NULL, FAILED_GETTING_HEADER,
			spLoggerSafePrintWarning(FAILED_WRITING_IMAGE_DATA,
					__FILE__,__FUNCTION__, __LINE__),
			SP_DP_INVALID_ARGUMENT);

	spValWcNc(fputs(line,imageFile) >= 0 ,FAILED_WRITING_IMAGE_DATA,
			free(line),
			SP_DP_FILE_WRITE_ERROR);

	fflush(imageFile);
	free(line);
	//write features to file
	for (i = 0; i < imageData->numOfFeatures ; i++){
		line = pointToString(imageData->featuresArray[i], &message);

		spValWcNc(message == SP_DP_SUCCESS && line != NULL, FAILED_TRANSLATING_POINT,
				free(line),
				SP_DP_INVALID_ARGUMENT);

		spValWcNc(fputs(line,imageFile) >= 0 , FAILED_WRITING_IMAGE_DATA,
				free(line),
				SP_DP_FILE_WRITE_ERROR);

		fflush(imageFile);
		free(line);
	}

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES saveImageData(const SPConfig config,char* configSignature, SPImageData imageData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	char* filePath;
	FILE* imageFile;

	spVerifyArgumentsNc(config != NULL && imageData != NULL, FAILED_WRITING_IMAGE_DATA, SP_DP_INVALID_ARGUMENT);

	filePath = getImagePath(config, imageData->index, true, &outputMessage);

	if (outputMessage != SP_DP_SUCCESS){
		free(filePath);
		spLoggerSafePrintWarning(FAILED_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerSafePrintWarning(FAILED_GETTING_PATH, __FILE__,__FUNCTION__, __LINE__);
		return outputMessage;
	}

	remove(filePath); //remove old files if exists

	spValWcNc((imageFile = fopen(filePath, CREATE_IF_NOT_EXISTS_FILE_MODE)) != NULL, FAILED_OPEN_FILE,
			spLoggerSafePrintWarning(FAILED_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
			free(filePath), SP_DP_FILE_WRITE_ERROR);


	outputMessage = writeImageDataToFile(imageFile, imageData, configSignature);

	fclose(imageFile);
	free(filePath);
	return outputMessage;
}

SP_DP_MESSAGES saveAllImagesData(const SPConfig config, char* configSignature, SPImageData* imagesData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMessage;
	int i, numOfImages, maxFailsAllowed;

	spVerifyArguments(config != NULL && imagesData != NULL && configSignature != NULL,
			FAILED_WRITING_IMAGES_DATA, SP_DP_INVALID_ARGUMENT);

	numOfImages = spConfigGetNumOfImages(config, &configMessage);

	spValWc(configMessage == SP_CONFIG_SUCCESS, ERROR_INVALID_ARGUMENT,
			spLoggerSafePrintError(FAILED_WRITING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
			spLoggerSafePrintWarning(WARNING_CONFIG_SHOULD_NOT_BE_NULL, __FILE__,__FUNCTION__, __LINE__),
					SP_DP_INVALID_ARGUMENT);

	maxFailsAllowed = numOfImages * MAX_ERRORS_PERCENTAGE_AT_SAVE_PROCESS / 100;

	for (i = 0 ; i < numOfImages ; i++){
		spLoggerSafePrintDebugWithIndex(DEBUG_SAVING_IMAGE_TO_FEAT_INDEX, i,
					__FILE__, __FUNCTION__, __LINE__);
		outputMessage = saveImageData(config, configSignature, imagesData[i]);
		if (outputMessage != SP_DP_SUCCESS){
			if (maxFailsAllowed == 0 || outputMessage == SP_DP_MEMORY_FAILURE){
				spLoggerSafePrintError(FAILED_WRITING_IMAGES_DATA,
									__FILE__, __FUNCTION__, __LINE__);
				return outputMessage;
				//log error and return message
			} else {
				spLoggerSafePrintWarning(WARNING_SAVE_IMAGE_FEAT_LIMIT_NOT_REACHED,
										__FILE__, __FUNCTION__, __LINE__);
				//log warning
				maxFailsAllowed--;
			}
		}
	}

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES spImagesParserStartParsingProcess(const SPConfig config, SPImageData* allImagesData){
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMsg = SP_CONFIG_SUCCESS;
	char* configSignature = NULL;
	bool createDatabase;

	spVerifyArguments(config != NULL, FAILED_AT_IMAGE_PARSING_PROCESS, SP_DP_INVALID_ARGUMENT);

	createDatabase = spConfigIsExtractionMode(config, &configMsg);

	spValWc(configMsg == SP_CONFIG_SUCCESS, ERROR_INVALID_ARGUMENT,
			spLoggerSafePrintError(FAILED_AT_IMAGE_PARSING_PROCESS,
					__FILE__,__FUNCTION__, __LINE__);
			spLoggerSafePrintWarning(WARNING_CONFIG_SHOULD_NOT_BE_NULL,
					__FILE__,__FUNCTION__, __LINE__), SP_DP_INVALID_ARGUMENT);


	configSignature = getSignature(config);
	spVal(configSignature != NULL, FAILED_AT_IMAGE_PARSING_PROCESS, SP_DP_INVALID_ARGUMENT);

	if (!createDatabase) {
		spLoggerSafePrintDebug(DEBUG_LOADING_IMAGES_DATA,
					__FILE__, __FUNCTION__, __LINE__);
		msg = loadAllImagesData(config, configSignature, allImagesData);
		spLoggerSafePrintDebug(DEBUG_DONE_LOADING_IMAGES_DATA,
					__FILE__, __FUNCTION__, __LINE__);
	} else {
		// already loaded allImagesData at main
		spLoggerSafePrintDebug(DEBUG_SAVING_IMAGES_DATA,
					__FILE__, __FUNCTION__, __LINE__);
		msg = saveAllImagesData(config, configSignature, allImagesData);
		spLoggerSafePrintDebug(DEBUG_DONE_SAVING_IMAGES_DATA,
					__FILE__, __FUNCTION__, __LINE__);
	}
	spVal(msg == SP_DP_SUCCESS, FAILED_AT_IMAGE_PARSING_PROCESS, msg);

	free(configSignature);

	return SP_DP_SUCCESS;
}
