#ifndef SPKDTREENODEKNN_H_
#define SPKDTREENODEKNN_H_

#include "SPKDTreeNode.h"
#include "SPBPriorityQueue.h"

/*
 * The method returns a squared distance between 2 real values
 * @param a - first value
 * @param b - second value
 *
 * @returns pow((a-b),2)
 */
double getSquaredDistance(double a, double b);

/*
 * The method fills the bpq with the k-nearest points to queryPoint
 * Pre assumptions - bpq and queryPoint are not NULL
 * the method returns false in case of failure to create list element or enqueue it
 * (and thus the search failed),otherwise true.
 *
 * @param curr - the current tree node that is tested
 * @param bpq - a pre-initialized priority queue that returns the k-nearest points
 * @param queryPoint - the query point
 *
 * @returns true iff the search was successful
 *
 * @logger - the method logs the relevant error to the logger
 */
bool kNearestNeighbors(SPKDTreeNode curr, SPBPQueue bpq, SPPoint queryPoint);

/*
 * The methods returns the other son of the tree.
 * Pre assumptions - tree != NULL and first child is either the right or
 * left child of tree
 * @param tree - a tree node
 * @param firstChild - one of the children of tree
 *
 * @returns - the other child of tree
 */
SPKDTreeNode getSecondChild(SPKDTreeNode tree, SPKDTreeNode firstChild);

/*
 * The method gets the current point, a queue and a query points
 * and pushed a list element representing the current point index and distance from queryPoint
 * to the queue
 *
 * pre-assumptions - bpq is initialized and not NULL, currPoint and queryPoint are not NULL
 *
 * @param currPoint - the relevant point to add to the queue
 * @param bpq - an initialized priority queue
 * @param queryPoint - the query point
 *
 * @returns true iff the enqueue process was successful
 *
 * @logger - the method logs the relevant error to the logger
 */
bool pushLeafToQueue(SPPoint currPoint, SPBPQueue bpq, SPPoint queryPoint);

/*
 * The method gets a tree node, and checks if the candidate hyper-sphere crosses the
 * splitting plane.
 *
 * pre-assumptions - node != NULL, maxBPQValue >= 0
 *
 * @param node - the current node that represents the splitting plane
 * @param coorValue - the relevant axis value of the query point
 * @param maxBPQValue - the maximum distance between the query point and the points in the queue
 *
 * @returns true iff the candidate's hyper-sphere crosses the the node's splitting plane
 */
bool isCrossingHypersphere(SPKDTreeNode node, double coorValue, double maxBPQValue);

#endif /* SPKDTREENODEKNN_H_ */
