#ifndef SPIMAGEDATA_H_
#define SPIMAGEDATA_H_

#include "../SPPoint.h"
#include <stdbool.h>

/** A type used for defining an ImageData item**/
typedef struct sp_image_data {
	int index;
	int numOfFeatures;
	SPPoint* featuresArray;
} sp_image_data;

/** A type used for defining an ImageData item as a pointer**/
typedef struct sp_image_data* SPImageData;

/*
 * The method creates a new image data, with a given index
 * @param index - the image index
 *
 * @returns
 * NULL in case of memory allocation failure
 * otherwise an ImageData item with the given index
 *
 * @logger - logs the relevant error if needed
 */
SPImageData createImageData(int index);

/*
 * Deallocates a number of features from a given features array
 *
 * @param features - the features array
 * @param numOfFeatures - the number of features
 *
 * @assert numOfFeatures >= 0
 *  @logger - prints a warning if images data is null
 */
void freeFeatures(SPPoint* features, int numOfFeatures);

/*
 * Deallocates an image data item
 *
 * @param imageData - the image data to destroy
 * @param suppressFeaturesArrayWarning - indicates if imageData->features = NULL should cause a warning
 * @param freeInternalFeatures - indicates if the internal features (SPPoints) should be destroyed too
 *
 * @logger - prints a warning if image data is null
 */
void freeImageData(SPImageData imageData, bool suppressFeaturesArrayWarning, bool freeInternalFeatures);

/*
 * Deallocates an images data items array
 *
 * @param imagesData - the images data array that should be destryed
 * @param size - the size of the array
 * @param freeInternalFeatures - indicates if the internal features (SPPoints) should be destroyed too
 *
 * @assert size >= 0
 *
 * @logger - prints a warning if images data is null
 */
void freeAllImagesData(SPImageData* imagesData, int size, bool freeInternalFeatures);

/*
 * Resets an image data, free's its features and sets its number of features to 0.
 *
 * @param image - the image to reset
 *
 * @logger - reports warning if image is null
 */
void resetImageData(SPImageData image);

#endif /* SPIMAGEDATA_H_ */
