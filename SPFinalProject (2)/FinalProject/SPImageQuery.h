/*
 * SPImageQuery.h
 *
 *  Created on: 2 баев 2016
 *      Author: Matan
 */

#ifndef SPIMAGEQUERY_H_
#define SPIMAGEQUERY_H_


#include "SPConfig.h"
#include "SPImagesParser.h"

int* spIQ_getSimilarImages(SPImageData* imagesDatabase,SPImageData workingImage,int countOfSimilar, int numOfImages, int knn);


#endif /* SPIMAGEQUERY_H_ */
