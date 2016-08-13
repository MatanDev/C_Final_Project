#include <stdlib.h>
#include "SPKDArray.h"
//#include "SPPoint.h"

struct index_with_coor_value {
	int index;
	double coor_value;
};

typedef struct index_with_coor_value* IndexWithCoorValue;

int compareIndexWithCoorValue(const void* a, const void* b) {
	IndexWithCoorValue* first = (IndexWithCoorValue *)a;
	IndexWithCoorValue* second = (IndexWithCoorValue *)b;
	//TODO - deal with problem with double...
	return (*first)->coor_value - (*second)->coor_value;
}

SPKDArray Init(SPPoint* arr, int size) {
	SPKDArray ret;
	int i, j;
	if (!arr)
		return NULL; //error
	if (size <= 0)
		return NULL; //error
	for (i = 0; i < size - 1; i++) {
		if (spPointGetDimension(arr[i]) !=
				spPointGetDimension(arr[i + 1]))
			return NULL; //error
	}
	ret = (SPKDArray)malloc(sizeof(struct sp_kd_array));
	if (!ret)
		return NULL; //error
	ret->dim = spPointGetDimension(arr[0]);
	ret->size = size;
	ret->pointsArray = (SPPoint*)calloc(ret->size, sizeof(SPPoint));
	if (!(ret->pointsArray)) {
		// TODO - cleanup
		return NULL;
	}
	for (i = 0; i < ret->size; i++) {
		// TODO - validate if copy does not return NULL
		ret->pointsArray[i] = spPointCopy(arr[i]);
	}
	ret->indicesMatrix = (int**)calloc(ret->dim, sizeof(int *));
	if (!(ret->indicesMatrix)) {
		free(ret);
		return NULL; //error
	}
	for (j = 0; j < ret->dim; j++) {
		ret->indicesMatrix[j] = (int*)calloc(ret->size, sizeof(int));
		if (!(ret->indicesMatrix[j])) {
			// need to free all previous arrays - export to function
			free(ret->indicesMatrix);
			free(ret);
			return NULL; //error
		}
		// TODO - validate
		IndexWithCoorValue* indexWithCoorValueArr =
				(IndexWithCoorValue*)calloc(ret->size,
						sizeof(IndexWithCoorValue));

		if (!indexWithCoorValueArr) {
			// need to free all previous arrays (including this one)
			// - export to function
			free(ret->indicesMatrix);
			free(ret);
			return NULL; //error
		}

		// initialize array
		for (i = 0; i < ret->size; i++) {
			indexWithCoorValueArr[i] =
					(IndexWithCoorValue)
					malloc(sizeof(struct index_with_coor_value));
			if (!indexWithCoorValueArr[i]) {
				//TODO - cleanup
				return NULL;
			}
			indexWithCoorValueArr[i]->index = i;
			indexWithCoorValueArr[i]->coor_value =
					spPointGetAxisCoor(arr[i], j);
		}

		qsort(indexWithCoorValueArr, ret->size,
				sizeof(IndexWithCoorValue), compareIndexWithCoorValue);

		for (i = 0; i < ret->size; i++)
			ret->indicesMatrix[j][i] = indexWithCoorValueArr[i]->index;

		free(indexWithCoorValueArr);
		//TODO - cleanup indexWithCoorValueArr[i] for all i
	}
	return ret;
}

SPKDArray Copy(SPKDArray kdArr) {
	SPKDArray ret;
	int i, j;
	if (kdArr == NULL)
		return NULL;
	ret = (SPKDArray)malloc(sizeof(struct sp_kd_array));
	if (!ret)
		return NULL;
	ret->dim = kdArr->dim;
	ret->size = kdArr->size;
	ret->pointsArray = (SPPoint*)calloc(ret->size, sizeof(SPPoint));
	if (!(ret->pointsArray)) {
		// TODO - cleanup
		return NULL;
	}
	for (i = 0; i < ret->size; i++) {
		// TODO - validate if copy does not return NULL
		ret->pointsArray[i] = spPointCopy(kdArr->pointsArray[i]);
	}
	ret->indicesMatrix = (int**)calloc(ret->dim, sizeof(int *));
	if (!(ret->indicesMatrix)) {
		free(ret);
		return NULL; //error
	}
	for (j = 0; j < ret->dim; j++) {
		ret->indicesMatrix[j] = (int*)calloc(ret->size, sizeof(int));
		if (!(ret->indicesMatrix[j])) {
			// need to free all previous arrays - export to function
			free(ret->indicesMatrix);
			free(ret);
			return NULL; //error
		}
		for (i = 0; i < ret->size; i++) {
			ret->indicesMatrix[j][i] = kdArr->indicesMatrix[j][i];
		}
	}
	return ret;
}

