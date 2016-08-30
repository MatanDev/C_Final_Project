#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SPImagesParser.h"
#include "../general_utils/SPUtils.h"

#define DOUBLE_PRECISION 6

#define BREAKLINE_NO_CR							   '\n'
#define END_OF_STRING							   '\0'
#define DEF_LINE_LEN                               512

#define HEADER_STRING_FORMAT                       "%d,%d\n"
#define POINT_STRING_FORMAT                        "%d%s\n"
#define INTERNAL_POINT_DATA_STRING_FORMAT          ",%f%s"
#define CREATE_IF_NOT_EXISTS_FILE_MODE		       "ab+"

#define ERROR_OPEN_FILE                            "Could not open file"
#define ERROR_WRITING_FILE                         "Error writing to file"
#define ERROR_READING_FILE                         "Error while reading from file"
#define ERROR_GETTING_HEADER                       "Error creating header"
#define ERROR_TRANSLATING_POINT                    "Error translating point to CSV string"
#define ERROR_STRING_PARSING_WRONG_FORMAT          "Wrong format in image2string parsing"
#define ERROR_GETTING_PATH                         "Error creating file path"
#define ERROR_ANALYZING_FEATURES                   "Error while analyzing features"
#define ERROR_CREATING_LOG_MESSAGE                 "Error creating log message"
#define ERROR_CREATING_IMAGE_PATH                  "Error creating image or image data path"
#define ERROR_CONVERTING_POINT_TO_STRING           "Error converting point to string"
#define ERROR_PARSING_STRING_TO_POINT              "Error parsing point from string"
#define ERROR_CREATING_IMAGE_STRING_HEADER         "Error converting image to string - error in creating header"
#define ERROR_PARSING_IMAGE_DATA_FROM_HEADER       "Error parsing image data from header"
#define ERROR_LOADING_IMAGES_DATA                  "Error at loading images data"
#define ERROR_LOADING_IMAGE_DATA                   "Error at loading an image data"
#define ERROR_AT_READING_FEATURES_FROM_FILE        "Problem at reading features from file"
#define ERROR_WRITING_IMAGES_DATA                  "Error at writing images data"
#define ERROR_WRITING_IMAGE_DATA                   "Error at writing an image data"
#define ERROR_AT_IMAGE_PARSING_PROCESS             "Error at image parsing process"
#define IMAGE_ERROR_DETAILS_FORMAT                 "Problem: Image index : %d \n Description : %s"
#define ERROR_NOT_MATCHING_CONFIG				   "Error loading image data, configuration data does not match"

#define WARNING_WRONG_POINT_SIZE_CALC              "Wrong point CSV size calculation"
#define WARNING_WRONG_DIGITS_CALC                  "Wrong digits calculation"
#define WARNING_TRY_TO_SET_NULL_FEATURES           "Features data is set to NULL"
#define WARNING_CONFIG_SHOULD_NOT_BE_NULL		   "Warning, could not extract config data, when config should not be null"
#define WARNING_FEATURES_NULL_PRE_DATABASE		   "Warning, features matrix is null pre database creation"



//TODO - forum - verify we can use a global http://moodle.tau.ac.il/mod/forum/discuss.php?d=77431
//global variable for holding the features matrix
SPImageData* featuresMatrix = NULL;

bool isAFullLine(char* line){
	return line[strlen(line)-1] == BREAKLINE_NO_CR;
}

void onGetLineError(char* s1, char* s2){
	if (s1) free(s1);
	if (s2) free(s2);
}

char* getLineByMinBufferSize(FILE* fp, int min_buffer_size){
	int buffer_size = min_buffer_size;
	char *line = NULL, *tempLine = NULL, *rslt = NULL;

	spSafeCalloc(line, char, buffer_size, onGetLineError(line,tempLine));

	rslt = fgets(line,buffer_size,fp);

	if (!feof(fp) && rslt == NULL){
		onGetLineError(line,tempLine);
		return NULL;
	}

	while (!feof(fp) && ! isAFullLine(line)){
		spSafeCalloc(tempLine, char, buffer_size,onGetLineError(line,tempLine));
		strcpy(tempLine,line);

		buffer_size <<= 1; // buffer *= 2

		spSafeRealloc(line, char, buffer_size , onGetLineError(line,tempLine));

		rslt = fgets(tempLine,buffer_size >> 1,fp);

		if (!feof(fp) && rslt == NULL){
			onGetLineError(line,tempLine);
			return NULL;
		}
		strcat(line,tempLine);
	}
	free(tempLine);
	return line;
}

