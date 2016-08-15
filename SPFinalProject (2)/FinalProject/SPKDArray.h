#ifndef SPKDARRAY_H_
#define SPKDARRAY_H_

#include "SPPoint.h"

// TODO - talk about logger

/*
 * A structure used to represent the KDArray,
 * contains indices matrix, dim as 'd' and size as 'n'
 */
struct sp_kd_array {
	SPPoint* pointsArray;
	int size;
	int dim;
	int** indicesMatrix;
};

/*
 * A pointer to the sp_kd_array structure
 */
typedef struct sp_kd_array* SPKDArray;


/*
 * A structure for representing a pair of KDArrays,
 * used for the result of the function 'Split'
 */
struct sp_kd_array_pair {
	SPKDArray kdLeft;
	SPKDArray kdRight;
} ;

/*
 * A pointer to the sp_kd_array_pair structure
 */
typedef struct sp_kd_array_pair* SPKDArrayPair;

/*
 * A pointer to the index_with_coor_value structure
 */
typedef struct index_with_coor_value* IndexWithCoorValue;

/*
 * The method is a comparator for IndexWithCoorValue array to be used in
 * the quick sort function
 *
 * @param a - pointer to the first element to be compared
 * @param b - pointer to the second element to be compared
 *
 * @returns 0 if the coordinate value of 'a' equals the coordinate value
 * of 'b', 1 if the coordinate value of 'a' is bigger than the coordinate
 * value of 'b', and -1 otherwise
 */
int compareIndexWithCoorValue(const void* a, const void* b);

/**
 * Frees all memory resources associated with indexWithCoorValueArr.
 * If indexWithCoorValueArr == NULL nothing is done.
 *
 * @param indexWithCoorValueArr - the IndexWithCoorValue array to destroy
 * @param size - the number of elements that were initiated in the array
 */
void indexWithCoorValueArrDestroy(IndexWithCoorValue*
		indexWithCoorValueArr, int size);

/*
 * The method validates that all params given to the init function of
 * SPKDArray are valid
 *
 * @param arr - The array of points from which the KDArray is built
 * @param size - The number of points in 'arr'
 *
 * @returns false if:
 *  - arr is NULL or
 *  - size <= 0 or
 *  - not all points in arr have the same dimension
 * otherwise returns true
 */
bool checkInitArgs(SPPoint* arr, int size);

/*
 * The method allocates an SPPoint array of size 'size' pointed by 'dst'
 * and copies all the points in the points array 'src' into it
 *
 * @param dst - a pointer to the destination SPPoint array
 * @param src - the source SPPoint array
 * @param size - the number of points to copy from src to *dst
 *
 * @returns false in case of memory allocation failure, otherwise
 * returns true
 */
bool copyPointsArr(SPPoint** dst, SPPoint* src, int size);

/*
 * The method allocates the indices matrix of the given SPKDArray 'arr',
 * according to its dimension and size.
 *
 * @param arr - the given SPKDArray to allocate its indices matrix
 *
 * @returns false in case of memory allocation failure, otherwise
 * returns true
 */
bool allocateKDArrayIndicesMatrix(SPKDArray arr);

/*
 * Destroys the given sp_kd_array structure instance and returns NULL
 *
 * @param arr - pointer to the sp_kd_array structure instance
 *
 * @returns NULL
 */
SPKDArray onErrorInInitOrCopy(SPKDArray arr);

/*
 * The method fills the given kd-array indices matrix
 *
 * @param arr - the given SPKDArray to fill its indices matrix
 *
 * @returns NULL in case of memory allocation failure, otherwise
 * returns arr after its indices matrix was filled
 */
SPKDArray fillIndicesMatrix(SPKDArray arr);

/*
 * The method initializes a new KDArray with the data given by arr
 *
 * @param arr - The array of points from which the KDArray is built
 * @param size - The number of points in 'arr'
 *
 * @returns -
 * NULL if
 *  - arr is NULL or
 *  - size <= 0 or
 *  - not all points in arr have the same dimension or
 *  - memory allocation failed
 * otherwise returns a new SPKDArray initialized according to the given data
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDArray Init(SPPoint* arr, int size);

/**
 * Allocates a copy of the given sp_kd_array structure instance.
 *
 * Given the sp_kd_array structure instance source, the functions returns
 * a new sp_kd_array structure instance 'ret' such that:
 * - dim(ret) = dim(source) (ret and source have the same dimension)
 * - size(ret) = size(source) (ret and source have the same size)
 * - every point in ret points array is a copy of the respective
 * point in source points array
 * - every entrance in the ret indices matrix is the same as the
 * repective entrance in source indices matrix
 *
 * @param source - The source sp_kd_array structure instance
 *
 * @returns
 * NULL in case source is null of memory allocation failure occurs
 * Otherwise a copy of source is returned.
 */
SPKDArray spKDArrayCopy(SPKDArray source);

