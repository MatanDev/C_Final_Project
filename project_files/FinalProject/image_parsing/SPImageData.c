#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "../SPLogger.h"
#include "../general_utils/SPUtils.h"

#include "SPImageData.h"


#define WARNING_IMAGE_DATA_POINTS_ARRAY_NULL       "Image data array is null when free is called"
#define WARNING_IMAGE_DATA_NULL                    "Image data object is null when free is called"
#define WARNING_IMAGES_DATA_NULL                   "Images data object is null when free is called"
#define WARNING_IMAGES_DATA_NULL_ON_RESET		   "Images data object is null when reset is called"

SPImageData createImageData(int index){
	SPImageData image = NULL;
	spCalloc(image, sp_image_data, 1);
	image->index = index;
	return image;
}

void freeFeatures(SPPoint* features, int numOfFeatures){
	assert(numOfFeatures >= 0);
	int i;
	if (features != NULL) {
		for (i = 0 ; i<numOfFeatures;i++){
			spPointDestroy(features[i]);
		}
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGE_DATA_POINTS_ARRAY_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

void freeImageData(SPImageData imageData, bool suppressFeaturesArrayWarning, bool freeInternalFeatures){
	if (imageData != NULL){
		if (imageData->featuresArray != NULL) {
			if (freeInternalFeatures)
				freeFeatures(imageData->featuresArray,imageData->numOfFeatures);
			free(imageData->featuresArray);
			imageData->featuresArray = NULL;
		}
		else {
			if (!suppressFeaturesArrayWarning)
				spLoggerPrintWarning(WARNING_IMAGE_DATA_POINTS_ARRAY_NULL, __FILE__,__FUNCTION__, __LINE__);
		}
		free(imageData);
		imageData = NULL;
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGE_DATA_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

void freeAllImagesData(SPImageData* imagesData, int size, bool freeInternalFeatures){
	assert(size>=0);
	int i;
	if (imagesData != NULL){
		for (i=0;i<size;i++){
			freeImageData(imagesData[i], false, freeInternalFeatures);
		}
		free(imagesData);
		imagesData = NULL;
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGES_DATA_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

void resetImageData(SPImageData image){
	if (image != NULL){
		if (image->featuresArray != NULL) {
			freeFeatures(image->featuresArray,image->numOfFeatures);
			free(image->featuresArray);
		}
		image->numOfFeatures = 0;
	}
	else {
		spLoggerPrintWarning(WARNING_IMAGES_DATA_NULL_ON_RESET, __FILE__,__FUNCTION__, __LINE__);
	}
}