char* getLine(FILE* fp){
	return getLineByMinBufferSize(fp, DEF_LINE_LEN);
}

char* getImagePath(const SPConfig config,int index,bool dataPath, SP_DP_MESSAGES* message){
	SP_CONFIG_MSG  configMessage;
	char *path  = NULL;

	spVerifyArgumentsWcRn(config != NULL, ERROR_CREATING_IMAGE_PATH, *message =  SP_DP_INVALID_ARGUMENT);

	spCallocErWc(path, char, MAX_PATH_LEN, ERROR_CREATING_IMAGE_PATH, *message =  SP_DP_MEMORY_FAILURE);

	configMessage = spConfigGetImagePathFeats(path,config,index,dataPath);
	spValWcRn(configMessage == SP_CONFIG_SUCCESS, ERROR_CREATING_IMAGE_PATH, *message =  SP_DP_INVALID_ARGUMENT; free(path) );

	*message = SP_DP_SUCCESS;
	return path;

}

double getFloatingNumberFromSubString(char* myString, int* start){
	assert (myString != NULL && *start >= 0);
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

	if (x==0)
		return 1;

	while (x!=0){
		digits++;
		x/=10;
	}
	return digits;
}

void setFeaturesMatrix(SPImageData* features){
	if (features == NULL){
		spLoggerPrintWarning(WARNING_TRY_TO_SET_NULL_FEATURES, __FILE__,__FUNCTION__, __LINE__);
	}
	featuresMatrix = features;
}

int getPointCSVSize(SPPoint point){
	int rslt, digits, i;
	digits =  getNumOfDigits(spPointGetDimension(point));

	if ( digits == 0 && spPointGetDimension(point) != 0){
		spLoggerPrintWarning(WARNING_WRONG_DIGITS_CALC, __FILE__,__FUNCTION__, __LINE__);
	}

	rslt = digits + 3; // +4 stands for "," + "\n" + "\0"
	for (i= 0; i<spPointGetDimension(point);i++){
		rslt += DOUBLE_PRECISION + 3; // +2 stands for the ".", "," and possible "-"
		rslt += getNumOfDigits((int)spPointGetAxisCoor(point,i));
	}
	return rslt;
}

