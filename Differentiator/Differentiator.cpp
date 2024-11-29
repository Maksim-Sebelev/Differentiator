#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Differentiator.h"
#include "../Tree/Tree.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"
#include "DiffDump.h"

static TreeErr DiffNode                  (Node_t** node);

static TreeErr HandleDiffNum             (Node_t** node);
static TreeErr HandleDiffVar             (Node_t** node);
static TreeErr HandleDiffOperation       (Node_t** node);
static TreeErr HandleDiffFunction        (Node_t** node);

static TreeErr HandleDiffPlus            (Node_t** node);
static TreeErr HandleDiffMinus           (Node_t** node);
static TreeErr HandleDiffMul             (Node_t** node);
static TreeErr HandleDiffDiv             (Node_t** node);
static TreeErr HandleDiffPow             (Node_t** node);

static TreeErr HandleDiffFunctionHelper  (Node_t** node);
static TreeErr HandleDiffLn              (Node_t** node);
static TreeErr HandleDiffSin             (Node_t** node);
static TreeErr HandleDiffCos             (Node_t** node);
static TreeErr HandleDiffTg              (Node_t** node);
static TreeErr HandleDiffCtg             (Node_t** node);
static TreeErr HandleDiffSh              (Node_t** node);
static TreeErr HandleDiffCh              (Node_t** node);
static TreeErr HandleDiffTh              (Node_t** node);
static TreeErr HandleDiffCth             (Node_t** node);

//-------------------------------------------------------------------------------------------------------------------------------------

TreeErr Diff(Tree_t* tree)
{
    assert(tree);

    TreeErr err = {};

    TREE_ASSERT(DiffNode(&tree->root));

    return TREE_VERIF(tree, err);    
}

//-------------------------------------------------------------------------------------------------------------------------------------

