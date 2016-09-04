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

int main() {
	SP_CONFIG_MSG msg = SP_CONFIG_SUCCESS;
	SPConfig config = spConfigCreate("./unit_tests/spcbir.config", &msg);
	char* loggerFilename = spConfigGetLoggerFilename(config, &msg);
	if (loggerFilename == NULL || msg != SP_CONFIG_SUCCESS)
		return -1;

	spLoggerCreate(!strcmp(loggerFilename, "stdout") ? NULL : loggerFilename,
			spConfigGetLoggerLevel(config, &msg));
	RunImagesParserTests(config);
	//runConfigTests();
	//runKDArrayTests();
	//runKDTreeNodeTests();
	//runKDTreeNodeKNNTests();
	//runListTests();
	//runPointTests();
	//runBPQueueTests();
	spConfigDestroy(config);
	spLoggerDestroy();
	return 0;
}
