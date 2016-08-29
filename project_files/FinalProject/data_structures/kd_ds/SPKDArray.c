#include <stdlib.h>
#include <stdbool.h>
#include "SPKDArray.h"
#include "../../SPLogger.h"
#include "../../general_utils/SPUtils.h"

#define ERROR_INITIALIZING_KD_ARRAY		"Error initializing KD Array"
#define ERROR_AT_COPYING_POINTS_ARRAY 	"Could not copy points array"
#define ERROR_ALLOCATING_INDICES		"Could not allocate indices matrix for KD Array"
#define ERROR_FILLING_INDICES			"Could not fill indices matrix for KD Array"
#define ERROR_COPYING_KD_ARRAY			"Error at copying KD array"
#define ERROR_PAIR_INDICES_MATRIX		"Error filling KD pair indices matrix"
#define ERROR_SPLITTING_KD_ARRAY		"Error when trying to split KD array"

#define WARNING_ARR_SIZE_1 				"KDArray of size 1 was passed to Split function"
#define WARNING_INVALID_ARGUMENT		"Invalid argument in an inner function"
#define WARNING_KDARR_NULL    			"KDArray object is null when destroy is called"
#define WARNING_KDARR_PAIR_NULL    		"KDArrayPair object is null when destroy is called"
#define WARNING_KDARR_PNTS_ARR_NULL		"KDArray points array is null when destroy is called"
#define WARNING_KDARR_INDS_MAT_NULL		"KDArray indices matrix is null when destroy is called"
#define WARNING_KDARR_MAT_LINE_NULL		"KDArray matrix line is null when destroy is called"
#define WARNING_IDX_COOR_ARR_NULL		"Index_with_coor array is null when destroy is called"
#define WARNING_IDX_COOR_OBJ_NULL		"Index_with_coor object is null when destroy is called"

/*
 * A structure used to represent the index of a point in a given points
 * array and one of its coordinates value,
 * used to fill one row in a KDArray indices matrix
 */
typedef struct index_with_coor_value {
	int index;
	double coor_value;
} index_with_coor_value;

int compareIndexWithCoorValue(const void* a, const void* b) {
	IndexWithCoorValue* first = (IndexWithCoorValue *)a;
	IndexWithCoorValue* second = (IndexWithCoorValue *)b;
	return isEqual((*first)->coor_value, (*second)->coor_value) ? 0 :
			((*first)->coor_value > (*second)->coor_value) ? 1 : -1;
}

void indexWithCoorValueArrDestroy(IndexWithCoorValue* indexWithCoorValueArr, int size) {
	int i;
	if (indexWithCoorValueArr != NULL) {
		for (i = 0; i < size; i++) {
			spSafeFree(indexWithCoorValueArr[i], WARNING_IDX_COOR_OBJ_NULL);
		}
		free(indexWithCoorValueArr);
	}
	else {
		spLoggerPrintWarning(WARNING_IDX_COOR_ARR_NULL, __FILE__, __FUNCTION__, __LINE__);
	}
}

bool checkInitArgs(SPPoint* arr, int size) {
	int i;
	spVerifyArguments(arr && size > 0, ERROR_INITIALIZING_KD_ARRAY, false);

	for (i = 0; i < size - 1; i++) {
		spVerifyArguments(spPointGetDimension(arr[i]) == spPointGetDimension(arr[i + 1]),
				ERROR_INITIALIZING_KD_ARRAY, false);
	}

	return true;
}

bool copyPointsArr(SPPoint** dst, SPPoint* src, int size) {
	int i;

	spVerifyArguments(dst && src && size > 0,
			ERROR_AT_COPYING_POINTS_ARRAY, false);

	spCallocWr((*dst), SPPoint, size, false);

	for (i = 0; i < size; i++)
		(*dst)[i] = src[i];

	return true;
}