/*
 * Frees the helper arrays, destroys the given sp_kd_array_pair structure
 * instance and returns NULL
 *
 * @param kdArrPair - pointer to the sp_kd_array_pair structure instance
 * @param xArr - the X array we use as a helper to the split function
 * @param map1 - an array containing the indices of the elements in
 * the left SPKDArray in kdArrPair
 * @param map2 - an array containing the indices of the elements in
 * the right SPKDArray in kdArrPair
 *
 * @returns NULL
 */
SPKDArrayPair onErrorInSplit(SPKDArrayPair kdArrPair, int* xArr,
		int* map1, int* map2);

/*
 * The method creates and fills the an array X, such that X[i] = 0
 * if kdArr->pointsArray[i] belongs to the left sub SPKDArray and
 * X[i] = 1 if kdArr->pointsArray[i] belongs to the right sub SPKDArray
 *
 * @param xArr - a pointer to the array to fill and return
 * @param kdArr - the given SPKDArray to split
 * @param medianIndex - [n/2] (upper crop) such that n is the number of
 * points in kdArr
 * @param coor - the relevant coordination to split by
 *
 * @returns false in case of memory allocation failure, otherwise
 * returns true
 */
bool initXArr(int** xArr, SPKDArray kdArr, int medianIndex, int coor);

/*
 * The method creates and fills the two kd-arrays points arrays
 * in kdArrPair, with respect to given kdArr and the array xArr
 *
 * @param kdArrPair - the given SPKDArrayPair to fill the points
 * arrays of its elements
 * @param kdArr - the given SPKDArray to split
 * @param xArr - the X array we use as a helper to the split function
 *
 * @returns false in case of memory allocation failure, otherwise
 * returns true
 */
bool createKDArrayPairPointsArrays(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr);

/*
 * The method creates and fills the two integer arrays, such that
 * map1 contains the indices of the elements in the left SPKDArray in
 * kdArrPair and map2 contains the indices of the elements in the right
 * SPKDArray in kdArrPair
 *
 * @param map1 - a pointer to an array to contain the indices of the
 * elements in the left KDArray in kdArrPair in the end of the function
 * @param map2 - a pointer to an array to contain the indices of the
 * elements in the right KDArray in kdArrPair in the end of the function
 * @param kdArr - the given SPKDArray to split
 * @param xArr - the X array we use as a helper to the split function
 *
 * @returns false in case of memory allocation failure, otherwise
 * returns true
 */
bool createMap1AndMap2(int** map1, int** map2, SPKDArray kdArr,
		int* xArr);

/*
 * The method creates and fills the two kd-arrays indices matrices
 * in kdArrPair, with respect to given kdArr, and the arrays: xArr, map1
 * and map2
 *
 * @param kdArrPair - the given SPKDArrayPair to fill the indices
 * matrices of its elements and eventually return
 * @param kdArr - the given SPKDArray to split
 * @param xArr - the X array we use as a helper to the split function
 * @param map1 - an array containing the indices of the elements in
 * the left SPKDArray in kdArrPair
 * @param map2 - an array containing the indices of the elements in
 * the right SPKDArray in kdArrPair
 *
 * @returns NULL in case of memory allocation failure, otherwise
 * returns kdArrPair after its kd-arrays indices matrices were filled
 */
SPKDArrayPair fillKDArrayPairIndicesMatrices(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr, int* map1, int* map2);

/*
 * The method calculates [size/2] (upper crop)
 * @param size - the size of the given points array
 *
 * @returns [size/2] (upper crop)
 */
int getMedianIndex(int size);

/*
 * The method returns two kd-arrays (using the SPKDArrayPair)
 * such that the first [n/2] (upper crop) points
 * with respect to the coordinate 'coor' are in kdLeft and the
 * rest of the points are in kdRight
 *
 * @param kdArr - the given SPKDArray to split
 * @param coor - the relevant coordination to split by
 *
 * @returns -
 * NULL if
 *  - kdArr is NULL or
 *  - coor < 0 or
 *  - memory allocation failed
 * otherwise:
 * if kdArr->size == 1 returns SPKDArrayPair that contains
 * pointer to kdLeft and sets kdRight to NULL
 * else returns SPKDArrayPair that contains
 * pointers to kdLeft and kdRight
 *
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDArrayPair Split(SPKDArray kdArr, int coor);

/**
 * Frees all memory resources associated with kdArr.
 * If kdArr == NULL nothing is done.
 *
 * @param kdArr - the SPKDArray instance to destroy
 */
void spKDArrayDestroy(SPKDArray kdArr);

/**
 * Frees all memory resources associated with kdArrPair.
 * If kdArrPair == NULL nothing is done.
 *
 * @param kdArrPair - the SPKDArrayPair instance to destroy
 */
void spKDArrayPairDestroy(SPKDArrayPair kdArrPair);

#endif /* SPKDARRAY_H_ */
