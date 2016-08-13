#ifndef SPKDARRAY_H_
#define SPKDARRAY_H_

#include "SPPoint.h"

/*
 * A structure used to represent the KDArray,
 * contains indexes matrix, dim as 'd' and size as 'n'
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
 * The method initializes a new KDArray with the data given by arr
 *
 *@param arr - The array of points from which the KDArray is built
 *@param size - The number of points in 'arr'
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

// TODO - doc
void spKDArrayDestroy(SPKDArray kdArr);

// TODO - doc
void spKDArrayPairDestroy(SPKDArrayPair kdArrPair);

#endif /* SPKDARRAY_H_ */
