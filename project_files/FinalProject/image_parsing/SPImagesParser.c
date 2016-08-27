#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SPImagesParser.h"
#include "../general_utils/SPUtils.h"

#define DOUBLE_PRECISION 6

#define BREAKLINE_NO_CR							   '\n'
#define BREAKLINE                                  "\r\n"
#define DEF_LINE_LEN                               512

#define HEADER_STRING_FORMAT                       "%d,%d\r\n"
#define POINT_STRING_FORMAT                        "%d%s\r\n"
#define INTERNAL_POINT_DATA_STRING_FORMAT          ",%f%s"

#define ERROR_INVALID_ARGUMENTS                    "Invalid arguments supplied to method"
#define ERROR_OPEN_FILE                            "Could not open file"
#define ERROR_WRITING_FILE                         "Error writing to file"
#define ERROR_READING_FILE                         "Error while reading from file"
#define ERROR_GETTING_HEADER                       "Error creating header"
#define ERROR_TRANSLATING_POINT                    "Error translating point to CSV string"
#define ERROR_STRING_PARSING_WRONG_FORMAT          "Wrong format in image2string parsing"
#define ERROR_GETTING_PATH                         "Error creating file path"
#define ERROR_ALLOCATING_MEMORY                    "Could not allocate memory"
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
#define IMAGE_ERROR_DETAILS_FORMAT                 "Problem: Image index : %d \r\n Description : %s"
#define ERROR_NOT_MATCHING_CONFIG				   "Error loading image data, configuration data does not match"

#define WARNING_WRONG_POINT_SIZE_CALC              "Wrong point CSV size calculation"
#define WARNING_WRONG_DIGITS_CALC                  "Wrong digits calculation"
#define WARNING_TRY_TO_SET_NULL_FEATURES           "Features data is set to NULL"




