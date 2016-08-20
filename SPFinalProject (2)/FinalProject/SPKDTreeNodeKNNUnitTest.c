/*
 typedef struct distanceWithPoint {
	double distance;
	SPPoint point;
} distanceWithPoint;
 int distanceWithPointComparator(const void * firstItem, const void * secondItem)
{
	distanceWithPoint* item1;
	distanceWithPoint* item2;
	double dist;
	item1 = (distanceWithPoint*)firstItem;
	item2 = (distanceWithPoint*)secondItem;

	dist = (item1)->distance - (item2)->distance;

	if (dist < 0.0)
		return -1;

	if (dist > 0.0)
		return 1;

	return spPointGetIndex((item1)->point) - spPointGetIndex((item2)->point);
}

distanceWithPoint* createAndSortDistancesArray(int totalNumberOfFeatures, SPPoint relevantFeature){
	distanceWithPoint* distancesArray = NULL;
	int i,j,k=0;
	distancesArray = (distanceWithPoint*)calloc(sizeof(distanceWithPoint),totalNumberOfFeatures);
	if (distancesArray  == NULL){
		return NULL; //TODO -report relevant error
	}

	for (i=0;i<numOfImages;i++){
		for (j=0;j<workingImagesDatabase[i]->numOfFeatures;j++){
			distancesArray[k].point =(workingImagesDatabase[i]->featuresArray)[j];
			distancesArray[k].distance = spPointL2SquaredDistance(relevantFeature, distancesArray[k].point);
			k++;
		}
	}
	qsort(distancesArray, totalNumberOfFeatures, sizeof(distanceWithPoint), distanceWithPointComparator);
	return distancesArray;
}
  int* createOutputArray(distanceWithPoint* distancesArray)
{
	int feature;
	int *outputArray = (int*)calloc(topImages, sizeof(int));
	if (outputArray == NULL)
	{
		return NULL; //TODO - report relevant error
	}

	for (feature = 0; feature < topImages; feature++)
		outputArray[feature] = spPointGetIndex((distancesArray[feature]).point);

	return outputArray;
}
  int* spIQ_BestSIFTL2SquaredDistance(SPPoint relevantFeature, int totalNumberOfFeatures){

	int* outputArray = NULL;
	distanceWithPoint* distancesArray = NULL;

	// create distances array, fill it and sort it
	distancesArray = createAndSortDistancesArray(totalNumberOfFeatures, relevantFeature);

	if (distancesArray == NULL){
		return NULL;
	}

	// allocate output array and fill it
	outputArray = createOutputArray(distancesArray);

	// free the memory which is no longer needed
	free(distancesArray);

	return outputArray;

}*/
