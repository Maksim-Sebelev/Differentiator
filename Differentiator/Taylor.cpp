#include <assert.h>
#include "Taylor.h"
#include "../Tree/Tree.h"
#include "../Tree/TreeDump.h"
#include "SimplifyTree.h"

static TreeErr SetXnodes(Node_t* node, Number valX);

//--------------------------------------------------------------------------------------------------------------------------------------

TreeErr CalcTree(Tree_t* tree, Number valX)
{
    assert(tree);
    assert(tree->root);

    TreeErr err = {};

    TREE_ASSERT(SetXnodes(tree->root, valX));
    TREE_ASSERT(SimplifyTree(tree));

    return TREE_VERIF(tree, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SetXnodes(Node_t* node, Number valX)
{
    assert(node);

    TreeErr err = {};

    if (node->left)  TREE_ASSERT(SetXnodes(node->left,  valX));
    if (node->right) TREE_ASSERT(SetXnodes(node->right, valX));

    NodeArgType type = node->type;

    if (type == NodeArgType::variable)
    {
        _SET_NUM(node, valX);
    }

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