//TODO - verify we can use a global http://moodle.tau.ac.il/mod/forum/discuss.php?d=77431
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

	if (config == NULL) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

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
	rsltAsString[i] = '\0';
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

	rslt = digits + 4; // +4 stands for "," + "\r\n" + "\0"
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

	tempString[0] = '\0';

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
	int stringLen = 4; // for : ',' + '\r\n\ + '\0\'
	int successFlag;

	if (imageData == NULL) {
		spLoggerPrintError(ERROR_CREATING_IMAGE_STRING_HEADER, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}

	stringLen += getNumOfDigits(imageData->index);
	stringLen += getNumOfDigits(imageData->numOfFeatures);

	rslt = (char*)calloc(sizeof(char),stringLen);

	if (rslt == NULL){
		*message = SP_DP_MEMORY_FAILURE;
		spLoggerPrintError(ERROR_CREATING_IMAGE_STRING_HEADER, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}

	successFlag = sprintf(rslt, HEADER_STRING_FORMAT, imageData->index, imageData->numOfFeatures);

	if (successFlag < 0){
		spLoggerPrintError(ERROR_CREATING_IMAGE_STRING_HEADER, __FILE__,__FUNCTION__, __LINE__);
		free(rslt);
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}
	return rslt;
}

SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image) {
	int i = 0;

	image->index = (int)getFloatingNumberFromSubString(header,&i);
	if (image->index < 0) {
		spLoggerPrintError(ERROR_STRING_PARSING_WRONG_FORMAT, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FORMAT_ERROR;
	}

	i++;

	image->numOfFeatures = (int)getFloatingNumberFromSubString(header,&i);
	if (image->index < 0){
		spLoggerPrintError(ERROR_STRING_PARSING_WRONG_FORMAT, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FORMAT_ERROR;
	}

	return SP_DP_SUCCESS;
}

SPImageData* loadAllImagesData(const SPConfig config, char* configSignature, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData *allImagesData = NULL, currentImageData = NULL;
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	int i,j, numOfImages;

	if (config == NULL || configSignature == NULL)
	{
		spLoggerPrintError(ERROR_LOADING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	numOfImages = spConfigGetNumOfImages(config,&configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_LOADING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}


	allImagesData = (SPImageData*)calloc(sizeof(struct sp_image_data),numOfImages);

	if (allImagesData == NULL){
		spLoggerPrintError(ERROR_LOADING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	for (i = 0 ;i < numOfImages ; i++)
	{
		currentImageData = loadImageData(config, configSignature, i ,createFlag, message);
		if (*message != SP_DP_SUCCESS)
		{
			spLoggerPrintError(ERROR_LOADING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
			// rollback and exit
			for (j=0;j<i;j++)
			{
				freeImageData(allImagesData[j], false, true);
				free(allImagesData);
				allImagesData = NULL;
			}
			return NULL;
		}
		allImagesData[i] = currentImageData;
	}

	if (featuresMatrix != NULL){
		free(featuresMatrix);
		featuresMatrix = NULL;
	}

	return allImagesData;

}

SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *line = NULL;
	SPPoint currentPoint;

	if (imageFile == NULL || imageData== NULL || imageData->featuresArray == NULL || imageData->numOfFeatures<0){
		spLoggerPrintError(ERROR_AT_READING_FEATURES_FROM_FILE, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}

	for (i=0;i<imageData->numOfFeatures;i++)
	{
		line = getLine(imageFile);
		if (line == NULL)
		{
			spLoggerPrintError(ERROR_AT_READING_FEATURES_FROM_FILE, __FILE__,__FUNCTION__, __LINE__);
			spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);

			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray,i);
			free(line);
			return SP_DP_FILE_READ_ERROR;
		}

		currentPoint = parsePointFromString(line, imageData->index, &message);

		if (message != SP_DP_SUCCESS) {
			spLoggerPrintError(ERROR_AT_READING_FEATURES_FROM_FILE, __FILE__,__FUNCTION__, __LINE__);
			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray,i);
			spPointDestroy(currentPoint);
			free(line);
			return message;
		}
		free(line);
		imageData->featuresArray[i] = currentPoint;
	}

	return message;

}


SP_DP_MESSAGES createImageDataByPreloadedPath(SPImageData imageData){
	if (imageData->index < 0 || featuresMatrix[imageData->index] == NULL)
	{
		spLoggerPrintError(ERROR_ANALYZING_FEATURES, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FEATURE_EXTRACTION_ERROR;
	}
	imageData->featuresArray = featuresMatrix[imageData->index]->featuresArray;
	imageData->numOfFeatures = featuresMatrix[imageData->index]->numOfFeatures;

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES loadKnownImageData(char* configSignature, char* imageDataPath, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	FILE* imageFile = NULL;
	imageFile = fopen(imageDataPath, "r");
	if (imageFile == NULL) {
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_OPEN_FILE, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FILE_READ_ERROR;
	}

	message = loadImageDataFromFile(configSignature, imageFile, imageData);

	fclose(imageFile);
	imageFile = NULL;
	return message;
}

SP_DP_MESSAGES loadImageDataFromFile(char* configSignature, FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	char *line = NULL;

	//verify config signature
	line = getLine(imageFile);
	if (line == NULL){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);
		free(line);
		return SP_DP_FILE_READ_ERROR;
	}
	if (strcmp(line,configSignature) != 0){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_NOT_MATCHING_CONFIG, __FILE__,__FUNCTION__, __LINE__);
		free(line);
		return SP_DP_FORMAT_ERROR;
	}
	free(line);

	//read headers
	line = getLine(imageFile);
	if (line == NULL){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);
		free(line);
		return SP_DP_FILE_READ_ERROR;
	}

	loadImageDataFromHeader(line, imageData);
	free(line);

	//read points
	imageData->featuresArray = (SPPoint*)calloc(sizeof(SPPoint),imageData->numOfFeatures);
	//allocation failure
	if (imageData->featuresArray == NULL){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_MEMORY_FAILURE;
	}

	message = readFeaturesFromFile(imageFile, imageData);

	if (message != SP_DP_SUCCESS){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		free(imageData->featuresArray);
	}

	return message;
}

SPImageData loadImageDataByPath(char* configSignature, char* imageDataPath,int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData resultImage = NULL;

	if (imageDataPath == NULL || message == NULL || configSignature == NULL) {
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	resultImage = createImageData(imageIndex);
	if (resultImage == NULL){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_MEMORY_FAILURE; //should only occur when memory allocation failed
		return NULL;
	}

	resultImage->index = imageIndex;

	if (createFlag){
		*message = createImageDataByPreloadedPath(resultImage);
	}
	else{
		*message = loadKnownImageData(configSignature, imageDataPath, resultImage);
	}

	if (*message != SP_DP_SUCCESS){
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		free(resultImage);
		resultImage = NULL;
	}

	return resultImage;
}

SPImageData loadImageData(const SPConfig config, char* configSignature, int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData image;
	char* filePath;

	if (config == NULL || imageIndex < 0 || configSignature == NULL) {
		spLoggerPrintError(ERROR_LOADING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

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
	char *line;
	//write config signature to file
	fputs(configSignature,imageFile);
	fflush(imageFile);

	//write header to file
	line = getImageStringHeader(imageData, &message);

	if (line == NULL)
	{
		spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_GETTING_HEADER, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}
	fputs(line,imageFile);
	fflush(imageFile);
	free(line);
	//write features to file
	for (i=0;i<imageData->numOfFeatures;i++)
	{
		line = pointToString(imageData->featuresArray[i], &message);

		if (message != SP_DP_SUCCESS || line == NULL)
		{
			spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
			spLoggerPrintError(ERROR_TRANSLATING_POINT, __FILE__,__FUNCTION__, __LINE__);
			return SP_DP_INVALID_ARGUMENT;
		}
		fputs(line,imageFile);
		fflush(imageFile);
		free(line);
	}

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES saveImageData(const SPConfig config,char* configSignature, SPImageData imageData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	char* filePath;
	FILE* imageFile;

	if (config == NULL || imageData == NULL) {
		spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}

	filePath = getImagePath(config, imageData->index, true, &outputMessage);

	if (outputMessage != SP_DP_SUCCESS){
		free(filePath);
		spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_GETTING_PATH, __FILE__,__FUNCTION__, __LINE__);
		return outputMessage;
	}

	remove(filePath); //remove old files if exists

	imageFile = fopen(filePath, "ab+"); //ab+ should create of not exist
	if (imageFile == NULL) {
		free(filePath);
		spLoggerPrintError(ERROR_WRITING_IMAGE_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_OPEN_FILE, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FILE_WRITE_ERROR;
	}

	outputMessage = writeImageDataToFile(imageFile, imageData, configSignature);

	fclose(imageFile);
	free(filePath);
	return outputMessage;
}

SP_DP_MESSAGES saveAllImagesData(const SPConfig config, char* configSignature, SPImageData* imagesData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMessage;
	int i, numOfImages;

	if (config == NULL || imagesData == NULL || configSignature == NULL){
		spLoggerPrintError(ERROR_WRITING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}

	numOfImages = spConfigGetNumOfImages(config, &configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_WRITING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}

	for (i= 0 ; i < numOfImages; i++)
	{
		outputMessage = saveImageData(config, configSignature, imagesData[i]);
		if (outputMessage != SP_DP_SUCCESS){
			spLoggerPrintError(ERROR_WRITING_IMAGES_DATA, __FILE__,__FUNCTION__, __LINE__);
			return outputMessage;
		}
	}

	return outputMessage;
}

SPImageData* spImagesParserStartParsingProcess(const SPConfig config, SP_DP_MESSAGES* msg){
	*msg = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMsg = SP_CONFIG_SUCCESS;
	SPImageData* allImagesData = NULL;
	char* configSignature = NULL;
	bool createDatabase;

	if (config == NULL){
		spLoggerPrintError(ERROR_AT_IMAGE_PARSING_PROCESS, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*msg = SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	createDatabase = spConfigIsExtractionMode(config, &configMsg);

	if (configMsg != SP_CONFIG_SUCCESS){
		spLoggerPrintError(ERROR_AT_IMAGE_PARSING_PROCESS, __FILE__,__FUNCTION__, __LINE__);
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*msg = SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	configSignature = getSignature(config);

	if (configSignature == NULL){
		spLoggerPrintError(ERROR_AT_IMAGE_PARSING_PROCESS, __FILE__,__FUNCTION__, __LINE__);
		*msg = SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	assert (!createDatabase || featuresMatrix != NULL);

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


