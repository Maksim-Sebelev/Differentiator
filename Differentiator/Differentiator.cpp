#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Differentiator.h"
#include "../Tree/Tree.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"
#include "DiffDump.h"


static void PrintError            (const DiffErr* const err);

static DiffErr HandleDiffNum       (Node_t** node);
static DiffErr HandleDiffVar       (Node_t** node);
static DiffErr HandleDiffOperation (Node_t** node);

static DiffErr HandleDiffPlus      (Node_t** node);
static DiffErr HandleDiffMinus     (Node_t** node);
static DiffErr HandleDiffMul       (Node_t** node);
static DiffErr HandleDiffDiv       (Node_t** node);
static DiffErr HandleDiffPow       (Node_t** node);


static DiffErr   NodeCopy         (Node_t** copy, const Node_t* node);

//-------------------------------------------------------------------------------------------------------------------------------------

DiffErr Diff(Node_t** node)
{
    assert(node);

    DiffErr err = {};

    NodeArgType type = (*node)->data.type;
    Node_t* diff_node_operation = {};

    switch (type)
    {
        case NodeArgType::number:
        {   
            DIFF_ASSERT(HandleDiffNum(node));
            return DIFF_VERIF(*node, err);
            break;
        }

        case NodeArgType::variable:
        {
            DIFF_ASSERT(HandleDiffVar(node));
            return DIFF_VERIF(*node, err);
            break;
        }

        case NodeArgType::operation:
        {
            DIFF_ASSERT(HandleDiffOperation(node));
            return DIFF_VERIF(*node, err);
            break;
        }

        default:
        {
            assert(0);
            break;
        }
    }

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffNum(Node_t** node)
{
    assert(node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    (*node)->data.arg = "0";

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffVar(Node_t** node)
{
    assert(node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    (*node)->data.arg = "1";

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffOperation(Node_t** node)
{
    assert(node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    Operation operation_type = GetOperationType((*node)->data.arg);

    switch (operation_type)
    {
        case Operation::plus:
        {
            DIFF_ASSERT(HandleDiffPlus(node));
            return DIFF_VERIF(*node, err);
            break;
        }

        case Operation::minus:
        {
            DIFF_ASSERT(HandleDiffMinus(node));
            return DIFF_VERIF(*node, err);
            break;
        }

        case Operation::mul:
        {
            DIFF_ASSERT(HandleDiffMul(node));

            return DIFF_VERIF(*node, err);
        }

        case Operation::dive:
        {
            DIFF_ASSERT(HandleDiffDiv(node));

            return DIFF_VERIF(*node, err);
        }

        default:
        {
            assert(0);
            break;
        }
    }

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffPlus(Node_t** node)
{
    assert(*node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    DIFF_ASSERT(Diff(&(*node)->left));
    DIFF_ASSERT(Diff(&(*node)->right));

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffMinus(Node_t** node)
{
    assert(node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    DIFF_ASSERT(Diff(&(*node)->left));
    DIFF_ASSERT(Diff(&(*node)->right));


    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffMul(Node_t** node)
{
    assert(node);
    assert(*node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    (*node)->data.arg = "+";

    Node_t* diff_left  = {};
    Node_t* diff_right = {};

    DIFF_ASSERT(NodeCopy(&diff_left,  (*node)->left));
    DIFF_ASSERT(NodeCopy(&diff_right, (*node)->right));

    DIFF_ASSERT(Diff(&diff_left));
    DIFF_ASSERT(Diff(&diff_right));

    Node_t* new_left  = {};
    Node_t* new_right = {};

    TREE_ASSERT(NodeCtor(&new_left,  NodeArgType::operation, "*", diff_left,  (*node)->right));
    TREE_ASSERT(NodeCtor(&new_right, NodeArgType::operation, "*", (*node)->left, diff_right));

    (*node)->left  = new_left;
    (*node)->right = new_right;

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffDiv(Node_t** node)
{
    assert(node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);

    (*node)->data.arg = "/";

    Node_t* diff_left  = {};
    Node_t* diff_right = {};
    
    DIFF_ASSERT(NodeCopy(&diff_left,  (*node)->left));
    DIFF_ASSERT(NodeCopy(&diff_right, (*node)->right));

    DIFF_ASSERT(Diff(&diff_left));
    DIFF_ASSERT(Diff(&diff_right));


    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_left_left  = {};
    Node_t* new_left_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};
    
    DIFF_ASSERT(NodeCopy(&new_right_left, (*node)->right));
    TREE_ASSERT(NodeCtor(&new_right_right, NodeArgType::number, "2", nullptr, nullptr));

    TREE_ASSERT(NodeCtor(&new_left_left , NodeArgType::operation, "*", diff_left,     (*node)->right));
    TREE_ASSERT(NodeCtor(&new_left_right, NodeArgType::operation, "*", (*node)->left, diff_right));

    TREE_ASSERT(NodeCtor(&new_left,  NodeArgType::operation, "-", new_left_left,  new_left_right));
    TREE_ASSERT(NodeCtor(&new_right, NodeArgType::operation, "^", new_right_left, new_right_right));

    (*node)->left  = new_left;
    (*node)->right = new_right;

    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static DiffErr HandleDiffPow(Node_t** node)
{
    assert(node);
    assert(*node);

    DiffErr err = {};
    DIFF_RETURN_IF_ERR(*node, err);



    return DIFF_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

DiffErr DiffVerif(const Node_t* node, DiffErr* err, const char* const file, const int line, const char* const func)
{
    assert(node);
    assert(err) ;
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    NodeArgType type = node->data.type;

    switch (type)
    {
        case NodeArgType::number:
        {
            if (node->left || node->right)
            {
                err->err = DiffErrorType::NUM_HAS_CHILD;
                return *err;
            }
            break;
        }

        case NodeArgType::variable:
        {
            if (node->left || node->right)
            {
                err->err = DiffErrorType::VAR_HAS_CHILD;
                return *err;
            }
            break;
        }

        case NodeArgType::operation:
        {
            if (!node->left && !node->right)
            {
                err->err = DiffErrorType::OPR_HAS_NOT_CHILDS;
                return *err;
            }
            break;
        }

        case NodeArgType::undefined:
        {
            err->err = DiffErrorType::UNDEFINED_NODE_TYPE;
            return *err;
            break;
        }

        default:
        {
            err->err = DiffErrorType::UNDEFINED_NODE_TYPE;
            return *err;
            break;
        }
    }

    return *err;
}

//--------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const DiffErr* const err)
{
    assert(err);

    if (err->err == DiffErrorType::NO_ERR)
    {
        return;
    }

    if (err->err == DiffErrorType::NUM_HAS_CHILD)
    {
        COLOR_PRINT(RED, "Error: Num-type-node has child.\n");
    }

    if (err->err == DiffErrorType::VAR_HAS_CHILD)
    {
        COLOR_PRINT(RED, "Error: Var-type-node has child.\n");
    }

    if (err->err == DiffErrorType::OPR_HAS_NOT_CHILDS)
    {
        COLOR_PRINT(RED, "Error: Oper-type-node hasn't childs.\n");
    }

    if (err->err == DiffErrorType::NODE_COPY_CALLOC_NULL)
    {
        COLOR_PRINT(RED, "Error: In NodeCopy() calloc return nullptr.\n");
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------

void DiffAssertPrint(const DiffErr* const err, const char* const file, const int line, const char* const func)
{
    assert(err);
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(file, line, func);
    PrintError(err);
    PrintPlace(err->place.file, err->place.line, err->place.func);
    COLOR_PRINT(CYAN, "\nabort() in 3, 2, 1...\n");

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------
