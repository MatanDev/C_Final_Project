#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SPImagesParser.h"

#define DOUBLE_PRECISION 6

#define BREAKLINE "\r\n"
#define MAXLINE_LEN 1024 //TODO - verify what this should be
#define HEADER_STRING_FOMAT "%d,%d\r\n"
#define POINT_STRING_FOMAT "%d%s\r\n"
#define INTERNAL_POINT_DATA_STRING_FORMAT ",%f%s"

#define ERROR_GETTING_PATH "Error creating file path"
#define ERROR_OPEN_FILE "Could not open file"
#define ERROR_WRITING_FILE "Error writing to file"
#define ERROR_READING_FILE "Error while reading from file"
#define ERROR_GETTING_HEADER "Error creating header"
#define ERROR_TRANSLATING_POINT "Error translating point to CSV string"
#define ERROR_INVALID_ARGUMENTS "Invalid arguments supplied to method"
#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"
#define ERROR_ANALYZING_FEATURES "Error while analyzing features"
#define ERROR_CREATING_LOG_MESSAGE "ERROR CREATEING LOG MESSAGE"

#define IMAGE_ERROR_DETAILS_FORMAT "Problem: Image index : %d \r\n Description : %s"


#define WARNING_IMAGES_DATA_NULL "Images data object is null when free is called"
#define WARNING_IMAGE_DATA_NULL "Image data object is null when free is called"
#define WARNING_IMAGE_DATA_POINTS_ARRAY_NULL "Image data points array is null when free is called"

char* wrapImageProblem(char* problem, int imageIndex){
	char* msg = (char*)calloc(sizeof(char),MAXLINE_LEN);

	if (msg == NULL || problem == NULL){
		return ERROR_CREATING_LOG_MESSAGE;
	}
	sprintf(msg,IMAGE_ERROR_DETAILS_FORMAT, imageIndex, problem);

	return msg;
}

void logImageProblem(char* problem, int imageIndex, const char* file,
		const char* function, const int line){
	char* msg = (char*)calloc(sizeof(char),MAXLINE_LEN);

	if (msg == NULL || problem == NULL){
		spLoggerPrintError(ERROR_CREATING_LOG_MESSAGE,file,function,line);
	}

	sprintf(msg,IMAGE_ERROR_DETAILS_FORMAT, imageIndex, problem);
	spLoggerPrintError(msg,file,function,line);

	free(msg);
}

//global variable for holding the features matrix
SPPoint** featuresMatrix = NULL;

char* getImagePath(const SPConfig config,int index,bool dataPath, SP_DP_MESSAGES* message){
	return NULL; //TODO - implement in config
}

double getPositiveNumberFromSubString(char* myString, int* start){
	assert (myString != NULL && *start >= 0);
	char rsltAsString[MAXLINE_LEN];
	int i = 0;

	while (isdigit(myString[*start]) || myString[*start] == '.'){
		rsltAsString[i] = myString[*start];
		i++;
		(*start)++;
	}
	rsltAsString[i] = '\0';
	return atof(rsltAsString);
}

int getNumOfDigits(int x){
	int digits = 0;
	while (x!=0){
		digits++;
		x/=10;
	}
	return digits;
}

void setFeaturesMatrix(SPPoint** features){
	featuresMatrix = features;
}

int getPointCSVSize(SPPoint point){
	int rslt, digits, i;
	digits =  getNumOfDigits(spPointGetDimension(point));
	rslt = digits + 3; // +4 stands for "," + "\r\n" + "\0"
	for (i= 0; i<digits;i++){
		rslt += DOUBLE_PRECISION + 2; // +2 stands for the "." and ","
		rslt += getNumOfDigits((int)spPointGetAxisCoor(point,i));
	}
	return rslt;
}

