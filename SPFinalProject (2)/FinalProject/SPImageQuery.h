#ifndef SPIMAGEQUERY_H_
#define SPIMAGEQUERY_H_


#include "SPConfig.h"
#include "SPImagesParser.h"
#include "SPBPriorityQueue.h"
#include "SPKDTreeNode.h"

int* spIQ_getSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq);


#endif /* SPIMAGEQUERY_H_ */
