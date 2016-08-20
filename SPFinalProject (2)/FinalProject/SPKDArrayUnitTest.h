#ifndef SPKDARRAYUNITTEST_H_
#define SPKDARRAYUNITTEST_H_



void runKDArrayTests();

SPPoint* generateRandomPointsArray(int dim, int size);

SPPoint generateRandomPoint(int dim, int index);

void destroyPointsArray(SPPoint* array, int numOfItems);

#endif /* SPKDARRAYUNITTEST_H_ */