char* pointToString(SPPoint point, SP_DP_MESSAGES* message){
	char *rsltString = NULL , *tempString = NULL;
	int size,i,rsltFlag;

	if (point == NULL){
		*message = SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	size = getPointCSVSize(point);
	assert(size > 4);
	rsltString = (char*)calloc(sizeof(char),size);
	tempString = (char*)calloc(sizeof(char),size);

	if (rsltString == NULL || tempString == NULL){
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	tempString[0] = '\0';

	//create internal string data
	for (i = spPointGetDimension(point) -1 ;i>=0;i--){
		rsltFlag = sprintf(tempString, INTERNAL_POINT_DATA_STRING_FORMAT,
				spPointGetAxisCoor(point,i),tempString);
		if (rsltFlag < 0){
			free(tempString);
			free(rsltString);
			*message = SP_DP_FORMAT_ERROR;
			return NULL;
		}
	}

	rsltFlag = sprintf(rsltString, POINT_STRING_FOMAT,
			spPointGetDimension(point),tempString);
	if (rsltFlag < 0){
		free(tempString);
		free(rsltString);
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}

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
	dim = (int)getPositiveNumberFromSubString(pointdata, &i);
	i++;

	if (dim < 0){
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}

	//allocate main data for the point
	data = (double*) calloc(sizeof(double), dim);

	if (data == NULL){
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	//extract main data
	for (j = 0 ; j< dim ; j++){
		data[j] = getPositiveNumberFromSubString(pointdata, &i);
		i++;
	}


	//allocate a new point
	rsltPoint = spPointCreate(data,dim,index);
	free(data);

	if (rsltPoint == NULL){
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	return rsltPoint;
}

char* getImageStringHeader(SPImageData imageData, SP_DP_MESSAGES* message){
	char *rslt = NULL;
	int stringLen = 3; // for : ',' + '\r\n\ + '\0\'
	int successFlag;

	if (imageData == NULL)
		return NULL;

	stringLen += getNumOfDigits(imageData->index);
	stringLen += getNumOfDigits(imageData->numOfFeatures);

	rslt = (char*)calloc(sizeof(char),stringLen);

	if (rslt == NULL){
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	successFlag = sprintf(rslt, HEADER_STRING_FOMAT, imageData->index, imageData->numOfFeatures);

	if (successFlag < 0){
		free(rslt);
		*message = SP_DP_FORMAT_ERROR;
		return NULL;
	}
	return rslt;
}

SP_DP_MESSAGES loadImageDataFromHeader(char* header, SPImageData image) {
	int i = 0;

	image->index = (int)getPositiveNumberFromSubString(header,&i);
	if (image->index < 0)
		return SP_DP_FORMAT_ERROR;

	i++;

	image->numOfFeatures = (int)getPositiveNumberFromSubString(header,&i);
	if (image->index < 0)
			return SP_DP_FORMAT_ERROR;

	return SP_DP_SUCCESS;
}

SPImageData* loadAllImagesData(const SPConfig config, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData *allImagesData = NULL, currentImageData = NULL;
	SP_CONFIG_MSG configMessage = SP_CONFIG_SUCCESS;
	int i,j, numOfImages;

	if (config == NULL)
	{
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	numOfImages = spConfigGetNumOfImages(config,&configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		spLoggerPrintError(translateMessageToString(configMessage), __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}


	allImagesData = (SPImageData*)calloc(sizeof(struct sp_image_data),numOfImages);

	if (allImagesData == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		*message = SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	for (i = 0 ;i < numOfImages ; i++)
	{
		currentImageData = loadImageData(config, i ,createFlag, message);
		if (message != SP_DP_SUCCESS)
		{
			// rollback and exit
			for (j=0;j<i;j++)
			{
				freeImageData(allImagesData[j]);
				free(allImagesData);
				allImagesData = NULL;
			}
			return NULL;
		}
		allImagesData[i] = currentImageData;
	}

	return allImagesData;

}

SP_DP_MESSAGES readFeaturesFromFile(FILE* imageFile, SPImageData imageData){
	char* problemData = NULL;
	assert(imageFile != NULL && imageData!= NULL && imageData->featuresArray != NULL && imageData->numOfFeatures>0);
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char *readResult, *line;
	SPPoint currentPoint;

	line = (char*)calloc(sizeof(char),MAXLINE_LEN);
	if (line == NULL){
		problemData = wrapImageProblem(ERROR_ALLOCATING_MEMORY, imageData->index);
		spLoggerPrintError(problemData, __FILE__,__FUNCTION__, __LINE__);
		free(problemData);
	//	spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_MEMORY_FAILURE;
	}

	for (i=0;i<imageData->numOfFeatures;i++)
	{
		readResult = fgets(line, MAXLINE_LEN, imageFile);
		if (readResult == NULL)
		{
			spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);

			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray,i);
			free(line);
			return SP_DP_FILE_READ_ERROR;
		}

		currentPoint = parsePointFromString(line, imageData->index, &message);
		if (message != SP_DP_SUCCESS) {
			//rollback - free the points that were allocated so far
			freeFeatures(imageData->featuresArray,i);
			free(line);
			return message;
		}
		imageData->featuresArray[i] = currentPoint;
	}
	free(line);
	return message;

}

SP_DP_MESSAGES createImageDataByPath(char* imagePath, SPImageData imageData){
	if (imageData->index < 0 || featuresMatrix[imageData->index] == NULL)
	{
		spLoggerPrintError(ERROR_ANALYZING_FEATURES, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FEATURE_EXTRACTION_ERROR;
	}
	imageData->featuresArray = featuresMatrix[imageData->index];//getImageFeatures(getImageFeatures, imageData->index, &(imageData->numOfFeatures));

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES loadKnownImageData(char* imageDataPath, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	FILE* imageFile;
	imageFile = fopen(imageDataPath, "r");
	if (imageFile == NULL) {
		spLoggerPrintError(ERROR_OPEN_FILE, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FILE_READ_ERROR;
	}

	message = loadImageDataFromFile(imageFile, imageData);

	fclose(imageFile);
	return message;
}

SP_DP_MESSAGES loadImageDataFromFile(FILE* imageFile, SPImageData imageData){
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	char *readResult, *line;

	line = (char*)calloc(sizeof(char),MAXLINE_LEN);
	if (line == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_MEMORY_FAILURE;
	}

	//read headers
	readResult = fgets(line, MAXLINE_LEN, imageFile);
	if (readResult == NULL)
	{
		spLoggerPrintError(ERROR_READING_FILE, __FILE__,__FUNCTION__, __LINE__);
		free(line);
		return SP_DP_FILE_READ_ERROR;
	}

	loadImageDataFromHeader(line, imageData);

	//read points
	imageData->featuresArray = (SPPoint*)calloc(sizeof(SPPoint),imageData->numOfFeatures);
	//allocation failure
	if (imageData->featuresArray == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		free(line);
		return SP_DP_MEMORY_FAILURE;
	}

	message = readFeaturesFromFile(imageFile, imageData);

	if (message != SP_DP_SUCCESS){
		free(imageData->featuresArray);
	}
	free(line);
	return message;
}

SPImageData loadImageDataByPath(char* imageDataPath,int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData resultImage = NULL;

	if (imageDataPath == NULL || message == NULL) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	resultImage = (SPImageData)malloc(sizeof(struct sp_image_data));

	if (resultImage == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_MEMORY_FAILURE;
		return NULL;
	}

	resultImage->index = imageIndex;

	if (createFlag){
		*message = createImageDataByPath(imageDataPath, resultImage);
	}
	else{
		*message = loadKnownImageData(imageDataPath, resultImage);
	}

	if (*message != SP_DP_SUCCESS){
		free(resultImage);
		resultImage = NULL;
	}

	return resultImage;
}

SPImageData loadImageData(const SPConfig config, int imageIndex, bool createFlag, SP_DP_MESSAGES* message){
	SPImageData image;
	char* filePath;

	if (config == NULL || imageIndex < 0) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENTS, __FILE__,__FUNCTION__, __LINE__);
		*message =  SP_DP_INVALID_ARGUMENT;
		return NULL;
	}

	filePath = getImagePath(config, imageIndex, createFlag, message);
	if (message == SP_DP_SUCCESS) {
		image = loadImageDataByPath(filePath,imageIndex, createFlag,  message);
	}

	free(filePath);
	return image;
}

SP_DP_MESSAGES writeImageDataToFile(FILE* imageFile, SPImageData imageData){
	assert(imageFile != NULL && imageData != NULL);
	SP_DP_MESSAGES message = SP_DP_SUCCESS;
	int i;
	char* line;
	line = getImageStringHeader(imageData, &message);

	if (line == NULL)
	{
		spLoggerPrintError(ERROR_GETTING_HEADER, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}
	fputs(line,imageFile);
	free(line);

	for (i=0;i<imageData->numOfFeatures;i++)
	{
		line = pointToString(imageData->featuresArray[i], &message);

		if (message != SP_DP_SUCCESS || line == NULL)
		{
			spLoggerPrintError(ERROR_TRANSLATING_POINT, __FILE__,__FUNCTION__, __LINE__);
			return SP_DP_INVALID_ARGUMENT;
		}
		fputs(line,imageFile);
		free(line);
	}

	return SP_DP_SUCCESS;
}

SP_DP_MESSAGES saveImageData(const SPConfig config, SPImageData imageData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	char* filePath;
	FILE* imageFile;

	if (config == NULL || imageData == NULL)
		return SP_DP_INVALID_ARGUMENT;

	filePath = getImagePath(config, imageData->index, true, &outputMessage);

	if (outputMessage != SP_DP_SUCCESS){
		free(filePath);
		spLoggerPrintError(ERROR_GETTING_PATH, __FILE__,__FUNCTION__, __LINE__);
		return outputMessage;
	}

	imageFile = fopen(filePath, "ab+"); //ab+ should create of not exist
	if (imageFile == NULL) {
		free(filePath);
		spLoggerPrintError(ERROR_OPEN_FILE, __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_FILE_WRITE_ERROR;
	}

	outputMessage = writeImageDataToFile(imageFile, imageData);

	fclose(imageFile);
	free(filePath);
	return outputMessage;
}

SP_DP_MESSAGES saveAllImagesData(const SPConfig config, SPImageData* imagesData){
	SP_DP_MESSAGES outputMessage = SP_DP_SUCCESS;
	SP_CONFIG_MSG configMessage;
	int i, numOfImages;

	if (config == NULL || imagesData == NULL)
		return SP_DP_INVALID_ARGUMENT;

	numOfImages = spConfigGetNumOfImages(config, &configMessage);

	if (configMessage != SP_CONFIG_SUCCESS){
		spLoggerPrintError(translateMessageToString(configMessage), __FILE__,__FUNCTION__, __LINE__);
		return SP_DP_INVALID_ARGUMENT;
	}

	for (i= 0 ; i < numOfImages; i++)
	{
		outputMessage = saveImageData(config, imagesData[i]);
		if (outputMessage != SP_DP_SUCCESS)
			return outputMessage;
	}

	return outputMessage;
}

void freeFeatures(SPPoint* features, int numOfFeatures){
	assert(numOfFeatures >= 0);
	int i;
	if (features != NULL) {
		for (i = 0 ; i<numOfFeatures;i++){
			spPointDestroy((features)[i]);
		}
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGE_DATA_POINTS_ARRAY_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

void freeImageData(SPImageData imageData){
	if (imageData != NULL){
		if (imageData->featuresArray != NULL) {
			freeFeatures(imageData->featuresArray,imageData->numOfFeatures);
			free(imageData->featuresArray);
			imageData->featuresArray = NULL;
		}
		else {
			spLoggerPrintWarning(WARNING_IMAGE_DATA_POINTS_ARRAY_NULL, __FILE__,__FUNCTION__, __LINE__);
		}
		free(imageData);
		imageData = NULL;
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGE_DATA_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

void freeAllImagesData(SPImageData* imagesData, int size){
	assert(size>=0);
	int i;
	if (imagesData != NULL){
		for (i=0;i<size;i++){
			freeImageData(imagesData[i]);
		}
		free(imagesData);
		imagesData = NULL;
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGES_DATA_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}