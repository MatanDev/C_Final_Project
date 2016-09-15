/* ------------------------------------------------------------README-----------------------------------------------------------------
Please read the following regarding the unit tests:

* In order to run the unit tests please verify that '/unit_tests/main_testers.cpp' file is fully un-commented and run
  'testers_makefile' instead if 'makefile'

* There is no need to do any special action if you do not want to compile the unit tests, yet if you are working with an IDE,
  you should verify that '/unit_tests/main_testers.cpp' is fully commented

* some of the testers check for invalid arguments reaction, thus causing the logger to log some warnings and errors, so it
  is recommended that you set the logger output to a file and not the console, in order to view the tests results in a
  convenient way.
 -----------------------------------------------------------------------------------------------------------------------------------*/


#include <cstring>
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include "../SPImageProc.h"

extern "C" {
#include "../SPConfig.h"
#include "../SPLogger.h"
#include "../SPPoint.h"
#include "../image_parsing/SPImagesParser.h"
#include "../main_and_ui/SPImageQuery.h"
#include "../main_and_ui/SPMainAux.h"
#include "../data_structures/bpqueue_ds/SPBPriorityQueue.h"
#include "../data_structures/kd_ds/SPKDTreeNode.h"

#include "SPImagesParserUnitTest.h"
#include "SPConfigUnitTest.h"
#include "SPKDArrayUnitTest.h"
#include "SPKDTreeNodeUnitTest.h"
#include "SPPointUnitTest.h"
#include "SPListUnitTest.h"
#include "SPBPQueueUnitTest.h"
#include "SPKDTreeNodeKNNUnitTest.h"
}

#define TESTS_START_DECORATION		"-------------%s Tests Start-------------\n"
#define TESTS_END_DECORATION		"-------------%s Tests End---------------\n\n\n"
#define	TESTS_CONFIG_FILE_PATH		"./unit_tests/spcbir.config"
#define MAIN_FAILURE_RET_VAL		-1
#define STDOUT						"stdout"
#define	IMAGES_PARSER_SEC_NAME		"Images Parser"
#define	CONFIG_SEC_NAME				"Configuration"
#define	KDARRAY_SEC_NAME			"KDArray"
#define	KDTREE_NODE_SEC_NAME		"KDTree Node"
#define	KDTREE_NODE_KNN_SEC_NAME	"KDTree Node KNN"
#define	LIST_SEC_NAME				"List"
#define	POINT_SEC_NAME				"Point"
#define	BPQ_SEC_NAME				"B Priority Queue"

#define testDecorator(testFunction, sectionName) do {				\
	printf(TESTS_START_DECORATION, sectionName);	\
	testFunction;													\
	printf(TESTS_END_DECORATION, sectionName);	\
	} while(0)


int main() {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate(TESTS_CONFIG_FILE_PATH, &msg);
	char* loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return MAIN_FAILURE_RET_VAL;

	spLoggerCreate(!strcmp(loggerFilename, STDOUT) ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	testDecorator(runImagesParserTests(config), IMAGES_PARSER_SEC_NAME);
	testDecorator(runConfigTests(), CONFIG_SEC_NAME);
	testDecorator(runKDArrayTests(), KDARRAY_SEC_NAME);
	testDecorator(runKDTreeNodeTests(), KDTREE_NODE_SEC_NAME);
	testDecorator(runKDTreeNodeKNNTests(), KDTREE_NODE_KNN_SEC_NAME);
	testDecorator(runListTests(), LIST_SEC_NAME);
	testDecorator(runPointTests(), POINT_SEC_NAME);
	testDecorator(runBPQueueTests(), BPQ_SEC_NAME);
	spConfigDestroy(config);
	spLoggerDestroy();
	return 0;
}


