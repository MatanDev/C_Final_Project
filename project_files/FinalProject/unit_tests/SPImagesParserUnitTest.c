#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../image_parsing/SPImagesParser.h"
#include "unit_test_util.h"
#include "../SPPoint.h"
#include "../SPConfig.h"
#include "SPImagesParserUnitTest.h"

SPConfig internalConfig = NULL;
char* configSign = NULL;

static bool identicalFiles(const char* fname1, const char* fname2) {
	FILE *fp1, *fp2;
	fp1 = fopen(fname1, "r");
	fp2 = fopen(fname2, "r");
	char ch1 = EOF, ch2 = EOF;

	if (fp1 == NULL) {
		return false;
	} else if (fp2 == NULL) {
		fclose(fp1);
		return false;
	} else {
		ch1 = getc(fp1);
		ch2 = getc(fp2);

		while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
			ch1 = getc(fp1);
			ch2 = getc(fp2);
		}
		fclose(fp1);
		fclose(fp2);
	}
	if (ch1 == ch2) {
		return true;
	} else {
		return false;
	}
}

static bool pointToStringTests(){
	char* csvLine = NULL;
	int rslt ;
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;
	double data[4] = { -5,6,0.43434343434,6.7};
	SPPoint point1 = spPointCreate(data, 4, 7);

	csvLine = pointToString(point1, &msg);
	rslt = strcmp(csvLine,"4,-5.000000,6.000000,0.434343,6.700000\n");
	ASSERT_TRUE(rslt == 0);
	ASSERT_TRUE(msg == SP_DP_SUCCESS);


	spPointDestroy(point1);
	free(csvLine);

	return true;
}

static bool stringToPoint(){
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;

	int i;
	int dim = 4;
	int index = 7;
	double data[4] = { -5,6,0.434343,6.7};
	SPPoint point1 = spPointCreate(data, dim, index), point2;

	point2 = parsePointFromString("4,-5.000000,6.000000,0.434343,6.700000\n", index, &msg);

	ASSERT_TRUE(msg == SP_DP_SUCCESS);


	ASSERT_TRUE(spPointGetDimension(point1) == dim
			&& spPointGetDimension(point2) == dim );


	ASSERT_TRUE(spPointGetIndex(point1) == index
			&& spPointGetIndex(point2) == index );


	for (i = 0; i < dim; i++) {
		ASSERT_TRUE(isEqual(spPointGetAxisCoor(point1, i), spPointGetAxisCoor(point2, i)));
	}

	spPointDestroy(point1);
	spPointDestroy(point2);

	return true;
}

static bool testGetImageHeaderAsString(){
	SPImageData imageData = NULL;
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;
	char *header;
	imageData = (SPImageData)malloc(sizeof(struct sp_image_data));

	ASSERT_TRUE(imageData != NULL);
	if (imageData != NULL){
		imageData->index = 5;
		imageData->numOfFeatures = 14;
		imageData->featuresArray = NULL;

		header = getImageStringHeader(imageData ,&msg);

		ASSERT_TRUE(msg == SP_DP_SUCCESS);
		ASSERT_TRUE(strcmp(header,"5,14\n")== 0);

		free(header);
		free(imageData);

		return true;
	}
	return false;
}

static bool testLoadImageDataFromHeader(){
	SPImageData imageData = NULL;
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;

	imageData = createImageData(5);
	imageData->featuresArray = NULL;
	ASSERT_TRUE(imageData != NULL);

	msg = loadImageDataFromHeader("5,14\n",imageData);

	ASSERT_TRUE(msg == SP_DP_SUCCESS);
	ASSERT_TRUE(imageData->index == 5);
	ASSERT_TRUE(imageData->numOfFeatures == 14);

	free(imageData);
	return true;
}

static bool testSaveImageData(){
	SPImageData imageData = NULL;

	double data1[] = {1,3,5,4};
	double data2[] = {4,5.5,13413,92,1};
	double data3[] = {0 , 0,0};

	SPPoint p1 = spPointCreate(data1, 4, 5),p2 = spPointCreate(data2, 5, 5),p3 = spPointCreate(data3, 3, 5);

	SPPoint points[] = {p1,p2,p3};


	SP_DP_MESSAGES msg = SP_DP_SUCCESS;
	imageData = (SPImageData)malloc(sizeof(struct sp_image_data));

	ASSERT_TRUE(imageData != NULL);
	if (imageData != NULL){


		imageData->index = 5;
		imageData->numOfFeatures = 3;
		imageData->featuresArray = points;

		msg = saveImageData(internalConfig,configSign, imageData);

		ASSERT_TRUE(msg == SP_DP_SUCCESS);


		ASSERT_TRUE(identicalFiles("./unit_tests/images/img5.feats","./unit_tests/images/test1.feats"));


		free(imageData);

		spPointDestroy(p1);
		spPointDestroy(p2);
		spPointDestroy(p3);

		return true;
	}
	return false;
}

