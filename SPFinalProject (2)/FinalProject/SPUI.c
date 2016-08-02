#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SPUI.h"
#include "SPLogger.h"
#include "SPImageQuery.h"

#define ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE "Enter a query image or # to terminate:\n"
#define EXITING "Exiting...\n"

#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"

char* getAsString(const char* message)
{
	char* response = (char*)calloc(1024, sizeof(char));
	if (response == NULL){
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,__FUNCTION__, __LINE__);
		return NULL;
	}
	printf("%s", message);
	fflush(NULL);
	scanf("%s", response);
	fflush(NULL);

	return response;
}

void presentSimilarImages(const SPConfig config, int* imagesIndexesArray, int imagesCount){
	//TODO - handle GUI
	int i = 0;
	printf("The closest images are: "); // TODO - export to macro
	fflush(NULL);
	for (i = 0; i < imagesCount; i++) {
		printf("%d%s", imagesIndexesArray[i], i == (imagesCount - 1) ? "\n" : ", ");
		fflush(NULL);
	}
}

void spUI_searchSimilarImages(const SPConfig config,SPImageData* imagesDatabase,SPImageData workingImage){
	int* similarImagesIndexes;
	int countOfSimilar;

	similarImagesIndexes = spIQ_getSimilarImages(config, imagesDatabase, workingImage, &countOfSimilar); //TODO - handle errors here
	if (similarImagesIndexes != NULL){
		presentSimilarImages(config, similarImagesIndexes, countOfSimilar);
		free(similarImagesIndexes);
	}
}


/*
void spUI_beginUserInteraction(const SPConfig config, SPImageData* imagesDatabase){
	// first run must always happen
	char* workingImagePath = getAsString(ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE);

	// iterating until the user inputs "#"
	while (workingImagePath != NULL && strcmp(workingImagePath, "#"))
	{
		spUI_searchSimilarImages(config, imagesDatabase, workingImagePath);
		free(workingImagePath);
		workingImagePath = getAsString(ENTER_A_QUERY_IMAGE_OR_TO_TERMINATE);
	}

	// free the memory allocated for the path string
	free(workingImagePath);

	// announce the user for exiting
	printf("%s", EXITING);
}*/
