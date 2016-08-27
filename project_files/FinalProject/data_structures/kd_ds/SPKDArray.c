#include <stdlib.h>
#include <stdbool.h>
#include "SPKDArray.h"
#include "../../SPLogger.h"

#define ERROR_INVALID_ARGUMENT	"Error Invalid argument"
#define ERROR_ALLOCATING_MEMORY "Could not allocate memory"
#define ERROR_POINT_COPY		"Error in copying point"

/*
 * A structure used to represent the index of a point in a given points
 * array and one of its coordinates value,
 * used to fill one row in a KDArray indices matrix
 */
struct index_with_coor_value {
	int index;
	double coor_value;
};

int compareIndexWithCoorValue(const void* a, const void* b) {
	IndexWithCoorValue* first = (IndexWithCoorValue *)a;
	IndexWithCoorValue* second = (IndexWithCoorValue *)b;
	//TODO - deal with epsilon issue
	return ((*first)->coor_value == (*second)->coor_value) ? 0 :
			((*first)->coor_value > (*second)->coor_value) ? 1 : -1;
}

void indexWithCoorValueArrDestroy(IndexWithCoorValue*
		indexWithCoorValueArr, int size) {
	int i;
	if (indexWithCoorValueArr != NULL) {
		for (i = 0; i < size; i++) {
			free(indexWithCoorValueArr[i]);
			indexWithCoorValueArr[i] = NULL;
		}
		free(indexWithCoorValueArr);
	}
}

bool checkInitArgs(SPPoint* arr, int size) {
	int i;

	if (!arr || size <= 0) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,
				__FUNCTION__, __LINE__);
		return false;
	}

	for (i = 0; i < size - 1; i++) {
		if (spPointGetDimension(arr[i]) !=
				spPointGetDimension(arr[i + 1])) {
			spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,
					__FUNCTION__, __LINE__);
			return false;
		}
	}

	return true;
}

bool copyPointsArr(SPPoint** dst, SPPoint* src, int size) {
	int i;
	if (!(*dst = (SPPoint*)calloc(size, sizeof(SPPoint)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
				__FUNCTION__, __LINE__);
		return false;
	}

	for (i = 0; i < size; i++) {
		if (!((*dst)[i] = spPointCopy(src[i]))) {
			spLoggerPrintError(ERROR_POINT_COPY, __FILE__,
							__FUNCTION__, __LINE__);
			return false;
		}
	}
	return true;
}

bool allocateKDArrayIndicesMatrix(SPKDArray arr) {
	int j;

	if (!(arr->indicesMatrix = (int**)calloc(arr->dim, sizeof(int *)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return false;
	}

	for (j = 0; j < arr->dim; j++) {
		if (!(arr->indicesMatrix[j] =
				(int*)calloc(arr->size, sizeof(int)))) {
			spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
							__FUNCTION__, __LINE__);
			return false;
		}
	}

	return true;
}

SPKDArray onErrorInInitOrCopy(SPKDArray arr) {
	spKDArrayDestroy(arr);
	return NULL;
}

SPKDArray fillIndicesMatrix(SPKDArray arr) {
	IndexWithCoorValue* indexWithCoorValueArr;
	int i, j;

	for (j = 0; j < arr->dim; j++) {
		// allocate memory for indexWithCoorValueArr
		if (!(indexWithCoorValueArr =
				(IndexWithCoorValue*)calloc(arr->size,
						sizeof(IndexWithCoorValue)))) {
			spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
							__FUNCTION__, __LINE__);
			return onErrorInInitOrCopy(arr);
		}

		// initialize indexWithCoorValueArr before sorting
		for (i = 0; i < arr->size; i++) {
			if (!(indexWithCoorValueArr[i] =
					(IndexWithCoorValue)
					calloc(1, sizeof(struct index_with_coor_value)))) {
				indexWithCoorValueArrDestroy(indexWithCoorValueArr, i);
				spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
								__FUNCTION__, __LINE__);
				return onErrorInInitOrCopy(arr);
			}
			indexWithCoorValueArr[i]->index = i;
			indexWithCoorValueArr[i]->coor_value =
					spPointGetAxisCoor(arr->pointsArray[i], j);
		}

		// sort indexWithCoorValueArr
		qsort(indexWithCoorValueArr, arr->size,
				sizeof(IndexWithCoorValue), compareIndexWithCoorValue);

		// fill arr->indicesMatrix according to sorted
		// indexWithCoorValueArr
		for (i = 0; i < arr->size; i++)
			arr->indicesMatrix[j][i] = indexWithCoorValueArr[i]->index;

		// destroy indexWithCoorValueArr
		indexWithCoorValueArrDestroy(indexWithCoorValueArr, arr->size);
	}

	return arr;
}