static TreeErr DiffNode(Node_t** node)
{
    assert(node);

    TreeErr err = {};

    NodeArgType type = (*node)->data.type;

    switch (type)
    {
        case NodeArgType::number:
        {   
            TREE_ASSERT(HandleDiffNum(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case NodeArgType::variable:
        {
            TREE_ASSERT(HandleDiffVar(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case NodeArgType::operation:
        {
            TREE_ASSERT(HandleDiffOperation(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case NodeArgType::function:
        {
            TREE_ASSERT(HandleDiffFunction(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case NodeArgType::undefined:
        {
            err.err = UNDEFINED_NODE_TYPE;
            return NODE_VERIF(*node, err);
        }

        default:
        {
            assert(0);
            break;
        }
    }

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffNum(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    _SET_NUM(*node, 0);
    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffVar(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    _SET_NUM(*node, 1);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffOperation(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Operation operation_type = (*node)->data.oper;

    switch (operation_type)
    {
        case Operation::plus:
        {
            TREE_ASSERT(HandleDiffPlus(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Operation::minus:
        {
            TREE_ASSERT(HandleDiffMinus(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Operation::mul:
        {
            TREE_ASSERT(HandleDiffMul(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Operation::dive:
        {
            TREE_ASSERT(HandleDiffDiv(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Operation::power:
        {
            TREE_ASSERT(HandleDiffPow(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Operation::undefined_operation:
        {
            err.err = TreeErrorType::UNDEFINED_OPERATION_TYPE;
            return NODE_VERIF(*node, err);
            break;
        }

        default:
        {
            assert(0 && "You forgot abour some operation.\n");
            break;
        }
    }

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffPlus(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    TREE_ASSERT(DiffNode(&(*node)->left));
    TREE_ASSERT(DiffNode(&(*node)->right));

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffMinus(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    TREE_ASSERT(DiffNode(&(*node)->left));

    if ((*node)->right)
    {
        TREE_ASSERT(DiffNode(&(*node)->right));
    }

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffMul(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    _SET_OPER_ONLY(*node, plus);

    Node_t* diff_left  = {};
    Node_t* diff_right = {};

    TREE_ASSERT(NodeCopy(&diff_left,  (*node)->left));
    TREE_ASSERT(NodeCopy(&diff_right, (*node)->right));

    TREE_ASSERT(DiffNode(&diff_left));
    TREE_ASSERT(DiffNode(&diff_right));

    Node_t* new_left  = {};
    Node_t* new_right = {};

    _OPER(&new_left,  Operation::mul, diff_left,     (*node)->right);
    _OPER(&new_right, Operation::mul, (*node)->left, diff_right);

    (*node)->left  = new_left;
    (*node)->right = new_right;

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffDiv(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);


    Node_t* diff_left  = {};
    Node_t* diff_right = {};
    
    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_left_left  = {};
    Node_t* new_left_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};


    _SET_OPER_ONLY(*node, Operation::dive);

    TREE_ASSERT(NodeCopy(&diff_left,  (*node)->left));
    TREE_ASSERT(NodeCopy(&diff_right, (*node)->right));
    TREE_ASSERT(DiffNode(&diff_left));
    TREE_ASSERT(DiffNode(&diff_right));
    

    TREE_ASSERT(NodeCopy(&new_right_left, (*node)->right));
    _NUM                (&new_right_right, 2);


    _OPER(&new_left_left,  Operation::mul, diff_left,     (*node)->right);
    _OPER(&new_left_right, Operation::mul, (*node)->left, diff_right);


    _OPER(&new_left, Operation::minus, new_left_left,  new_left_right);
    _OPER(&new_right, Operation::power, new_right_left, new_right_right);

    (*node)->left  = new_left;
    (*node)->right = new_right;

    // GRAPHIC_DUMP(*node);
    // TEXT_DUMP(*node);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffPow(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    RETURN_IF_FALSE(*node, err);

    Node_t* new_left  = {}; // ^ f g
    Node_t* new_right = {}; // + * *

    Node_t* new_left_left  = {}; // f nullptr nullptr
    Node_t* new_left_right = {}; // g nullptr nullptr

    Node_t* new_right_left  = {}; // * g' ln
    Node_t* new_right_right = {}; // * f' /

    Node_t* new_right_left_left  = {}; // g'
    Node_t* new_right_left_right = {}; // ln f       nullptr

    Node_t* new_right_right_left  = {}; // f'
    Node_t* new_right_right_right = {}; // / g f'

    Node_t* new_right_left_right_left  = {}; // f nullptr

    Node_t* new_right_right_right_left  = {}; // g
    Node_t* new_right_right_right_right = {}; // f


    TREE_ASSERT(NodeCopy(&new_right_right_right_left,  (*node)->right));
    TREE_ASSERT(NodeCopy(&new_right_right_right_right, (*node)->left));


    TREE_ASSERT(NodeCopy(&new_right_left_right_left, (*node)->left));

    TREE_ASSERT(NodeCopy(&new_right_left_left, (*node)->right));
    TREE_ASSERT(DiffNode(&new_right_left_left));
    _FUNC(&new_right_left_right, Function::ln, new_right_left_right_left);


    TREE_ASSERT(NodeCopy(&new_right_right_left, (*node)->left));
    TREE_ASSERT(DiffNode(&new_right_right_left));
    _OPER(&new_right_right_right, Operation::dive, new_right_right_right_left, new_right_right_right_right);

    new_left_left  = (*node)->left;
    new_left_right = (*node)->right;

    _OPER(&new_right_left,  Operation::mul, new_right_left_left,  new_right_left_right);
    _OPER(&new_right_right, Operation::mul, new_right_right_left, new_right_right_right);

    _OPER(&new_left,  Operation::power, new_left_left,  new_left_right);
    _OPER(&new_right, Operation::plus, new_right_left, new_right_right);

    _SET_OPER(*node, Operation::mul, new_left, new_right);;

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffFunction(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    TREE_ASSERT(NodeCopy(&new_left, *node));
    TREE_ASSERT(HandleDiffFunctionHelper(&new_left));
    TREE_ASSERT(DiffNode(&(*node)->left));
    new_right = (*node)->left;

    _SET_OPER(*node, Operation::mul, new_left, new_right);
    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffFunctionHelper(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Function function = (*node)->data.func;

    switch (function)
    {
        case Function::ln:
        {
            TREE_ASSERT(HandleDiffLn(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::sin:
        {
            TREE_ASSERT(HandleDiffSin(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::cos:
        {
            TREE_ASSERT(HandleDiffCos(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::tg:
        {
            TREE_ASSERT(HandleDiffTg(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::ctg:
        {
            TREE_ASSERT(HandleDiffCtg(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::sh:
        {
            TREE_ASSERT(HandleDiffSh(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::ch:
        {
            TREE_ASSERT(HandleDiffCh(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::th:
        {
            TREE_ASSERT(HandleDiffTh(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::cth:
        {
            TREE_ASSERT(HandleDiffCth(node));
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::arcsin:
        {
            assert(0 && "you forgot to add thia function in switch.\n");
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::arccos:
        {
            assert(0 && "you forgot to add thia function in switch.\n");
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::arctg:
        {
            assert(0 && "you forgot to add thia function in switch.\n");
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::arcctg:
        {
            assert(0 && "you forgot to add thia function in switch.\n");
            return NODE_VERIF(*node, err);
            break;
        }

        case Function::undefined_function:
        {
            err.err = UNDEFINED_FUNCTION_TYPE;
            return NODE_VERIF(*node, err);
            break;
        }

        default:
        {
            assert(0);
            break;
        }
    }

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffLn(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    Node_t* new_left  = {};
    Node_t* new_right = {};

    _NUM(&new_left, 1);
    new_right = (*node)->left;

    _SET_OPER(*node, Operation::dive, new_left, new_right);
    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffSin(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    _SET_FUNC_ONLY(*node, Function::cos);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffCos(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    new_right = nullptr;

    Node_t* new_left_left  = {};

    new_left_left  = (*node)->left;

    _FUNC(&new_left, Function::sin, new_left_left);

    _SET_OPER(*node, Operation::minus, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffTg(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};

    Node_t* new_right_left_left  = {};

    new_right_left_left = (*node)->left;

    _FUNC(&new_right_left, Function::cos, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _OPER(&new_right, Operation::power, new_right_left, new_right_right);

    _SET_OPER(*node, Operation::dive, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffCtg(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};

    Node_t* new_left_left_left  = {};

    Node_t* new_right_left_left  = {};

    _NUM(&new_left_left_left, 1);

    new_right_left_left = (*node)->left;
    _FUNC(&new_right_left, Function::sin, new_right_left_left);
    _NUM(&new_right_right, 2);

    _OPER(&new_left, Operation::minus,  new_left_left_left, nullptr);
    _OPER(&new_right, Operation::power, new_right_left,     new_right_right);

    _SET_OPER(*node, Operation::dive, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffSh(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    _SET_FUNC_ONLY(*node, Function::ch);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffCh(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    _SET_FUNC_ONLY(*node, Function::sh);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffTh(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};

    Node_t* new_right_left_left  = {};

    new_right_left_left = (*node)->left;

    _FUNC(&new_right_left, Function::ch, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _OPER(&new_right, Operation::power, new_right_left, new_right_right);

    _SET_OPER(*node, Operation::dive, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffCth(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {};
    Node_t* new_right = {};

    Node_t* new_right_left  = {};
    Node_t* new_right_right = {};

    Node_t* new_right_left_left  = {};

    new_right_left_left = (*node)->left;

    _FUNC(&new_right_left, Function::sh, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _OPER(&new_right, Operation::power, new_right_left, new_right_right);

    _SET_OPER(*node, Operation::dive, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------
