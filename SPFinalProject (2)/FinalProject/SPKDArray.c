#include <stdlib.h>
#include <stdbool.h>
#include "SPKDArray.h"

struct index_with_coor_value {
	int index;
	double coor_value;
};

typedef struct index_with_coor_value* IndexWithCoorValue;

int compareIndexWithCoorValue(const void* a, const void* b) {
	IndexWithCoorValue* first = (IndexWithCoorValue *)a;
	IndexWithCoorValue* second = (IndexWithCoorValue *)b;
	//TODO - deal with epsilon issue
	//TODO - validate what happens in case of equality
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

	if (!arr || size <= 0)
		return false;

	for (i = 0; i < size - 1; i++) {
		if (spPointGetDimension(arr[i]) !=
				spPointGetDimension(arr[i + 1]))
			return false;
	}

	return true;
}

bool copyPointsArr(SPPoint** dst, SPPoint* src, int size) {
	int i;
	if (!(*dst = (SPPoint*)calloc(size, sizeof(SPPoint))))
		return false;

	for (i = 0; i < size; i++) {
		if (!((*dst)[i] = spPointCopy(src[i])))
			return false;
	}
	return true;
}

bool initializeKDArrayIndicesMatrix(SPKDArray arr) {
	int j;

	if (!(arr->indicesMatrix = (int**)calloc(arr->dim, sizeof(int *))))
		return false;

	for (j = 0; j < arr->dim; j++) {
		if (!(arr->indicesMatrix[j] =
				(int*)calloc(arr->size, sizeof(int))))
			return false;
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
						sizeof(IndexWithCoorValue))))
			return onErrorInInitOrCopy(arr);

		// initialize indexWithCoorValueArr before sorting
		for (i = 0; i < arr->size; i++) {
			if (!(indexWithCoorValueArr[i] =
					(IndexWithCoorValue)
					calloc(1, sizeof(struct index_with_coor_value)))) {
				indexWithCoorValueArrDestroy(indexWithCoorValueArr, i);
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

	if (!(ret = (SPKDArray)calloc(1, sizeof(struct sp_kd_array))))
		return NULL;

	ret->dim = spPointGetDimension(arr[0]);
	ret->size = size;

	if (!copyPointsArr(&(ret->pointsArray), arr, ret->size))
		return onErrorInInitOrCopy(ret);

	if (!initializeKDArrayIndicesMatrix(ret))
		return onErrorInInitOrCopy(ret);

	return fillIndicesMatrix(ret);
}

SPKDArray Copy(SPKDArray kdArr) {
	SPKDArray ret;
	int i, j;

	if (kdArr == NULL)
		return NULL;

	if (!(ret = (SPKDArray)calloc(1, sizeof(struct sp_kd_array))))
		return NULL;

	ret->dim = kdArr->dim;
	ret->size = kdArr->size;

	if (!copyPointsArr(&(ret->pointsArray), kdArr->pointsArray,
			ret->size))
		onErrorInInitOrCopy(ret);

	if (!initializeKDArrayIndicesMatrix(ret))
		onErrorInInitOrCopy(ret);

	// copy all indicesMatrix elements from kdArr to ret
	for (j = 0; j < ret->dim; j++) {
		for (i = 0; i < ret->size; i++)
			ret->indicesMatrix[j][i] = kdArr->indicesMatrix[j][i];
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

bool initXArr(int** xArr, SPKDArray kdArr, int median, int coor) {
	int i = 0;
	if (!(*xArr = (int*)calloc(kdArr->size, sizeof(int))))
		return false;

	for (i = 0; i < median; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 0;

	for (i = median; i < kdArr->size; i++)
		(*xArr)[kdArr->indicesMatrix[coor][i]] = 1;

	return true;
}

bool createKDArrayPairPointsArrays(SPKDArrayPair kdArrPair,
		SPKDArray kdArr, int* xArr) {
	int i, kdLeftIndex = 0, kdRightIndex = 0;

	if (!(kdArrPair->kdLeft->pointsArray =
			(SPPoint*)calloc(kdArrPair->kdLeft->size, sizeof(SPPoint)))
			|| !(kdArrPair->kdRight->pointsArray =
			(SPPoint*)calloc(kdArrPair->kdRight->size, sizeof(SPPoint))))
		return false;

	for (i = 0; i < kdArr->size; i++) {
		if (xArr[i] == 0) {
			if (!(kdArrPair->kdLeft->pointsArray[kdLeftIndex++] =
					spPointCopy(kdArr->pointsArray[i])))
				return false;
		}
		else {
			if (!(kdArrPair->kdRight->pointsArray[kdRightIndex++] =
					spPointCopy(kdArr->pointsArray[i])))
				return false;
		}
	}

	return true;
}

bool createMap1AndMap2(int** map1, int** map2, SPKDArray kdArr,
		int* xArr) {
	int i, kdLeftIndex = 0,	kdRightIndex = 0;
	bool isFirstArrayindex;

	if (!(*map1 = (int*)calloc(kdArr->size, sizeof(int))) ||
			!(*map2 = (int*)calloc(kdArr->size, sizeof(int))))
		return false;

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

	if (!initializeKDArrayIndicesMatrix(kdArrPair->kdLeft) ||
			!initializeKDArrayIndicesMatrix(kdArrPair->kdRight))
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

	return kdArrPair;
}

SPKDArrayPair Split(SPKDArray kdArr, int coor) {
	SPKDArrayPair ret = NULL;
	int median;
	int *xArr = NULL, *map1 = NULL, *map2 = NULL;

	if (!kdArr || coor < 0)
		return NULL;

	if (  !(ret =
			   (SPKDArrayPair)calloc(1, sizeof(struct sp_kd_array_pair)))
	   || !(ret->kdLeft =
			   (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))
	   || !(ret->kdRight =
			   (SPKDArray)calloc(1, sizeof(struct sp_kd_array)))  )
		return onErrorInSplit(ret, xArr, map1, map2);

	if (kdArr->size == 1) {
		ret->kdLeft = Copy(kdArr);
		ret->kdRight = NULL;
		return ret;
	}

	median = (kdArr->size % 2 == 0) ?
			(kdArr->size / 2) : (kdArr->size / 2) + 1;

	ret->kdLeft->dim = kdArr->dim;
	ret->kdRight->dim = kdArr->dim;
	ret->kdLeft->size = median;
	ret->kdRight->size = kdArr->size - median;

	if (!initXArr(&xArr, kdArr, median, coor))
		return onErrorInSplit(ret, xArr, map1, map2);

	if (!createKDArrayPairPointsArrays(ret, kdArr, xArr))
		return onErrorInSplit(ret, xArr, map1, map2);

	if (!createMap1AndMap2(&map1, &map2, kdArr, xArr))
		return onErrorInSplit(ret, xArr, map1, map2);

	if (!initializeKDArrayIndicesMatrix(ret->kdLeft) ||
			!initializeKDArrayIndicesMatrix(ret->kdRight))
		return onErrorInSplit(ret, xArr, map1, map2);

	return fillKDArrayPairIndicesMatrices(ret, kdArr, xArr, map1, map2);
}

void spKDArrayDestroy(SPKDArray kdArr) {
	int i, j;
	if (!kdArr) {
		if (!(kdArr->pointsArray)) {
			for (i = 0; i < kdArr->size; i++)
				spPointDestroy(kdArr->pointsArray[i]);
		}

		if (!(kdArr->indicesMatrix)) {
			for (j = 0; j < kdArr->dim; j++) {
				if (!(kdArr->indicesMatrix[j])) {
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
	if (!kdArrPair) {
		if (!(kdArrPair->kdLeft)) {
			spKDArrayDestroy(kdArrPair->kdLeft);
			kdArrPair->kdLeft = NULL;
		}
		if (!(kdArrPair->kdRight)) {
			spKDArrayDestroy(kdArrPair->kdRight);
			kdArrPair->kdRight = NULL;
		}
		free(kdArrPair);
	}
}