SPKDArray Init(SPPoint* arr, int size) {
	SPKDArray ret;

	if (!checkInitArgs(arr, size))
		return NULL;

	if (!(ret = (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return NULL;
	}

	ret->dim = spPointGetDimension(arr[0]);
	ret->size = size;

	if (!copyPointsArr(&(ret->pointsArray), arr, ret->size))
		return onErrorInInitOrCopy(ret);

	if (!allocateKDArrayIndicesMatrix(ret))
		return onErrorInInitOrCopy(ret);

	return fillIndicesMatrix(ret);
}

SPKDArray spKDArrayCopy(SPKDArray source) {
	SPKDArray ret;
	int i, j;

	if (!source) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if (!(ret = (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return NULL;
	}

	ret->dim = source->dim;
	ret->size = source->size;

	if (!copyPointsArr(&(ret->pointsArray), source->pointsArray,
			ret->size))
		onErrorInInitOrCopy(ret);

	if (!allocateKDArrayIndicesMatrix(ret))
		onErrorInInitOrCopy(ret);

	// copy all indicesMatrix elements from kdArr to ret
	for (j = 0; j < ret->dim; j++) {
		for (i = 0; i < ret->size; i++)
			ret->indicesMatrix[j][i] = source->indicesMatrix[j][i];
	}

	return ret;
}

SPKDArrayPair onErrorInSplit(SPKDArrayPair kdArrPair, int* xArr,
		int* map1, int* map2) {
	if (xArr)
		free(xArr);
	if (map1)
		free(map1);
	if (map2)
		free(map2);
	spKDArrayPairDestroy(kdArrPair);
	return NULL;
}

bool initXArr(int** xArr, SPKDArray kdArr, int leftKdArrSize, int coor) {
	int i;
	if (!(*xArr = (int*)calloc(kdArr->size, sizeof(int)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return false;
	}

	for (i = 0; i < leftKdArrSize; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 0;

	for (i = leftKdArrSize; i < kdArr->size; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 1;

	return true;
}

bool createKDArrayPairPointsArrays(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr) {
	int i, kdLeftIndex = 0, kdRightIndex = 0;

	if (!(kdArrPair->kdLeft->pointsArray =
		(SPPoint*)calloc(kdArrPair->kdLeft->size, sizeof(SPPoint)))
		|| !(kdArrPair->kdRight->pointsArray =
		(SPPoint*)calloc(kdArrPair->kdRight->size, sizeof(SPPoint)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return false;
	}

	for (i = 0; i < kdArr->size; i++) {
		if (xArr[i] == 0) {
			if (!(kdArrPair->kdLeft->pointsArray[kdLeftIndex++] =
					spPointCopy(kdArr->pointsArray[i]))) {
				spLoggerPrintError(ERROR_POINT_COPY, __FILE__,
								__FUNCTION__, __LINE__);
				return false;
			}
		}
		else {
			if (!(kdArrPair->kdRight->pointsArray[kdRightIndex++] =
					spPointCopy(kdArr->pointsArray[i]))) {
				spLoggerPrintError(ERROR_POINT_COPY, __FILE__,
								__FUNCTION__, __LINE__);
				return false;
			}
		}
	}

	return true;
}

bool createMap1AndMap2(int** map1, int** map2, SPKDArray kdArr,
		int* xArr) {
	int i, kdLeftIndex = 0,	kdRightIndex = 0;
	bool isFirstArrayindex;

	if (!(*map1 = (int*)calloc(kdArr->size, sizeof(int))) ||
			!(*map2 = (int*)calloc(kdArr->size, sizeof(int)))) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__,
						__FUNCTION__, __LINE__);
		return false;
	}

	for (i = 0; i < kdArr->size; i++) {
		isFirstArrayindex = (xArr[i] == 0);
		(*map1)[i] = isFirstArrayindex ? kdLeftIndex++ : -1;
		(*map2)[i] = isFirstArrayindex ? -1 : kdRightIndex++;
	}

	return true;
}

SPKDArrayPair fillKDArrayPairIndicesMatrices(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr, int* map1, int* map2) {
	int i, j, indexInOrig, kdLeftIndex, kdRightIndex;

	if (!allocateKDArrayIndicesMatrix(kdArrPair->kdLeft) ||
			!allocateKDArrayIndicesMatrix(kdArrPair->kdRight))
		return onErrorInSplit(kdArrPair, xArr, map1, map2);

	for (j = 0; j < kdArr->dim; j++) {
		kdLeftIndex = 0, kdRightIndex = 0;
		for (i = 0; i < kdArr->size; i++) {
			indexInOrig = kdArr->indicesMatrix[j][i];

			if (xArr[indexInOrig] == 0)
				kdArrPair->kdLeft->indicesMatrix[j][kdLeftIndex++] =
						map1[indexInOrig];

			else
				kdArrPair->kdRight->indicesMatrix[j][kdRightIndex++] =
						map2[indexInOrig];
		}
	}

	// TODO - decide if this should be here
	free(xArr);
	free(map1);
	free(map2);
	return kdArrPair;
}

SPKDArrayPair Split(SPKDArray kdArr, int coor) {
	SPKDArrayPair ret = NULL;
	int *xArr = NULL, *map1 = NULL, *map2 = NULL;

	if (!kdArr || coor < 0) {
		spLoggerPrintError(ERROR_INVALID_ARGUMENT, __FILE__,
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if (  !(ret =
			   (SPKDArrayPair)calloc(1, sizeof(struct sp_kd_array_pair)))  ) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return onErrorInSplit(ret, xArr, map1, map2);
	}

	// This condition should never be true in the way we build the KDTree, but just in case
	if (kdArr->size == 1) {
		if (!(ret->kdLeft = spKDArrayCopy(kdArr)))
			return onErrorInSplit(ret, xArr, map1, map2);
		ret->kdRight = NULL;
		return ret;
	}

	if (  !(ret->kdLeft =
				   (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))
	   || !(ret->kdRight =
			   	   (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))  ) {
		spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__);
		return onErrorInSplit(ret, xArr, map1, map2);
	}

	ret->kdLeft->dim = kdArr->dim;
	ret->kdRight->dim = kdArr->dim;
	ret->kdLeft->size = ((kdArr->size - 1) / 2) + 1; // +1 because we start from 0
	ret->kdRight->size = kdArr->size - ret->kdLeft->size;

	if (!initXArr(&xArr, kdArr, ret->kdLeft->size, coor))
		return onErrorInSplit(ret, xArr, map1, map2);

	if (!createKDArrayPairPointsArrays(ret, kdArr, xArr))
		return onErrorInSplit(ret, xArr, map1, map2);

	if (!createMap1AndMap2(&map1, &map2, kdArr, xArr))
		return onErrorInSplit(ret, xArr, map1, map2);

	return fillKDArrayPairIndicesMatrices(ret, kdArr, xArr, map1, map2);
}

void spKDArrayDestroy(SPKDArray kdArr) {
	int i, j;
	if (kdArr) {
		if (kdArr->pointsArray) {
			for (i = 0; i < kdArr->size; i++) {
				spPointDestroy(kdArr->pointsArray[i]);
			}
			free(kdArr->pointsArray);
			kdArr->pointsArray = NULL;
		}

		if (kdArr->indicesMatrix) {
			for (j = 0; j < kdArr->dim; j++) {
				if (kdArr->indicesMatrix[j]) {
					free(kdArr->indicesMatrix[j]);
					kdArr->indicesMatrix[j] = NULL;
				}
			}
			free(kdArr->indicesMatrix);
			kdArr->indicesMatrix = NULL;
		}
		free(kdArr);
	}
}

void spKDArrayPairDestroy(SPKDArrayPair kdArrPair) {
	if (kdArrPair) {
		if (kdArrPair->kdLeft) {
			spKDArrayDestroy(kdArrPair->kdLeft);
			kdArrPair->kdLeft = NULL;
		}
		if (kdArrPair->kdRight) {
			spKDArrayDestroy(kdArrPair->kdRight);
			kdArrPair->kdRight = NULL;
		}
		free(kdArrPair);
	}
}