char* pointToString(SPPoint point, SP_DP_MESSAGES* message){
	char *rsltString = NULL , *tempString = NULL, *secondTempString;
	int size,i;

	if (point == NULL){
		*message = SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	size = getPointCSVSize(point);

	if ( size < 4){
		spLoggerPrintWarning(WARNING_WRONG_POINT_SIZE_CALC, __FILE__,__FUNCTION__, __LINE__);
	}

	spCallocErWc(rsltString, char, size, ERROR_CONVERTING_POINT_TO_STRING,*message = SP_DP_MEMORY_FAILURE);
	spCallocErWc(tempString, char, size, ERROR_CONVERTING_POINT_TO_STRING,*message = SP_DP_MEMORY_FAILURE);
	spCallocErWc(secondTempString, char, size, ERROR_CONVERTING_POINT_TO_STRING,*message = SP_DP_MEMORY_FAILURE);

	tempString[0] = END_OF_STRING;

	//create internal string data
	for (i = spPointGetDimension(point) -1 ;i>=0;i--){
		strcpy(secondTempString, tempString);
		spValWcRn(sprintf(tempString, INTERNAL_POINT_DATA_STRING_FORMAT,
				spPointGetAxisCoor(point,i),secondTempString) >= 0,
				ERROR_CONVERTING_POINT_TO_STRING,
				free(tempString); free(rsltString); *message = SP_DP_FORMAT_ERROR );
	}
	spValWcRn(sprintf(rsltString, POINT_STRING_FORMAT,
			spPointGetDimension(point),tempString) >= 0,
			ERROR_CONVERTING_POINT_TO_STRING,
			free(tempString); free(rsltString); free(secondTempString); *message = SP_DP_FORMAT_ERROR );

	free(secondTempString);
	free(tempString);
	return rsltString;
}

SPPoint parsePointFromString(char* pointdata, int index, SP_DP_MESSAGES* message){
	SPPoint rsltPoint = NULL;
	int i = 0, j, dim;
	double* data = NULL;

	if (pointdata == NULL)
		return NULL;

	//extract dimension
	dim = (int)getFloatingNumberFromSubString(pointdata, &i);
	i++;

	if (dim < 0){
		spLoggerPrintError(ERROR_PARSING_STRING_TO_POINT, __FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}

	//allocate main data for the point
	data = (double*) calloc(sizeof(double), dim);

	if (data == NULL){
		spLoggerPrintError(ERROR_PARSING_STRING_TO_POINT, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	//extract main data
	for (j = 0 ; j< dim ; j++){
		data[j] = getFloatingNumberFromSubString(pointdata, &i);
		i++;
	}


	//allocate a new point
	rsltPoint = spPointCreate(data,dim,index);
	free(data);

	if (rsltPoint == NULL){
		spLoggerPrintError(ERROR_PARSING_STRING_TO_POINT, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	return rsltPoint;
}

char* getImageStringHeader(SPImageData imageData, SP_DP_MESSAGES* message){
	char *rslt = NULL;
	int stringLen = 3; // for : ',' + '\n' + '\0\'

	spVerifyArgumentsRn(imageData != NULL, ERROR_CREATING_IMAGE_STRING_HEADER);

	stringLen += getNumOfDigits(imageData->index);
	stringLen += getNumOfDigits(imageData->numOfFeatures);

	spCallocErWc(rslt, char, stringLen,
			ERROR_CREATING_IMAGE_STRING_HEADER, *message = SP_DP_MEMORY_FAILURE);

	 spValWcRn((sprintf(rslt, HEADER_STRING_FORMAT, imageData->index, imageData->numOfFeatures)) >= 0,
			 ERROR_CREATING_IMAGE_STRING_HEADER,
			 free(rslt);
			*message = SP_DP_FORMAT_ERROR);

	return rslt;
}

SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image) {
	int i = 0;

	spVal((image->index = (int)getFloatingNumberFromSubString(header,&i))>= 0,
			ERROR_STRING_PARSING_WRONG_FORMAT, SP_DP_FORMAT_ERROR);

	i++;
	spVal((image->numOfFeatures = (int)getFloatingNumberFromSubString(header,&i))>= 0,
			ERROR_STRING_PARSING_WRONG_FORMAT, SP_DP_FORMAT_ERROR);

	return SP_DP_SUCCESS;
}

SPImageData* loadAllImagesData(const SPConfig config, char* configSignature, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData *allImagesData = NULL, currentImageData = NULL;
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	int i, numOfImages;

	spVerifyArgumentsRn(config != NULL && configSignature != NULL, ERROR_LOADING_IMAGES_DATA);

	numOfImages = spConfigGetNumOfImages(config,&configMessage);

	spValWcRn(configMessage == SP_CONFIG_SUCCESS, ERROR_LOADING_IMAGES_DATA, *message =  SP_DP_INVALID_ARGUMENT);

	spCallocErWc(allImagesData, SPImageData, numOfImages,
			ERROR_LOADING_IMAGES_DATA, *message = SP_DP_MEMORY_FAILURE);

	for (i = 0 ;i < numOfImages ; i++) {
		currentImageData = loadImageData(config, configSignature, i ,createFlag, message);
		spValWcRn(*message == SP_DP_SUCCESS, ERROR_LOADING_IMAGES_DATA,
					freeAllImagesData(allImagesData, i, true);
					allImagesData = NULL);
		allImagesData[i] = currentImageData;
	}

	if (featuresMatrix){
		freeAllImagesData(featuresMatrix, numOfImages, false);
	}

	return allImagesData;
}

SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *line = NULL;
	SPPoint currentPoint;

	spVerifyArguments(imageFile != NULL && imageData != NULL && imageData->featuresArray != NULL && imageData->numOfFeatures>=0,
			ERROR_AT_READING_FEATURES_FROM_FILE, SP_DP_INVALID_ARGUMENT);

	for (i=0;i<imageData->numOfFeatures;i++)
	{

		spValWc((line = getLine(imageFile)) != NULL, ERROR_AT_READING_FEATURES_FROM_FILE,
			spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);
			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray,i);
			free(line), SP_DP_FILE_READ_ERROR);

		currentPoint = parsePointFromString(line, imageData->index, &message);

		spValWc(message == SP_DP_SUCCESS, ERROR_AT_READING_FEATURES_FROM_FILE,
				//rollback - free the points that were allocated so far
				freeFeatures(imageData->featuresArray,i);
				spPointDestroy(currentPoint);
				free(line), message);

		free(line);
		imageData->featuresArray[i] = currentPoint;
	}

	return message;

}


SP_DP_MESSAGES createImageDataByPreloadedPath(SPImageData imageData){
	spVerifyArguments(imageData->index >= 0 && featuresMatrix[imageData->index] != NULL,
			ERROR_ANALYZING_FEATURES, SP_DP_FEATURE_EXTRACTION_ERROR);

	imageData->featuresArray = featuresMatrix[imageData->index]->featuresArray;
	imageData->numOfFeatures = featuresMatrix[imageData->index]->numOfFeatures;

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES loadKnownImageData(char* configSignature, char* imageDataPath, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	FILE* imageFile = NULL;
	spValWc((imageFile = fopen(imageDataPath, "r")) != NULL,
			ERROR_OPEN_FILE, spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__),
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
	spValWc((line = getLine(imageFile))!= NULL,ERROR_READING_FILE,
			spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);free(line),
			SP_DP_FILE_READ_ERROR);

	spValWc(strcmp(line,configSignature) == 0,ERROR_LOADING_IMAGE_DATA,
			spLoggerPrintError(ERROR_NOT_MATCHING_CONFIG, __FILE__,__FUNCTION__, __LINE__);free(line),
			SP_DP_FORMAT_ERROR);

	free(line);

	//read headers
	spValWc((line = getLine(imageFile))!= NULL,ERROR_READING_FILE,
			spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);free(line),
			SP_DP_FILE_READ_ERROR);

	spValWc((message = loadImageDataFromHeader(line, imageData)) == SP_DP_SUCCESS,
			ERROR_LOADING_IMAGE_DATA, free(line), message);

	free(line);

	//read points
	spCallocEr(imageData->featuresArray, SPPoint, imageData->numOfFeatures,
			ERROR_LOADING_IMAGE_DATA, SP_DP_MEMORY_FAILURE);

	spValWc((message = readFeaturesFromFile(imageFile, imageData)) == SP_DP_SUCCESS,
			ERROR_LOADING_IMAGE_DATA, free(imageData->featuresArray), message);

	return message;
}

SPImageData loadImageDataByPath(char* configSignature, char* imageDataPath,int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData resultImage = NULL;
	spVerifyArgumentsWcRn( imageDataPath != NULL && message != NULL && configSignature != NULL,
			ERROR_LOADING_IMAGE_DATA, *message =  SP_DP_INVALID_ARGUMENT);

	spValWcRn((resultImage = createImageData(imageIndex)) != NULL, ERROR_LOADING_IMAGE_DATA,
			*message =  SP_DP_MEMORY_FAILURE);

	if (createFlag){
		*message = createImageDataByPreloadedPath(resultImage);
	}
	else{
		*message = loadKnownImageData(configSignature, imageDataPath, resultImage);
	}

	spValWcRn(*message == SP_DP_SUCCESS, ERROR_LOADING_IMAGE_DATA, free(resultImage));

	return resultImage;
}

SPImageData loadImageData(const SPConfig config, char* configSignature, int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData image;
	char* filePath;

	spVerifyArgumentsWcRn( config != NULL && imageIndex >= 0 && configSignature != NULL,
			ERROR_LOADING_IMAGE_DATA, *message =  SP_DP_INVALID_ARGUMENT);

	filePath = getImagePath(config, imageIndex, !createFlag, message);
	if (*message == SP_DP_SUCCESS) {
		image = loadImageDataByPath(configSignature, filePath,imageIndex, createFlag,  message);
	}
	else{
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
	}

	free(filePath);
	return image;
}

SP_DP_MESSAGES writeImageDataToFile(FILE* imageFile, SPImageData imageData, char* configSignature){
	assert(imageFile != NULL && imageData != NULL && configSignature != NULL);
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *line = NULL;
	//write config signature to file
	spVal(fputs(configSignature,imageFile) >= 0 , ERROR_WRITING_IMAGE_DATA,	SP_DP_FILE_WRITE_ERROR);

	fflush(imageFile);

	//write header to file
	line = getImageStringHeader(imageData, &message);

	spValWc(line != NULL, ERROR_GETTING_HEADER,
			spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__),
			SP_DP_INVALID_ARGUMENT);

	spValWc(fputs(line,imageFile) >= 0 , ERROR_WRITING_IMAGE_DATA,
			free(line),
			SP_DP_FILE_WRITE_ERROR);

	fflush(imageFile);
	free(line);
	//write features to file
	for (i=0;i<imageData->numOfFeatures;i++)
	{
		line = pointToString(imageData->featuresArray[i], &message);

		spValWc(message == SP_DP_SUCCESS && line != NULL, ERROR_TRANSLATING_POINT,
				free(line),
				SP_DP_INVALID_ARGUMENT);

		spValWc(fputs(line,imageFile) >= 0 , ERROR_WRITING_IMAGE_DATA,
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

	spVerifyArguments(config != NULL && imageData != NULL, ERROR_WRITING_IMAGE_DATA, SP_DP_INVALID_ARGUMENT);

	filePath = getImagePath(config, imageData->index, true, &outputMessage);

	if (outputMessage != SP_DP_SUCCESS){
		free(filePath);
		spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_GETTING_PATH, __FILE__,__FUNCTION__, __LINE__);
		return outputMessage;
	}

	remove(filePath); //remove old files if exists

	spValWc((imageFile = fopen(filePath, CREATE_IF_NOT_EXISTS_FILE_MODE))!= NULL, ERROR_OPEN_FILE,
			spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
			free(filePath), SP_DP_FILE_WRITE_ERROR);


	outputMessage = writeImageDataToFile(imageFile, imageData, configSignature);

	fclose(imageFile);
	free(filePath);
	return outputMessage;
}

SP_DP_MESSAGES saveAllImagesData(const SPConfig config, char* configSignature, SPImageData* imagesData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMessage;
	int i, numOfImages;

	spVerifyArguments(config != NULL && imagesData != NULL && configSignature != NULL,
			ERROR_WRITING_IMAGES_DATA, SP_DP_INVALID_ARGUMENT);

	numOfImages = spConfigGetNumOfImages(config, &configMessage);

	spValWc(configMessage == SP_CONFIG_SUCCESS, ERROR_INVALID_ARGUMENT,
			spLoggerPrintError(ERROR_WRITING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
			spLoggerPrintWarning(WARNING_CONFIG_SHOULD_NOT_BE_NULL, __FILE__,__FUNCTION__, __LINE__),
					SP_DP_INVALID_ARGUMENT);


	for (i= 0 ; i < numOfImages; i++)
	{
		outputMessage = saveImageData(config, configSignature, imagesData[i]);
		//TODO - ask in forum, maybe we don't want to stop the process ...
		spVal(outputMessage == SP_DP_SUCCESS, ERROR_WRITING_IMAGES_DATA, outputMessage);
	}

	return outputMessage;
}

SPImageData* spImagesParserStartParsingProcess(const SPConfig config, SP_DP_MESSAGES* msg){
	*msg = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMsg = SP_CONFIG_SUCCESS;
	SPImageData* allImagesData = NULL;
	char* configSignature = NULL;
	bool createDatabase;

	spVerifyArgumentsWcRn(config != NULL, ERROR_AT_IMAGE_PARSING_PROCESS,
			 *msg = SP_DP_INVALID_ARGUMENT);

	createDatabase = spConfigIsExtractionMode(config, &configMsg);

	spValWcRn(configMsg == SP_CONFIG_SUCCESS, ERROR_INVALID_ARGUMENT,
			spLoggerPrintError(ERROR_AT_IMAGE_PARSING_PROCESS, __FILE__,__FUNCTION__, __LINE__);
			spLoggerPrintWarning(WARNING_CONFIG_SHOULD_NOT_BE_NULL,
					__FILE__,__FUNCTION__, __LINE__);*msg = SP_DP_INVALID_ARGUMENT);


	configSignature = getSignature(config);
	spValWcRn(configSignature != NULL, ERROR_AT_IMAGE_PARSING_PROCESS, *msg = SP_DP_INVALID_ARGUMENT);


	if (createDatabase && featuresMatrix == NULL){
		spLoggerPrintWarning(WARNING_FEATURES_NULL_PRE_DATABASE,
				__FILE__,__FUNCTION__, __LINE__);
	}

	allImagesData = loadAllImagesData(config, configSignature, createDatabase, msg);

	if (*msg == SP_DP_SUCCESS && createDatabase){
		*msg = saveAllImagesData(config, configSignature, allImagesData);
	}
	if (*msg != SP_DP_SUCCESS){
		spLoggerPrintError(ERROR_AT_IMAGE_PARSING_PROCESS, __FILE__,__FUNCTION__, __LINE__);
	}
	free(configSignature);
	return allImagesData;
}



