#include <stdlib.h>
#include <assert.h>
#include "SPConfig.h"


SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg)
{
	return NULL;
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg){
	return 1;
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg){
	return 1;
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg){
	return 1;
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg){
	return 1;
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg){
	return 1;
}

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
		int index){
	return SP_CONFIG_MISSING_DIR;
}

SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config){
	return SP_CONFIG_MISSING_DIR;
}

void spConfigDestroy(SPConfig config){
	return;
}

