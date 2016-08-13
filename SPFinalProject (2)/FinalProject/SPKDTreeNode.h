//TODO - verify the name SPKDTreeNode is OK (and not KDTreeNode)

#ifndef SPKDTREENODE_H_
#define SPKDTREENODE_H_

#include "SPPoint.h"
#include "SPKDArray.h"
#include "SPConfig.h"

/*
 * A structure used to represent a KD tree node,
 * contains splitting dimension as 'dim',
 * median value as 'val',
 * left son as 'kdtLeft',
 * right son as 'kdtRight'
 * and relevant data as 'data'
 */
struct sp_kd_tree_node {
	int dim;
	int val;
	SPKDTreeNode kdtLeft;
	SPKDTreeNode kdtRight;
	SPPoint data;
};

/*
 * A pointer to the sp_kd_tree_node structure
 */
typedef struct sp_kd_tree_node* SPKDTreeNode;

/*
 * The method initializes a new kd-tree recursively according to the given
 * kd-array and split method.
 *
 * @param array - the relevant kd-array to work by
 * @param splitMethod - an enum representing the splitting criteria :
 *   - MAX_SPREAD - uses the dimension that contains the largest difference between the last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where i is the recursive depth.
 * @returns -
 *  NULL if :
 *   - array is NULL
 *   - memory allocation failed
 *   - //TODO - verify no other errors in this process
 *   otherwise returns a pointer to the root of the
 *   kd-tree that is built by the given array and split method.
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDTreeNode InitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod);

/*
 * The method initializes a new kd-tree recursively according to the given
 * kd-array and split method.
 *
 * @param array - the relevant kd-array to work by
 * @param splitMethod - an enum representing the splitting criteria :
 *   - MAX_SPREAD - uses the dimension that contains the largest difference between the last and first items (when sorted)
 *   - RANDOM - choose a random dimension
 *	 - INCREMENTAL - the splitting dimension of the upper level is i%d where i is the recursive depth.
 * @param recDepth - the ((depth of the recursive - 1) modulo d), used in case the splitMethod is 'INCREMENTAL'
 *
 * @returns -
 *  NULL if :
 *   - array is NULL
 *   - memory allocation failed
 *   - //TODO - verify no other errors in this process
 *   otherwise returns a pointer to the root of the
 *   kd-tree that is built by the given array and split method.
 *
 * @logger -
 * in case of any type of failure the relevant error is logged to the logger
 */
SPKDTreeNode internalInitKDTree(SPKDArray array, SP_KDTREE_SPLIT_METHOD splitMethod, int recDepth);

// TODO - documentation
void spKDTreeDestroy(SPKDTreeNode kdTreeNode);


#endif /* SPKDTREENODE_H_ */
