#ifndef SPIMAGEQUERY_H_
#define SPIMAGEQUERY_H_

#include "../image_parsing/SPImagesParser.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"

/*
 * Allocates a counterArray of size 'size' and initialize each cell in it to 0
 *
 * @param size - the size of the counter array
 * @returns -
 *  NULL if memory allocation failed, otherwise returns the initialized counter array
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* initializeCounterArray(int size);

/*
 * Pops the current minimal element from given priority queue 'bpq' (peek + dequeue)
 * and places its index in 'indicesArray'['arrayIndex']
 *
 * @param bpq - the priority queue to pop from
 * @param indicesArray - the array to place the index popped from queue in
 * @param arrayIndex - the index in 'indicesArray' to place the index popped from queue in
 *
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if 'bpq' is NULL
 * SP_BPQUEUE_EMPTY if the 'bpq' if empty
 * SP_BPQUEUE_SUCCESS if the element has been removed successfully
 */
SP_BPQUEUE_MSG popFromQueueToIndicesArray(SPBPQueue bpq, int* indicesArray,
		int arrayIndex);

/*
 * Creates an integer array containing the indices of the most similar images based
 * on the given priority queue 'bpq'
 *
 * @param bpq - the priority queue to pop from
 * @param originalQueueSize - the size of the queue before we started popping elems from it
 *
 * @returns NULL in case of memory failure or in case we are trying to pop from 'bpq'
 * and it's empty, else returns the desired indices array
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* createSimImagesToFeatureIndicesArray(SPBPQueue bpq, int originalQueueSize);

/*
 * Finds the indices of the images containing the features that are the most similar
 * to the given feature 'relevantFeature', creates an integer array containing them and
 * eventually returns it.
 *
 * @param relevantFeature - the feature we compare the elements in 'kdTree' to
 * @param kdTree - a KDTree instance representing the KDTree created from all the features
 * of all the images whose paths were given in the configuration file
 * @param bpq - a priority queue used to store the nearest features to the given feature
 * 'relevantFeature'
 * @param finalQueueSize - an integer pointer to contain the size of 'bpq' after filling it
 * in and before starting to pop from it
 *
 * @returns NULL in case of error in an internal function, otherwise returns the desired
 * indices array
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* getSimilarImagesIndicesToFeature(SPPoint relevantFeature, SPKDTreeNode kdTree,
		SPBPQueue bpq, int* finalQueueSize);

/*
 * Updates 'counterArray' according to the indices of the images containing the features
 * that are the most similar to the given feature 'relevantFeature'.
 *
 * @param relevantFeature - the feature we compare the elements in 'kdTree' to
 * @param kdTree - a KDTree instance representing the KDTree created from all the features
 * of all the images whose paths were given in the configuration file
 * @param bpq - a priority queue used to store the nearest features to the given feature
 * 'relevantFeature'
 * @param counterArray - the counter array to update
 *
 * @returns false in case of failure in one of the internal function, otherwise true
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
bool updateCounterArrayPerFeature(int* counterArray, SPPoint relevantFeature,
		SPKDTreeNode kdTree, SPBPQueue bpq);

/*
 * Returns an integer array of size 'retArraySize' containing the indices of 'counterArray'
 * with the maximum value
 *
 * @param counterArray - the counter array to work by
 * @param counterArraySize - the size of 'counterArray'
 * @param retArraySize - the size of the return array
 *
 * @returns NULL in case of memory allocation failure, otherwise returns the desired array
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* getTopItems(int* counterArray, int counterArraySize, int retArraySize);

/*
 * Returns an integer array of size 'numOfSimilarImages' containing the indices of
 * the images that contained the most features that were the most similar to the features
 * of the given image 'workingImage'
 *
 * @param workingImage - the SPImageData instance containing the features of the given
 * image
 * @param kdTree - a KDTree instance representing the KDTree created from all the features
 * of all the images whose paths were given in the configuration file
 * @param numOfImages - the number of images whose paths were given in the configuration
 * file
 * @param numOfSimilarImages - the size of the returned array
 * @param bpq - a priority queue used to store the nearest features to each feature of the
 * working image
 *
 * @returns NULL in case of failure in an internal function, otherwise returns the desired
 * array
 *
 * @logger - in case of any type of failure the relevant error is logged to the logger
 */
int* getSimilarImages(SPImageData workingImage, SPKDTreeNode kdTree, int numOfImages,
		int numOfSimilarImages, SPBPQueue bpq);


#endif /* SPIMAGEQUERY_H_ */
