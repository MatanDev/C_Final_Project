#include "SPListElement.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../general_utils/SPUtils.h"

typedef struct sp_list_element_t {
	int index;
	double value;
} sp_list_element_t;

SPListElement spListElementCreate(int index, double value) {
	SPListElement temp = NULL;

	spMinimalVerifyArgumentsRn(index >= 0 && value >= 0.0);

	spCalloc(temp, sp_list_element_t, 1);

	temp->index = index;
	temp->value = value;
	return temp;
}

SPListElement spListElementCopy(SPListElement data) {
	SPListElement elementCopy = NULL;
	spMinimalVerifyArgumentsRn(data != NULL);

	spCalloc(elementCopy, sp_list_element_t, 1);

	elementCopy->index = data->index;
	elementCopy->value = data->value;
	return elementCopy;
}

void spListElementDestroy(SPListElement data) {
	if (data == NULL) {
		return;
	}
	free(data);
}

SP_ELEMENT_MSG spListElementSetIndex(SPListElement data, int index) {
	spMinimalVerifyArguments(data != NULL && index >= 0,
			SP_ELEMENT_INVALID_ARGUMENT);
	data->index = index;
	return SP_ELEMENT_SUCCESS;
}

int spListElementGetIndex(SPListElement data) {
	spMinimalVerifyArguments(data != NULL,	-1);
	return data->index;
}

SP_ELEMENT_MSG spListElementSetValue(SPListElement data, double newValue) {
	spMinimalVerifyArguments(data != NULL && newValue >= 0.0,
			SP_ELEMENT_INVALID_ARGUMENT);

	data->value = newValue;
	return SP_ELEMENT_SUCCESS;
}

double spListElementGetValue(SPListElement data) {
	spMinimalVerifyArguments(data != NULL,-1.0);
	return data->value;
}

int spListElementCompare(SPListElement e1, SPListElement e2){
	assert(e1!=NULL && e2!=NULL);
	if(e1->value == e2->value){
		return e1->index - e2->index;
	}else if(e1->value > e2->value){
		return 1;
	}
	return -1;
}
