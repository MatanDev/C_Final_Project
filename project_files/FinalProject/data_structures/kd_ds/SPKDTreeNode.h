#ifndef SPKDTREENODE_H_
#define SPKDTREENODE_H_

#include "../../SPPoint.h"
#include "../../SPConfig.h"
#include "SPKDArray.h"

/*
 * A structure used to represent a KD tree node,
 * contains splitting dimension as 'dim',
 * pointer to median value as 'val',
 * left son as 'kdtLeft',
 * right son as 'kdtRight'
 * and relevant data as 'data'
 */
typedef struct sp_kd_tree_node {
	int dim;
	double* val;
	struct sp_kd_tree_node* kdtLeft;
	struct sp_kd_tree_node* kdtRight;
	SPPoint data;
} sp_kd_tree_node;

/*
 * A pointer to the sp_kd_tree_node structure
 */
typedef struct sp_kd_tree_node* SPKDTreeNode;

/* The method initializes a new kd-tree recursively according to the given SPPoint
 * array and split method.
 *
 * @param pointsArray - the relevant SPPoint array to work by
 * @param size - the size of the 'pointsArray'
 * @param splitMethod - an enum representing the splitting criteria:
 *   - MAX_SPREAD - uses the dimension that contains the largest difference between the
 *   last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where i is the
 *	 recursive depth.
 * @returns -
 *  NULL if :
 *  - pointsArray is NULL or
 *  - size <= 0 or
 *  - not all points in pointsArray have the same dimension or
 *  - memory allocation failed
 *   otherwise returns a pointer to the root of the
 *   kd-tree that is built by the given points array and split method.
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 * debug prints are also printed to the logger
 */
SPKDTreeNode InitKDTreeFromPoints(SPPoint* pointsArray, int size,
		SP_KDTREE_SPLIT_METHOD splitMethod);

/*
 * The method initializes a new kd-tree recursively according to the given
 * kd-array and split method.
 *
 * @param array - the relevant kd-array to work by
 * @param splitMethod - an enum representing the splitting criteria:
 *   - MAX_SPREAD - uses the dimension that contains the largest difference between the
 *   last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where i is the
 *	 recursive depth.
 * @returns -
 *  NULL if :
 *   - array is NULL
 *   - memory allocation failed
 *   otherwise returns a pointer to the root of the
 *   kd-tree that is built by the given array and split method.
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 * debug prints are also printed to the logger
 */
SPKDTreeNode InitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod);

/*
 * Destroys the given sp_kd_tree_node structure instance and returns NULL
 *
 * @param node - pointer to the sp_kd_tree_node structure instance
 *
 * @returns NULL
 */
SPKDTreeNode onErrorInInitKDTree(SPKDTreeNode node);

/*
 * The method is called when the size of the SPKDArray 'array' is 1,
 * and returns a leaf whose data is the single point in 'array' points-array
 *
 * pre assumptions - node and array are valid
 *
 * @param node - the previously initiated SPKDTreeNode instance
 * @param array - the relevant kd-array to work by
 * @assert array->pointsArray != NULL
 *
 * @returns a leaf whose data is the single point in 'array' points-array
 */
SPKDTreeNode createLeaf(SPKDTreeNode node, SPKDArray array);

/*
 * The method calculates and returns the split dimension in case the
 * split method is MAX_SPREAD
 *
 * pre assumptions - array, array->pointsArray, array->indicesMatrix are valid
 *
 * @param array - the relevant kd-array to work by
 *
 * @returns the calculated split dimension
 */
int getSplitDimInMaxSpreadMethod(SPKDArray array);

/*
 * The method creates an inner node in the kd-tree according to the given
 * parameters
 *
 * pre assumptions - node is valid
 *
 * @param node - the previously initiated SPKDTreeNode instance
 * @param array - the relevant kd-array to work by
 * @param splitMethod - an enum representing the splitting criteria:
 *   - MAX_SPREAD - uses the dimension that contains the largest difference
 *   between the last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where
 *	 i is the recursive depth.
 * @param recDepth - the ((depth of the recursive - 1) modulo d), used in case
 * the splitMethod is 'INCREMENTAL'
 * @param splitDim - the dimension with which we call the split function
 * on 'array'
 *
 * @returns NULL if calling split function on given 'array' with 'splitDim'
 * as its 'coor' parameter failed or memory allocation failed otherwise
 * returns the inner kd-tree node built according to the given parameters
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDTreeNode createInnerNode(SPKDTreeNode node, SPKDArray array,
		SP_KDTREE_SPLIT_METHOD splitMethod, int recDepth, int splitDim);

/*
 * The method initializes a new kd-tree recursively according to the given
 * kd-array and split method.
 *
 * @param array - the relevant kd-array to work by
 * @param splitMethod - an enum representing the splitting criteria :
 *   - MAX_SPREAD - uses the dimension that contains the largest difference between the
 *   last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where i is the
 *	 recursive depth.
 * @param recDepth - the ((depth of the recursive - 1) modulo d), used in case the
 * splitMethod is 'INCREMENTAL'
 *
 * @returns -
 *  NULL if :
 *   - array is NULL
 *   - memory allocation failed
 *   otherwise returns a pointer to the root of the
 *   kd-tree that is built by the given array and split method.
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDTreeNode internalInitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod,
		int recDepth);

/**
 * Frees all memory resources associated with kdTreeNode.
 * If kdTreeNode == NULL nothing is done.
 *
 * @param kdTreeNode - the SPKDTreeNode instance to destroy
 * @param freePointsData - indicates if the points data (at the leafs)
 * 							should be freed also
 *
 * @logger -
 * in case we try to free a null pointer a relevant warning is logged to the logger
 */
void spKDTreeDestroy(SPKDTreeNode kdTreeNode, bool freePointsData);

/*
 * Returns true iff the given node is a leaf in a kd-tree
 *
 * pre assumptions - treeNode is valid
 *
 * @param treeNode - the given node
 *
 * @returns - true iff the given node is a leaf
 */
bool isLeaf(SPKDTreeNode treeNode);

#endif /* SPKDTREENODE_H_ */
