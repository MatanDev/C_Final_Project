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

int* spIQ_getSimilarImages(SPConfig config,SPImageData* imagesDatabase,SPImageData workingImage,int countOfSimilar);


#endif /* SPIMAGEQUERY_H_ */
