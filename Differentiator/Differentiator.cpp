#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Differentiator.h"
#include "../Tree/Tree.h"
#include "../Tree/TreeDump.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"

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
static TreeErr HandleDiffSqrt            (Node_t** node);
static TreeErr HandleDiffSin             (Node_t** node);
static TreeErr HandleDiffCos             (Node_t** node);
static TreeErr HandleDiffTg              (Node_t** node);
static TreeErr HandleDiffCtg             (Node_t** node);
static TreeErr HandleDiffSh              (Node_t** node);
static TreeErr HandleDiffCh              (Node_t** node);
static TreeErr HandleDiffTh              (Node_t** node);
static TreeErr HandleDiffCth             (Node_t** node);
static TreeErr HandleDiffArcsin          (Node_t** node);
static TreeErr HandleDiffArccos          (Node_t** node);
static TreeErr HandleDiffArctg           (Node_t** node);
static TreeErr HandleDiffArcctg          (Node_t** node);


static bool IsNodeConst(const Node_t* node);

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

    NodeArgType type = (*node)->type;

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

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffPow(Node_t** node)
{
    assert(node);
    assert(*node);
    assert(_L);
    assert(_R);
    assert((*node)->type == NodeArgType::operation);
    assert((*node)->data.oper == Operation::power);

    TreeErr err = {};
    RETURN_IF_FALSE(*node, err);

    if (IsNodeConst(_R))
    {
        Node_t* new_left  = {}; // *
        Node_t* new_right = {}; // (x)'

        Node_t* new_left_left  = {}; // num
        Node_t* new_left_right = {}; // ^

        Node_t* new_left_right_left  = {}; // x
        Node_t* new_left_right_right = {}; // -
        
        Node_t* new_left_right_right_left  = {}; // num
        Node_t* new_left_right_right_right = {}; // 1
    

        Node_t* number = _R;

        new_left_right_right_left = number;
        _NUM(&new_left_right_right_right, 1);

        new_left_right_left = _L;
        _SUB(&new_left_right_right, new_left_right_right_left, new_left_right_right_right);


        TREE_ASSERT(NodeCopy(&new_left_left, number));
        _POW(&new_left_right, new_left_right_left, new_left_right_right);


        TREE_ASSERT(NodeCopy(&new_right, _L));
        TREE_ASSERT(DiffNode(&new_right));
        _MUL(&new_left, new_left_left, new_left_right);


        _SET_MUL(*node, new_left, new_right);

        return NODE_VERIF(*node, err);
    }

    Node_t* new_left  = {}; // ^ f g
    Node_t* new_right = {}; // + * *

    Node_t* new_left_left  = {}; // f nullptr nullptr
    Node_t* new_left_right = {}; // g nullptr nullptr

    Node_t* new_right_left  = {}; // * g' Ln
    Node_t* new_right_right = {}; // * f' /

    Node_t* new_right_left_left  = {}; // g'
    Node_t* new_right_left_right = {}; // Ln f       nullptr

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
    _FUNC(&new_right_left_right, Function::Ln, new_right_left_right_left);

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
        case Function::Sqrt:     TREE_ASSERT(HandleDiffSqrt  (node));                                break;
        case Function::Ln:       TREE_ASSERT(HandleDiffLn    (node));                                break;
        case Function::Sin:      TREE_ASSERT(HandleDiffSin   (node));                                break;
        case Function::Cos:      TREE_ASSERT(HandleDiffCos   (node));                                break;
        case Function::Tg:       TREE_ASSERT(HandleDiffTg    (node));                                break;
        case Function::Ctg:      TREE_ASSERT(HandleDiffCtg   (node));                                break;
        case Function::Sh:       TREE_ASSERT(HandleDiffSh    (node));                                break;
        case Function::Ch:       TREE_ASSERT(HandleDiffCh    (node));                                break;
        case Function::Th:       TREE_ASSERT(HandleDiffTh    (node));                                break;
        case Function::Cth:      TREE_ASSERT(HandleDiffCth   (node));                                break;
        case Function::Arcsin:   TREE_ASSERT(HandleDiffArcsin(node));                                break;
        case Function::Arccos:   TREE_ASSERT(HandleDiffArccos(node));                                break;
        case Function::Arctg:    TREE_ASSERT(HandleDiffArctg (node));                                break;
        case Function::Arcctg:   TREE_ASSERT(HandleDiffArcctg(node));                                break;
        case Function::undefined_function: err.err = UNDEFINED_FUNCTION_TYPE;                        break;
        default: assert(0 && "You forgpt about some function.\n");                                   break;
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

static TreeErr HandleDiffSqrt(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    Node_t* new_left  = {}; // 1
    Node_t* new_right = {}; // *
    
    Node_t* new_right_left  = {}; // 2
    Node_t* new_right_right = {}; // Sqrt

    Node_t* new_right_right_left  = {}; // x


    new_right_right_left = _L;

    _NUM  (&new_right_left,  2);
    _FUNC (&new_right_right, Function::Sqrt, new_right_right_left);

    _NUM(&new_left, 1);
    _MUL(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffSin(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    _SET_FUNC_ONLY(*node, Function::Cos);

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

    _FUNC(&new_left, Function::Sin, new_left_left);

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

    _FUNC(&new_right_left, Function::Cos, new_right_left_left);
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
    _FUNC(&new_right_left, Function::Sin, new_right_left_left);
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

    _SET_FUNC_ONLY(*node, Function::Ch);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffCh(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};

    _SET_FUNC_ONLY(*node, Function::Sh);

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

    _FUNC(&new_right_left, Function::Ch, new_right_left_left);
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

    _FUNC(&new_right_left, Function::Sh, new_right_left_left);
    _NUM(&new_right_right, 2);

    _NUM(&new_left, 1);
    _POW(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffArcsin(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {}; // 1
    Node_t* new_right = {}; // Sqrt (  - )

    Node_t* new_right_left = {}; // - ( 1  ^ ) 

    Node_t* new_right_left_left  = {}; // 1
    Node_t* new_right_left_right = {}; // ^

    Node_t* new_right_left_right_left  = {}; // x
    Node_t* new_right_left_right_right = {}; // 2


    new_right_left_right_left = _L;
    _NUM(&new_right_left_right_right, 2);

    _NUM(&new_right_left_left, 1);
    _POW(&new_right_left_right, new_right_left_right_left, new_right_left_right_right);

    _SUB(&new_right_left, new_right_left_left, new_right_left_right);

    _NUM(&new_left, 1);
    _FUNC(&new_right, Function::Sqrt, new_right_left);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffArccos(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {}; // - ( 1 )
    Node_t* new_right = {}; // Sqrt (  - )

    Node_t* new_left_left = {}; // 1

    Node_t* new_right_left = {}; // - ( 1  ^ ) 

    Node_t* new_right_left_left  = {}; // 1
    Node_t* new_right_left_right = {}; // ^

    Node_t* new_right_left_right_left  = {}; // x
    Node_t* new_right_left_right_right = {}; // 2


    new_right_left_right_left = _L;
    _NUM(&new_right_left_right_right, 2);

    _NUM(&new_right_left_left, 1);
    _POW(&new_right_left_right, new_right_left_right_left, new_right_left_right_right);

    _NUM(&new_left_left, 1);
    _SUB(&new_right_left, new_right_left_left, new_right_left_right);

    _SUB(&new_left, new_left_left, nullptr);
    _FUNC(&new_right, Function::Sqrt, new_right_left);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffArctg(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {}; // 1
    Node_t* new_right = {}; // +

    Node_t* new_right_left  = {}; // 1
    Node_t* new_right_right = {}; // ^

    Node_t* new_right_right_left  = {}; // x
    Node_t* new_right_right_right = {}; // 2


    new_right_right_left = _L;
    _NUM(&new_right_right_right, 2);

    _NUM(&new_right_left, 1);
    _POW(&new_right_right, new_right_right_left, new_right_right_right);

    _NUM(&new_left, 1);
    _ADD(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleDiffArcctg(Node_t** node)
{
    assert(node);
    assert(*node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    Node_t* new_left  = {}; // - ( 1 )
    Node_t* new_right = {}; // +

    Node_t* new_left_left = {}; // 1

    Node_t* new_right_left  = {}; // 1
    Node_t* new_right_right = {}; // ^

    Node_t* new_right_right_left  = {}; // x
    Node_t* new_right_right_right = {}; // 2


    new_right_right_left = _L;
    _NUM(&new_right_right_right, 2);

    _NUM(&new_left_left, 1);

    _NUM(&new_right_left, 1);
    _POW(&new_right_right, new_right_right_left, new_right_right_right);

    _SUB(&new_left, new_left_left, nullptr);
    _ADD(&new_right, new_right_left, new_right_right);

    _SET_DIV(*node, new_left, new_right);

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeConst(const Node_t* node)
{
    assert(node);

    if (node->type == NodeArgType::variable) return false;

    if (node->left)
    {
        RETURN_IF_FALSE(IsNodeConst(node->left), false);
    }

    if (node->right)
    {
        RETURN_IF_FALSE(IsNodeConst(node->right), false);
    }

    return true;
}


#undef _L
#undef _R
