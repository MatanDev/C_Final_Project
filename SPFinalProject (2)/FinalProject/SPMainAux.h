/*
 * SPMainAux.h
 *
 *  Created on: 3 баев 2016
 *      Author: Matan
 */

#ifndef SPMAINAUX_H_
#define SPMAINAUX_H_

#include "SPLogger.h"
#include "SPConfig.h"
#include "SPImagesParser.h"


void endControlFlow(SPConfig config, SPImageData image, SPImageData* imagesList, int numOfImages);

void getAsString(const char* message, char* destination);

void getQuery(char* destination);

int* searchSimilarImages(const SPConfig config,SPImageData* imagesDatabase,SPImageData workingImage, int simmilarCount);

void presentSimilarImagesNoGUI(int* imagesIndexesArray, int imagesCount);

SP_CONFIG_MSG loadRelevantSettingsData(const SPConfig config, int* numOfImages, int* numOfSimilar, bool* extractFlag, bool* GUIFlag);

void initializeImagesDataList(SPImageData** imagesDataList, int numOfImages);

SPImageData initializeWorkingImage();

#endif /* SPMAINAUX_H_ */