static bool testLoadKnownImageData(){
	SPImageData imageData = NULL;
	SP_DP_MESSAGES msg = SP_DP_SUCCESS;
	bool successFlag = true;
	double data1[] = {1,3,5,4};
	double data2[] = {4,5.5,13413,92,1};
	double data3[] = {0 , 0,0};

	SPPoint p1 = spPointCreate(data1, 4, 5),p2 = spPointCreate(data2, 5, 5),p3 = spPointCreate(data3, 3, 5);



	imageData = (SPImageData)malloc(sizeof(struct sp_image_data));
	successFlag &= (imageData != NULL);
	imageData->index = 5;

	if (imageData != NULL){
		msg = loadKnownImageData(configSign,"./unit_tests/images/test1.feats", imageData);

		successFlag &= (msg == SP_DP_SUCCESS);

		successFlag &= (imageData->numOfFeatures = 3);
		successFlag &= (imageData->index = 5);

		imageData->numOfFeatures = 3;

		successFlag &= (spPointCompare(imageData->featuresArray[0],p1));
		successFlag &= (spPointCompare(imageData->featuresArray[1],p2));
		successFlag &= (spPointCompare(imageData->featuresArray[2],p3));

		freeImageData(imageData,false, true);
	}
	spPointDestroy(p1);
	spPointDestroy(p2);
	spPointDestroy(p3);
	return successFlag;
}

bool testGetLineBySize(int size){
	char *line0 = NULL, *line1 = NULL, *line2 = NULL, *line3 = NULL, *line4 = NULL, *line5 = NULL, *line6 = NULL;
	FILE* fp = NULL;
	fp = fopen("./unit_tests/images/test1.feats","r");

	ASSERT_TRUE(fp != NULL);

	line0 = getLineByMinBufferSize(fp,size);
	line1 = getLineByMinBufferSize(fp,size);
	line2 = getLineByMinBufferSize(fp,size);
	line3 = getLineByMinBufferSize(fp,size);
	line4 = getLineByMinBufferSize(fp,size);
	line5 = getLineByMinBufferSize(fp,size);
	line6 = getLineByMinBufferSize(fp,size);

	ASSERT_TRUE(strcmp(line0,configSign) == 0);
	ASSERT_TRUE(strcmp(line1,"5,3\n") == 0);
	ASSERT_TRUE(strcmp(line2,"4,1.000000,3.000000,5.000000,4.000000\n") == 0);
	ASSERT_TRUE(strcmp(line3,"5,4.000000,5.500000,13413.000000,92.000000,1.000000\n") == 0);
	ASSERT_TRUE(strcmp(line4,"3,0.000000,0.000000,0.000000\n") == 0);
	ASSERT_TRUE(strcmp(line5,"") == 0);
	ASSERT_TRUE(strcmp(line5,"") == 0);

	free(line0);
	free(line1);
	free(line2);
	free(line3);
	free(line4);
	free(line5);
	free(line6);

	fclose(fp);
	return true;
}

bool testGetLine(){
	int i;
	for (i = 1 ; i <= 1024 ; i++){
		if (!testGetLineBySize(2*i)){
			return false;
		}
	}
	return true;
}

void RunImagesParserTests(SPConfig config){
	internalConfig = config;
	configSign = getSignature(config);
	if (configSign == NULL){
		printf("%s Line %d: %s", __FILE__, __LINE__, "could not create config signature");
		return;
	}
	RUN_TEST(pointToStringTests);
	RUN_TEST(stringToPoint);
	RUN_TEST(testGetImageHeaderAsString);
	RUN_TEST(testLoadImageDataFromHeader);
	RUN_TEST(testSaveImageData);
	RUN_TEST(testLoadKnownImageData);
	RUN_TEST(testGetLine);
	free(configSign);
}
