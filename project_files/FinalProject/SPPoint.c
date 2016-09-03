#include "SPPoint.h"
#include "SPLogger.h"
#include <stdlib.h>
#include <assert.h>
#include "general_utils/SPUtils.h"


bool isEqual(double x,double y)
{
	return (0 <=(x-y) && (x-y) <= epsilon) || (0<= (y-x) && (y-x) <= epsilon);
}


#define WARNING_POINT_NULL "Point is null when free is called"

/*
 * A structure used for the point data type
 * data - an array of the axis data of the point
 * dim - an integer representing the dimension of the point
 * index -  an integer representing the image index related to the point
 */
typedef struct sp_point_t {
	double* data;
	int dim;
	int index;
} sp_point_t;

/*
 * creates a new double array with the same values of the given array
 * @data - the source array to be copied
 * @size - the size of the array
 *
 * @returns
 * NULL if data is NULL or size < 0, otherwise a hard copy of the given array
 */
double* copyData(double* data, int size) {
	int i;
	double* newData;
	spMinimalVerifyArgumentsRn(data != NULL && size >= 0);

	spCalloc(newData, double, size);

	for (i = 0; i < size; i++)
		newData[i] = data[i];

	return newData;
}

SPPoint spPointCreate(double* data, int dim, int index) {
	spMinimalVerifyArgumentsRn(data != NULL && index >= 0 && dim >0);
	SPPoint item = NULL;

	spCalloc(item, sp_point_t, 1);

	item->data = copyData(data,dim);
	if (item->data == NULL)
		return NULL; //allocation error

	item->dim = dim;
	item->index = index;

	return item;
}

SPPoint spPointCopy(SPPoint source) {
	assert (source != NULL);
	spMinimalVerifyArgumentsRn(source->data != NULL);
	return spPointCreate(source->data,source->dim,source->index);
}

bool spPointCompare(SPPoint p1, SPPoint p2){
	int i;
	spMinimalVerifyArguments(p1 != NULL && p2 != NULL, false);

	if (p1->dim != p2->dim)
		return false;
	if (p1->index != p2->index)
		return false;

	for (i = 0; i< p1->dim;i++){
		if (!isEqual(p1->data[i],p2->data[i]))
			return false;
	}
	return true;
}

void spPointDestroy(SPPoint point) {
	if (point != NULL) {
		spFree(point->data);
		free(point);
		point = NULL;
	}
	else {
		spLoggerSafePrintWarning(WARNING_POINT_NULL, __FILE__,__FUNCTION__, __LINE__);
	}
}

int spPointGetDimension(SPPoint point) {
	assert(point != NULL);
	return point->dim;
}

int spPointGetIndex(SPPoint point) {
	assert(point != NULL);
	return point->index;
}

double spPointGetAxisCoor(SPPoint point, int axis) {
	assert(point != NULL && axis < point->dim && axis >= 0);
	return point->data[axis];
}

double spPointL2SquaredDistance(SPPoint p, SPPoint q) {
	int dimIndex;
	double l2Dist = 0, currentDist;

	assert(p != NULL && q != NULL && p->dim == q->dim);

	for (dimIndex = 0;dimIndex < p->dim ; dimIndex++) {
		currentDist = p->data[dimIndex]-q->data[dimIndex];
		l2Dist += currentDist * currentDist;
	}

	return l2Dist;
}

