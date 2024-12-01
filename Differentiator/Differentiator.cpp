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

#define _L (*node)->left
#define _R (*node)->right

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
        case NodeArgType::number:    TREE_ASSERT(HandleDiffNum(node));       break;
        case NodeArgType::variable:  TREE_ASSERT(HandleDiffVar(node));       break;
        case NodeArgType::operation: TREE_ASSERT(HandleDiffOperation(node)); break;
        case NodeArgType::function:  TREE_ASSERT(HandleDiffFunction(node));  break;
        case NodeArgType::undefined: err.err = UNDEFINED_NODE_TYPE;          break;
        default: assert(0 && "you forgot about some operation.\n");          break;
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
        case Operation::plus:   TREE_ASSERT(HandleDiffPlus(node));                              break;
        case Operation::minus:  TREE_ASSERT(HandleDiffMinus(node));                             break;
        case Operation::mul:    TREE_ASSERT(HandleDiffMul(node));                               break;
        case Operation::dive:   TREE_ASSERT(HandleDiffDiv(node));                               break;
        case Operation::power:  TREE_ASSERT(HandleDiffPow(node));                               break;
        case Operation::undefined_operation: err.err = TreeErrorType::UNDEFINED_OPERATION_TYPE; break;
        default: assert(0 && "You forgot abour some operation.\n");                             break;
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

    TREE_ASSERT(DiffNode(&_L));
    TREE_ASSERT(DiffNode(&_R));

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffMinus(Node_t** node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    TREE_ASSERT(DiffNode(&_L));

    if (_R)
    {
        TREE_ASSERT(DiffNode(&_R));
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

    _SET_ADD_ONLY(*node);

    Node_t* diff_left  = {};
    Node_t* diff_right = {};

    TREE_ASSERT(NodeCopy(&diff_left,  _L));
    TREE_ASSERT(NodeCopy(&diff_right, _R));

    TREE_ASSERT(DiffNode(&diff_left));
    TREE_ASSERT(DiffNode(&diff_right));

    Node_t* new_left  = {};
    Node_t* new_right = {};


    _MUL(&new_left, diff_left, _R);
    _MUL(&new_right, _L, diff_right);

    _L  = new_left;
    _R = new_right;

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

    _SET_DIV_ONLY(*node);

    TREE_ASSERT(NodeCopy(&diff_left,  _L));
    TREE_ASSERT(NodeCopy(&diff_right, _R));
    TREE_ASSERT(DiffNode(&diff_left));
    TREE_ASSERT(DiffNode(&diff_right));
    

    TREE_ASSERT(NodeCopy(&new_right_left, _R));
    _NUM                (&new_right_right, 2);

    _MUL(&new_left_left,  diff_left,     _R);
    _MUL(&new_left_right, _L, diff_right);

    _SUB(&new_left,  new_left_left,  new_left_right);
    _POW(&new_right, new_right_left, new_right_right);

    _L  = new_left;
    _R = new_right;

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


    TREE_ASSERT(NodeCopy(&new_right_right_right_left,  _R));
    TREE_ASSERT(NodeCopy(&new_right_right_right_right, _L));


    TREE_ASSERT(NodeCopy(&new_right_left_right_left, _L));

    TREE_ASSERT(NodeCopy(&new_right_left_left, _R));
    TREE_ASSERT(DiffNode(&new_right_left_left));
    _FUNC(&new_right_left_right, Function::ln, new_right_left_right_left);


    TREE_ASSERT(NodeCopy(&new_right_right_left, _L));
    TREE_ASSERT(DiffNode(&new_right_right_left));
    _DIV(&new_right_right_right, new_right_right_right_left, new_right_right_right_right);

    new_left_left  = _L;
    new_left_right = _R;

    _MUL(&new_right_left,  new_right_left_left,  new_right_left_right);
    _MUL(&new_right_right, new_right_right_left, new_right_right_right);

    _POW(&new_left,  new_left_left,  new_left_right);
    _ADD(&new_right, new_right_left, new_right_right);

    _SET_MUL(*node,  new_left,        new_right);

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
    TREE_ASSERT(DiffNode(&_L));
    new_right = _L;

    _SET_MUL(*node, new_left, new_right);

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
        case Function::ln:       TREE_ASSERT(HandleDiffLn(node));                                  break;
        case Function::sin:      TREE_ASSERT(HandleDiffSin(node));                                 break;
        case Function::cos:      TREE_ASSERT(HandleDiffCos(node));                                 break;
        case Function::tg:       TREE_ASSERT(HandleDiffTg(node));                                  break;
        case Function::ctg:      TREE_ASSERT(HandleDiffCtg(node));                                 break;
        case Function::sh:       TREE_ASSERT(HandleDiffSh(node));                                  break;
        case Function::ch:       TREE_ASSERT(HandleDiffCh(node));                                  break;
        case Function::th:       TREE_ASSERT(HandleDiffTh(node));                                  break;
        case Function::cth:      TREE_ASSERT(HandleDiffCth(node));                                 break;
        case Function::arcsin:   assert(0 && "you forgot to add thia function in switch.\n");      break;
        case Function::arccos:   assert(0 && "you forgot to add thia function in switch.\n");      break;
        case Function::arctg:    assert(0 && "you forgot to add thia function in switch.\n");      break;
        case Function::arcctg:   assert(0 && "you forgot to add thia function in switch.\n");      break;
        case Function::undefined_function: err.err = UNDEFINED_FUNCTION_TYPE;                      break;
        default: assert(0);                                                                        break;
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
    new_right = _L;

    _SET_DIV(*node, new_left, new_right);

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

    new_left_left  = _L;

    _FUNC(&new_left, Function::sin, new_left_left);

    _SET_SUB(*node, new_left, new_right);

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

    new_right_left_left = _L;

    _FUNC(&new_right_left, Function::cos, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _POW(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

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

    new_right_left_left = _L;
    _FUNC(&new_right_left, Function::sin, new_right_left_left);
    _NUM(&new_right_right, 2);

    _SUB(&new_left,  new_left_left_left, nullptr);
    _POW(&new_right, new_right_left,     new_right_right);

    _SET_DIV(*node, new_left, new_right);

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

    new_right_left_left = _L;

    _FUNC(&new_right_left, Function::ch, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _POW(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

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

    new_right_left_left = _L;

    _FUNC(&new_right_left, Function::sh, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _POW(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

#undef _L
#undef _R