SPKDArrayPair Split(SPKDArray kdArr, int coor) {
	SPKDArrayPair ret;
	int median, i, indexInOrigArr, j, kdLeftIndex = 0, kdRightIndex = 0;
	int *xArr, *map1, *map2;

	if (!kdArr)
		return NULL; //error
	if (coor < 0)
		return NULL; //error
	ret = (SPKDArrayPair)malloc(sizeof(struct sp_kd_array_pair));
	if (!ret)
		return NULL; //error
	ret->kdLeft = (SPKDArray)malloc(sizeof(struct sp_kd_array));
	if (!(ret->kdLeft)) {
		free(ret);
		return NULL; //error
	}
	ret->kdRight = (SPKDArray)malloc(sizeof(struct sp_kd_array));
	if (!(ret->kdRight)) {
		free(ret->kdLeft);
		free(ret);
		return NULL; //error
	}
	if (kdArr->size == 1) {
		ret->kdLeft = Copy(kdArr);
		ret->kdRight = NULL;
		return ret;
	}

	median = (kdArr->size % 2 == 0) ?
			(kdArr->size / 2) : (kdArr->size / 2) + 1;

	xArr = (int*)calloc(kdArr->size, sizeof(int));
	if (!xArr) {
		free(ret);
		return NULL; //error
	}

	ret->kdLeft->dim = kdArr->dim;
	ret->kdRight->dim = kdArr->dim;
	ret->kdLeft->size = median;
	ret->kdRight->size = kdArr->size - median;

	ret->kdLeft->pointsArray = (SPPoint*)calloc(ret->kdLeft->size,
			sizeof(SPPoint));
	if (!ret->kdLeft->pointsArray) {
		free(xArr);
		free(ret);
		return NULL; //error
	}

	ret->kdRight->pointsArray = (SPPoint*)calloc(ret->kdRight->size,
			sizeof(SPPoint));
	if (!ret->kdRight->pointsArray) {
		free(ret->kdLeft->pointsArray);
		free(xArr);
		free(ret);
		return NULL; //error
	}

	for (i = 0; i < median; i++)
		xArr[kdArr->indicesMatrix[coor][i]] = 0;

	for (i = median; i < kdArr->size; i++)
		xArr[kdArr->indicesMatrix[coor][i]] = 1;

	for (i = 0; i < kdArr->size; i++) {
		if (xArr[i] == 0) {
			// TODO - check the ret value is not null and clean everything
			// if it is
			ret->kdLeft->pointsArray[kdLeftIndex] =
					spPointCopy(kdArr->pointsArray[i]);
			kdLeftIndex++;
		}
		else {
			// TODO - check the ret value is not null and clean everything
			// if it is
			ret->kdRight->pointsArray[kdRightIndex] =
					spPointCopy(kdArr->pointsArray[i]);
			kdRightIndex++;
		}
	}

	map1 = (int*)calloc(kdArr->size, sizeof(int));
	if (!map1) {
		//TODO - cleanup
		return NULL; //error
	}

	map2 = (int*)calloc(kdArr->size, sizeof(int));
	if (!map2) {
		//TODO - cleanup
		return NULL; //error
	}

	kdLeftIndex = 0;
	kdRightIndex = 0;
	for (i = 0; i < kdArr->size; i++) {
		if (xArr[i] == 0) {
			map1[i] = kdLeftIndex;
			map2[i] = -1;
			kdLeftIndex++;
		} else {
			map2[i] = kdRightIndex;
			map1[i] = -1;
			kdRightIndex++;
		}
	}

	ret->kdLeft->indicesMatrix = (int**)calloc(ret->kdLeft->dim,
			sizeof(int *));
	if (!(ret->kdLeft->indicesMatrix)) {
		// TODO - clean everything
		return NULL; //error
	}

	ret->kdRight->indicesMatrix = (int**)calloc(ret->kdRight->dim,
			sizeof(int *));
	if (!(ret->kdRight->indicesMatrix)) {
		// TODO - clean everything
		return NULL; //error
	}

	for (j = 0; j < kdArr->dim; j++) {
		ret->kdLeft->indicesMatrix[j] = (int*)calloc(ret->kdLeft->dim,
					sizeof(int));
		if (!(ret->kdLeft->indicesMatrix[j])) {
			// TODO - clean everything
			return NULL; //error
		}

		ret->kdRight->indicesMatrix[j] = (int*)calloc(ret->kdRight->dim,
					sizeof(int));
		if (!(ret->kdRight->indicesMatrix[j])) {
			// TODO - clean everything
			return NULL; //error
		}

		kdLeftIndex = 0;
		kdRightIndex = 0;

		for (i = 0; i < kdArr->size; i++) {
			indexInOrigArr = kdArr->indicesMatrix[j][i];
			if (xArr[indexInOrigArr] == 0) {
				ret->kdLeft->indicesMatrix[j][kdLeftIndex] =
						map1[indexInOrigArr];
				kdLeftIndex++;
			}
			else {
				ret->kdRight->indicesMatrix[j][kdRightIndex] =
						map2[indexInOrigArr];
				kdRightIndex++;
			}
		}
	}

	return ret;
}

void spKDArrayDestroy(SPKDArray kdArr) {
	int i, j;
	if (kdArr != NULL) {
		if (kdArr->pointsArray != NULL) {
			for (i = 0; i < kdArr->size; i++)
				spPointDestroy(kdArr->pointsArray[i]);
		}

		if (kdArr->indicesMatrix != NULL) {
			for (j = 0; j < kdArr->dim; j++) {
				if (kdArr->indicesMatrix[j] != NULL) {
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
	if (kdArrPair != NULL) {
		if (kdArrPair->kdLeft != NULL) {
			spKDArrayDestroy(kdArrPair->kdLeft);
			kdArrPair->kdLeft = NULL;
		}
		if (kdArrPair->kdRight != NULL) {
			spKDArrayDestroy(kdArrPair->kdRight);
			kdArrPair->kdRight = NULL;
		}
		free(kdArrPair);
	}
}
