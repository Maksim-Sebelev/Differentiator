#include <assert.h>
#include "Differentiator.h"
#include "Taylor.h"
#include "../Tree/Tree.h"
#include "../Tree/TreeDump.h"
#include "SimplifyTree.h"
#include "../Tree/TreeDump.h"
#include "MathFunctions.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr CreateNewNode       (const Tree_t* tree, Node_t** node, size_t degree);
static Number  GetTaylorCoeff      (const Tree_t* tree);
static TreeErr SetXnodes           (Node_t* node, Number valX);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr Taylor(const Tree_t* tree, Tree_t* taylor, size_t degree)
{
    assert(tree);
    assert(tree->root);
    assert(taylor);

    TreeErr err = {};

    Tree_t treeCopy = {};
    TREE_ASSERT(NodeCopy(&treeCopy.root, tree->root));
    Number coeff = GetTaylorCoeff(&treeCopy);

    _NUM(&taylor->root, coeff);

    for (size_t degree_i = 1; degree_i <= degree; degree_i++)
    {
        TREE_ASSERT(Diff(&treeCopy));
        TREE_ASSERT(SimplifyTree(&treeCopy));
        TREE_GRAPHIC_DUMP(treeCopy.root);

        Node_t* node = {};
        TREE_ASSERT(CreateNewNode(&treeCopy, &node, degree_i));

        node->left = taylor->root;
        TREE_ASSERT(SwapNode(&node, &taylor->root));
    }

    TREE_ASSERT(TreeDtor(&treeCopy));

    return TREE_VERIF(taylor, err);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr CreateNewNode(const Tree_t* tree, Node_t** node, size_t degree)
{
    COLOR_PRINT(GREEN, "1\n");
    assert(tree);
    assert(tree->root);
    assert(node);

    TreeErr err = {};

    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};

    Node_t* new_right_right_left  = {};
    Node_t* new_right_right_right = {};

    Node_t* new_right_left_left  = {};
    Node_t* new_right_left_right = {};

    Number coeff = GetTaylorCoeff(tree);
    Number fac   = factorial(degree);

    _VAR(&new_right_right_left,  Variable::x);
    _NUM(&new_right_right_right, (Number) degree);

    _NUM(&new_right_left_left,  coeff);
    _NUM(&new_right_left_right, fac);

    _DIV(&new_right_left,  new_right_left_left, new_right_left_right);
    _POW(&new_right_right, new_right_right_left, new_right_right_right);   

    new_left = tree->root;
    _MUL(&new_right, new_right_left, new_right_right);

    _ADD(node, new_left, new_right);

    COLOR_PRINT(RED, "2\n\n");


    return NODE_VERIF(*node, err);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number GetTaylorCoeff(const Tree_t* tree)
{
    assert(tree);

    Tree_t treeCopy = {};
    TREE_ASSERT(NodeCopy(&treeCopy.root, tree->root));


    TREE_ASSERT(SetXnodes(treeCopy.root, 0));
    TREE_ASSERT(SimplifyTree(&treeCopy));

    Number coeff = treeCopy.root->data.num;

    TREE_ASSERT(TreeDtor(&treeCopy));

    return coeff;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
