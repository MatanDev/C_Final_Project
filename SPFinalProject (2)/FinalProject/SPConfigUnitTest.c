#include "unit_test_util.h"
#include "SPConfig.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"

bool testGivenConfFile() {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("spcbir.config", &msg);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(config->spImagesDirectory, "./images/"));
	ASSERT_TRUE(!strcmp(config->spImagesPrefix, "img"));
	ASSERT_TRUE(!strcmp(config->spImagesSuffix, ".png"));
	ASSERT_TRUE(config->spNumOfImages == 22);
	ASSERT_TRUE(config->spPCADimension == 20);
	ASSERT_TRUE(!strcmp(config->spPCAFilename, "pca.yml"));
	ASSERT_TRUE(config->spNumOfFeatures == 100);
	ASSERT_TRUE(config->spExtractionMode == false);
	ASSERT_TRUE(config->spMinimalGUI == true);
	ASSERT_TRUE(config->spNumOfSimilarImages == 5);
	ASSERT_TRUE(config->spKNN == 1);
	ASSERT_TRUE(config->spKDTreeSplitMethod == MAX_SPREAD);
	ASSERT_TRUE(config->spLoggerLevel == 3);
	ASSERT_TRUE(!strcmp(config->spLoggerFilename, "stdout"));

	ASSERT_TRUE(spConfigIsExtractionMode(config, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigIsExtractionMode(NULL, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	ASSERT_TRUE(spConfigMinimalGui(config, &msg) == true);
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

	char imagePath[100];
	ASSERT_TRUE(spConfigGetImagePath(imagePath, config, 13) == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(imagePath, "./images/img13.png"));

	ASSERT_TRUE(spConfigGetImagePath(imagePath, config, 22) == SP_CONFIG_INDEX_OUT_OF_RANGE);
	ASSERT_TRUE(spConfigGetImagePath(NULL, config, 1) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetImagePath(imagePath, NULL, 1) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetImagePath(NULL, NULL, 1) == SP_CONFIG_INVALID_ARGUMENT);

	char pcaPath[100];
	ASSERT_TRUE(spConfigGetPCAPath(pcaPath, config) == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(!strcmp(pcaPath, "./images/pca.yml"));

	ASSERT_TRUE(spConfigGetPCAPath(NULL, config) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetPCAPath(pcaPath, NULL) == SP_CONFIG_INVALID_ARGUMENT);
	ASSERT_TRUE(spConfigGetPCAPath(NULL, NULL) == SP_CONFIG_INVALID_ARGUMENT);

	spConfigDestroy(config);

	printf("all well\n");
	fflush(NULL);
	return true;
}

bool testParseLine() {
	char line[1024];
	char *varName, *value;
	bool isCommentOrEmpty;
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	ASSERT_TRUE(parseLine("a", 1, line = "a = b\n", &varName, &value,
			&isCommentOrEmpty, &msg) == true);
	ASSERT_TRUE(!strcmp(varName, "a"));
	ASSERT_TRUE(!strcmp(value, "b"));
	ASSERT_TRUE(!isCommentOrEmpty);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	return true;
}

void runConfigTests() {
	RUN_TEST(testGivenConfFile);
	RUN_TEST(testParseLine);
}