bool allocateKDArrayIndicesMatrix(SPKDArray arr) {
	int j;
	spVerifyArguments(arr, ERROR_ALLOCATING_INDICES, false);

	spCallocWr((arr->indicesMatrix), int* ,arr->dim,  false);

	for (j = 0; j < arr->dim; j++) {
		spCallocWr((arr->indicesMatrix[j]), int, arr->size, false);
	}

	return true;
}

SPKDArray onErrorInInitOrCopy(SPKDArray arr) {
	spKDArrayDestroy(arr);
	return NULL;
}

SPKDArray fillIndicesMatrix(SPKDArray arr) {
	IndexWithCoorValue* indexWithCoorValueArr = NULL;
	int i, j;
	spVerifyArguments(arr, ERROR_FILLING_INDICES, false);

	for (j = 0; j < arr->dim; j++) {
		// allocate memory for indexWithCoorValueArr
		spCallocEr(indexWithCoorValueArr, IndexWithCoorValue, arr->size,
				ERROR_FILLING_INDICES, onErrorInInitOrCopy(arr));

		// initialize indexWithCoorValueArr before sorting
		for (i = 0; i < arr->size; i++) {
			spCallocEr(indexWithCoorValueArr[i], index_with_coor_value, 1,
							ERROR_FILLING_INDICES, onErrorInInitOrCopy(arr));

			indexWithCoorValueArr[i]->index = i;
			indexWithCoorValueArr[i]->coor_value =
					spPointGetAxisCoor(arr->pointsArray[i], j);
		}

		// sort indexWithCoorValueArr
		qsort(indexWithCoorValueArr, arr->size, sizeof(IndexWithCoorValue),
				compareIndexWithCoorValue);

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

	spCalloc(ret, sp_kd_array, 1);

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
	spVerifyArgumentsRn(source, ERROR_COPYING_KD_ARRAY);

	spCalloc(ret, sp_kd_array, 1);

	ret->dim = source->dim;
	ret->size = source->size;

	if (!copyPointsArr(&(ret->pointsArray), source->pointsArray, ret->size))
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
	spCallocWr((*xArr), int, kdArr->size, false);

	for (i = 0; i < leftKdArrSize; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 0;

	for (i = leftKdArrSize; i < kdArr->size; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 1;

	return true;
}

bool createKDArrayPairPointsArrays(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr) {
	int i, kdLeftIndex = 0, kdRightIndex = 0;

	spCallocWr(kdArrPair->kdLeft->pointsArray, SPPoint, kdArrPair->kdLeft->size, false);
	spCallocWr(kdArrPair->kdRight->pointsArray, SPPoint, kdArrPair->kdRight->size, false);

	for (i = 0; i < kdArr->size; i++) {
		if (xArr[i] == 0)
			kdArrPair->kdLeft->pointsArray[kdLeftIndex++] = kdArr->pointsArray[i];
		else
			kdArrPair->kdRight->pointsArray[kdRightIndex++] = kdArr->pointsArray[i];
	}

	return true;
}

bool createMap1AndMap2(int** map1, int** map2, SPKDArray kdArr,
		int* xArr) {
	int i, kdLeftIndex = 0,	kdRightIndex = 0;
	bool isFirstArrayindex;

	spCallocWr(*map1, int, kdArr->size, false);
	spCallocWr(*map2, int, kdArr->size, false);

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

	spValRCb(allocateKDArrayIndicesMatrix(kdArrPair->kdLeft) && allocateKDArrayIndicesMatrix(kdArrPair->kdRight),
			ERROR_PAIR_INDICES_MATRIX, onErrorInSplit(kdArrPair, xArr, map1, map2));

	for (j = 0; j < kdArr->dim; j++) {
		kdLeftIndex = 0, kdRightIndex = 0;
		for (i = 0; i < kdArr->size; i++) {
			indexInOrig = kdArr->indicesMatrix[j][i];

			if (xArr[indexInOrig] == 0)
				kdArrPair->kdLeft->indicesMatrix[j][kdLeftIndex++] = map1[indexInOrig];

			else
				kdArrPair->kdRight->indicesMatrix[j][kdRightIndex++] = map2[indexInOrig];
		}
	}

	free(xArr);
	free(map1);
	free(map2);
	return kdArrPair;
}

SPKDArrayPair Split(SPKDArray kdArr, int coor) {
	SPKDArrayPair ret = NULL;
	int *xArr = NULL, *map1 = NULL, *map2 = NULL;

	spVerifyArgumentsRn(kdArr && coor>=0, ERROR_SPLITTING_KD_ARRAY);

	spCallocEr(ret, sp_kd_array_pair, 1, ERROR_SPLITTING_KD_ARRAY,
			onErrorInSplit(ret, xArr, map1, map2));

	// This condition should never be true in the way we build the KDTree, but just in case
	if (kdArr->size == 1) {
		spLoggerPrintWarning(WARNING_ARR_SIZE_1, __FILE__, __FUNCTION__, __LINE__);

		spValRCb((ret->kdLeft = spKDArrayCopy(kdArr)),
				ERROR_SPLITTING_KD_ARRAY, onErrorInSplit(ret, xArr, map1, map2));

		ret->kdRight = NULL;
		return ret;
	}

	spCallocEr(ret->kdLeft, sp_kd_array, 1, ERROR_SPLITTING_KD_ARRAY,
			onErrorInSplit(ret, xArr, map1, map2));

	spCallocEr(ret->kdRight, sp_kd_array, 1, ERROR_SPLITTING_KD_ARRAY,
			onErrorInSplit(ret, xArr, map1, map2));

	ret->kdLeft->dim = kdArr->dim;
	ret->kdRight->dim = kdArr->dim;
	ret->kdLeft->size = ((kdArr->size - 1) / 2) + 1; // +1 because we start from 0
	ret->kdRight->size = kdArr->size - ret->kdLeft->size;

	spValRCb((initXArr(&xArr, kdArr, ret->kdLeft->size, coor)),
					ERROR_SPLITTING_KD_ARRAY, onErrorInSplit(ret, xArr, map1, map2));

	spValRCb((createKDArrayPairPointsArrays(ret, kdArr, xArr)),
					ERROR_SPLITTING_KD_ARRAY, onErrorInSplit(ret, xArr, map1, map2));

	spValRCb((createMap1AndMap2(&map1, &map2, kdArr, xArr)),
					ERROR_SPLITTING_KD_ARRAY, onErrorInSplit(ret, xArr, map1, map2));

	return fillKDArrayPairIndicesMatrices(ret, kdArr, xArr, map1, map2);
}

void spKDArrayDestroy(SPKDArray kdArr) {
	int j;
	if (kdArr) {
		spSafeFree(kdArr->pointsArray, WARNING_KDARR_PNTS_ARR_NULL);

		if (kdArr->indicesMatrix) {
			for (j = 0; j < kdArr->dim; j++) {
				spSafeFree(kdArr->indicesMatrix[j], WARNING_KDARR_MAT_LINE_NULL);
			}
			free(kdArr->indicesMatrix);
			kdArr->indicesMatrix = NULL;
		}
		else {
			spLoggerPrintWarning(WARNING_KDARR_INDS_MAT_NULL, __FILE__, __FUNCTION__,
								__LINE__);
		}
		free(kdArr);
	}
	else {
		spLoggerPrintWarning(WARNING_KDARR_NULL, __FILE__, __FUNCTION__, __LINE__);
	}
}

void spKDArrayPairDestroy(SPKDArrayPair kdArrPair) {
	if (kdArrPair) {
		spKDArrayDestroy(kdArrPair->kdLeft);
		kdArrPair->kdLeft = NULL;
		spKDArrayDestroy(kdArrPair->kdRight);
		kdArrPair->kdRight = NULL;
		free(kdArrPair);
	}
	else {
		spLoggerPrintWarning(WARNING_KDARR_PAIR_NULL, __FILE__, __FUNCTION__, __LINE__);
	}
}

