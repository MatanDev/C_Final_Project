#include "unit_test_util.h"
#include "../SPConfig.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

bool testGivenConfFile() {
	char imagePath[100], pcaPath[100];
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("./unit_tests/spcbirTestCase1.config", &msg);

	ASSERT_TRUE(config != NULL);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(!strcmp(spConfigGetImagesDirectory(config, &msg), "./images/"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetImagesDirectory(NULL, &msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(!strcmp(spConfigGetImagesPrefix(config, &msg), "img"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetImagesPrefix(NULL, &msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(!strcmp(spConfigGetImagesSuffix(config, &msg), ".png"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetImagesSuffix(NULL, &msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(!strcmp(spConfigGetPCAFilename(config, &msg), "pca.yml"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetPCAFilename(NULL, &msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigIsExtractionMode(config, &msg) == true);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigIsExtractionMode(NULL, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigMinimalGui(config, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigMinimalGui(NULL, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetNumOfImages(config, &msg) == 22);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetNumOfImages(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetNumOfFeatures(config, &msg) == 100);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetNumOfFeatures(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetPCADim(config, &msg) == 20);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetPCADim(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetNumOfSimilarImages(config, &msg) == 5);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetNumOfSimilarImages(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetKNN(config, &msg) == 5);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetKNN(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetSplitMethod(config, &msg) == MAX_SPREAD);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetSplitMethod(NULL, &msg) == MAX_SPREAD);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigGetLoggerLevel(config, &msg) == SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetLoggerLevel(NULL, &msg) == SP_LOGGER_INFO_WARNING_ERROR_LEVEL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(!strcmp(spConfigGetLoggerFilename(config, &msg), "stdout"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetLoggerFilename(NULL, &msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);


	ASSERT_TRUE(spConfigGetImagePath(imagePath, config, 13) == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(imagePath, "./images/img13.png"));

	ASSERT_TRUE(spConfigGetImagePath(imagePath, config, 22) == SP_CONFIG_INDEX_OUT_OF_RANGE);
	ASSERT_TRUE(spConfigGetImagePath(NULL, config, 1) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetImagePath(imagePath, NULL, 1) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetImagePath(NULL, NULL, 1) == SP_CONFIG_INVALID_ARGUMENT);


	ASSERT_TRUE(spConfigGetPCAPath(pcaPath, config) == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(pcaPath, "./images/pca.yml"));

	ASSERT_TRUE(spConfigGetPCAPath(NULL, config) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetPCAPath(pcaPath, NULL) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetPCAPath(NULL, NULL) == SP_CONFIG_INVALID_ARGUMENT);

	spConfigDestroy(config);

	return true;
}

bool testParseLine() {
	char line[1024];
	char *varName, *value;
	bool isCommentOrEmpty = false;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	strcpy(line, "a = b\n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(!strcmp(varName, "a"));
	ASSERT_TRUE(!strcmp(value, "b"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "c=d\n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(!strcmp(varName, "c"));
	ASSERT_TRUE(!strcmp(value, "d"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "now with spaces	 =in the value also\n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(!strcmp(varName, "now with spaces"));
	ASSERT_TRUE(!strcmp(value, "in the value also"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "now with spaces=		in the value also  \n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(!strcmp(varName, "now with spaces"));
	ASSERT_TRUE(!strcmp(value, "in the value also"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "		 and now with spaces before var name = in the value also  \n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(!strcmp(varName, "and now with spaces before var name"));
	ASSERT_TRUE(!strcmp(value, "in the value also"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	isCommentOrEmpty = false;

	strcpy(line, "         ");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	isCommentOrEmpty = false;

	strcpy(line, "#");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	isCommentOrEmpty = false;

	strcpy(line, "#		a = b");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg));
	ASSERT_TRUE(isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	strcpy(line, "		a ");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_LINE);

	strcpy(line, "this is a text with spaces\n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_LINE);

	strcpy(line, "spImagesDirectory =    \n");
	ASSERT_TRUE(parseLine("a", 1, line, &varName, &value,
			&isCommentOrEmpty, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_LINE);

	return true;
}

bool testDefault() {
	char imagePath[100], *loggerFilename = NULL, *pcaFilename = NULL;
	SPConfig config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	ASSERT_TRUE(config != NULL);

	initConfigToDefault(config);

	ASSERT_TRUE(spConfigGetImagesDirectory(config, &msg) == NULL);
	ASSERT_TRUE(spConfigGetImagesPrefix(config, &msg) == NULL);
	ASSERT_TRUE(spConfigGetImagesSuffix(config, &msg) == NULL);
	ASSERT_TRUE(spConfigGetNumOfImages(config, &msg) == 0);

	ASSERT_TRUE(parameterSetCheck(config, &msg, "a", 1, NULL) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_MISSING_DIR);
	msg = SP_CONFIG_SUCCESS;

	config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	ASSERT_TRUE(config != NULL);
	initConfigToDefault(config);
	spConfigSetImagesDirectory(config, duplicateString("./bla/bla/"));
	ASSERT_TRUE(parameterSetCheck(config, &msg, "a", 1, NULL) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_MISSING_PREFIX);
	msg = SP_CONFIG_SUCCESS;

	config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	ASSERT_TRUE(config != NULL);
	initConfigToDefault(config);
	spConfigSetImagesDirectory(config, duplicateString("./bla/bla/"));
	spConfigSetImagesPrefix(config, duplicateString("whatever"));
	ASSERT_TRUE(parameterSetCheck(config, &msg, "a", 1, NULL) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_MISSING_SUFFIX);
	msg = SP_CONFIG_SUCCESS;

	config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	ASSERT_TRUE(config != NULL);
	initConfigToDefault(config);
	spConfigSetImagesDirectory(config, duplicateString("./bla/bla/"));
	spConfigSetImagesPrefix(config, duplicateString("whatever"));
	spConfigSetImagesSuffix(config, duplicateString(".jpg"));
	ASSERT_TRUE(parameterSetCheck(config, &msg, "a", 1, NULL) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_MISSING_NUM_IMAGES);
	msg = SP_CONFIG_SUCCESS;

	config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	ASSERT_TRUE(config != NULL);
	initConfigToDefault(config);
	spConfigSetImagesDirectory(config, duplicateString("./bla/bla/"));
	spConfigSetImagesPrefix(config, duplicateString("whatever"));
	spConfigSetImagesSuffix(config, duplicateString(".jpg"));
	spConfigSetImagesNum(config, 9000);
	ASSERT_TRUE(parameterSetCheck(config, &msg, "a", 1, NULL) == config);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigGetImagePath(imagePath, config, 4444) == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(imagePath, "./bla/bla/whatever4444.jpg"));

	ASSERT_TRUE(checkAndSetDefIfNeeded(&loggerFilename, "stdout", &msg));
	ASSERT_TRUE(!strcmp(loggerFilename, "stdout"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	free(loggerFilename);

	loggerFilename = duplicateString("/tmp/whatever");
	ASSERT_TRUE(checkAndSetDefIfNeeded(&loggerFilename, "stdout", &msg));
	ASSERT_TRUE(!strcmp(loggerFilename, "/tmp/whatever"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	free(loggerFilename);

	ASSERT_TRUE(checkAndSetDefIfNeeded(&pcaFilename, "pca.yml", &msg));
	ASSERT_TRUE(!strcmp(pcaFilename, "pca.yml"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	free(pcaFilename);

	pcaFilename = duplicateString("/tmp/whatever2");
	ASSERT_TRUE(checkAndSetDefIfNeeded(&pcaFilename, "pca.yml", &msg));
	ASSERT_TRUE(!strcmp(pcaFilename, "/tmp/whatever2"));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	free(pcaFilename);

	spConfigDestroy(config);
	return true;
}

bool testHandler() {
	SPConfig config = (SPConfig)calloc(1, spConfigGetConfigStructSize());
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;

	ASSERT_TRUE(config != NULL);

	ASSERT_FALSE(handleVariable(config, "a", 1, "spImagesDirectori", "C:\\MyDocuments\\",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_LINE);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spImagesDirectory", "C:\\My Documents\\",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_STRING);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spImagesPrefix", "my image", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_STRING);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spImagesSuffix", ".jpj", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_STRING);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spNumOfImages", "-9000", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	// check that a valid int is according to the rules in the forum (next 4 tests)
	ASSERT_TRUE(handleVariable(config, "a", 1, "spNumOfImages", "014", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfImages(config, &msg) == 14);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_TRUE(handleVariable(config, "a", 1, "spNumOfImages", "+1", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfImages(config, &msg) == 1);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spNumOfImages", "1.0", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spNumOfImages", "20 40", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spNumOfSimilarImages", "0", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spKNN", "aaa", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spLoggerLevel", "5", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spPCADimension", "9", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spPCADimension", "30", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_INTEGER);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spKDTreeSplitMethod", "something", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_KDTREE_SPLIT_METHOD);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spExtractionMode", "tru", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_BOOLEAN);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_FALSE(handleVariable(config, "a", 1, "spMinimalGUI", "fal", &msg));
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_BOOLEAN);
	msg = SP_CONFIG_SUCCESS;

	ASSERT_TRUE(handleVariable(config, "a", 1, "spImagesDirectory", "C:\\Documents\\",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(spConfigGetImagesDirectory(config, &msg), "C:\\Documents\\"));

	ASSERT_TRUE(handleVariable(config, "a", 1, "spImagesSuffix", ".bmp",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(spConfigGetImagesSuffix(config, &msg), ".bmp"));

	ASSERT_TRUE(handleVariable(config, "a", 1, "spNumOfFeatures", "80",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfFeatures(config, &msg) == 80);

	ASSERT_TRUE(handleVariable(config, "a", 1, "spPCADimension", "19",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetPCADim(config, &msg) == 19);

	ASSERT_TRUE(handleVariable(config, "a", 1, "spExtractionMode", "true",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigIsExtractionMode(config, &msg));

	ASSERT_TRUE(handleVariable(config, "a", 1, "spKDTreeSplitMethod", "INCREMENTAL",
			&msg));
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetSplitMethod(config, &msg) == INCREMENTAL);

	spConfigDestroy(config);
	return true;
}

void runConfigTests() {
	// Because of the fact that edge cases are also tested in these unit tests
	// there might be printed messages that can be ignored in case all we care about is
	// whether the tests have passed or not
	RUN_TEST(testGivenConfFile);
	RUN_TEST(testParseLine);
	RUN_TEST(testDefault);
	RUN_TEST(testHandler);
}

