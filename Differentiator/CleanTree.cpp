#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "CleanTree.h"
#include "../Tree/Tree.h"
#include "../Onegin/onegin.h"


static TreeErr CleanTreeHelper  (Node_t** node);

typedef int TreeElemIntArg;

//----------------------------------------------------------------------------------------------------------------------------------

TreeErr CleanTree(Tree_t* tree)
{
    assert(tree);

    TreeErr err = {};


    return TREE_VERIF(tree, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanTreeHelper(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    NodeArgType type = (*node)->data.type;

    switch (type)
    {

    default:
        break;
    }

    return NODE_VERIF(*node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanOperation(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Operation operation = GetOperationType((*node)->data.arg);

    switch (operation)
    {
    
    default:
        break;
    }

    return NODE_VERIF(*node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanPlus(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);
    const char* arg_left  = (*node)->left-> data.arg;
    const char* arg_right = (*node)->right->data.arg;

    if (IsArgInt((*node)->left) && IsArgInt((*node)->right))
    {
        TreeElemIntArg argl = strtoi(arg_left);    
        TreeElemIntArg argr = strtoi(arg_right);

        TREE_ASSERT(NodeDtor(&(*node)->left));
        TREE_ASSERT(NodeDtor(&(*node)->right));



    }


    return NODE_VERIF(*node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgInt(Node_t* node)
{   
    assert(node);

    const char* arg = node->data.arg;
    char* argEnd = nullptr;

    strtol(arg, &argEnd, 10);
    assert(argEnd);

    return((int) strlen(arg) == argEnd - arg);
}

//----------------------------------------------------------------------------------------------------------------------------------

static