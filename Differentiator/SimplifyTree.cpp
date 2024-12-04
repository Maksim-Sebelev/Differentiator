#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "SimplifyTree.h"
#include "../Tree/Tree.h"
#include "../Onegin/onegin.h"
#include "../Tree/Tree.h"
#include "../Tree/TreeDump.h"


static TreeErr SimplifyTreeHelper                         (Node_t* node);
static TreeErr SimplifyOperation                          (Node_t* node);

static TreeErr HandleSimplifyMinusWith1NumChild           (Node_t* node);

static TreeErr HandleSimplifyOpearationWith2Num           (Node_t* node);
static TreeErr HandleSimplifyOperationWith0Child          (Node_t* node);

static TreeErr HandleSimplifyLeft0Num                     (Node_t* node);
static TreeErr HandleSimplifyRight0Num                    (Node_t* node);

static TreeErr Create0NumNode                             (Node_t* node);
static TreeErr Create1NumNode                             (Node_t* node);

static TreeErr HandleSimplifyOperationWith1NumChild       (Node_t* node);

static TreeErr HandleSimplifyOperationWith1NumLeftChild   (Node_t* node);
static TreeErr HandleSimplifyOperationWith1NumRightChild  (Node_t* node);

static TreeErr SetNodeLeftChild                           (Node_t* node);
static TreeErr SetNodeRightChild                          (Node_t* node);

static TreeErr SimplifyFunction                           (Node_t* node);

static TreeErr HandleSimplifyLn                           (Node_t* node);
static TreeErr HandleSimplifySin                          (Node_t* node);
static TreeErr HandleSimplifyCos                          (Node_t* node);
static TreeErr HandleSimplifyTg                           (Node_t* node);

static bool    IsArgNum                             (const Node_t* node);
static bool    IsArgOper                            (const Node_t* node);
static bool    HasNode2NumChildren                  (const Node_t* node);
static bool    HasNode0NumChild                     (const Node_t* node);
static bool    IsNum0                               (const Node_t* node);
static bool    IsArgLn                              (const Node_t* node);
static bool    IsArgSin                             (const Node_t* node);
static bool    IsArgCos                             (const Node_t* node);
static bool    IsArgTg                              (const Node_t* node);
static bool    IsOperationSimplifySituationGood     (const Node_t* node);
static bool    IsNodeMinusWith1NumChild             (const Node_t* node);
static bool    HasNode1NumChild                     (const Node_t* node);

static Number  MakeArithmeticOperation             (Number firstOpearnd, Number secondOperand, Operation Operator);
static Number  pow                                 (Number firstOperand, Number secondOperand);

//----------------------------------------------------------------------------------------------------------------------------------

TreeErr SimplifyTree(Tree_t* tree)
{
    assert(tree);

    TreeErr err = {};

    TREE_ASSERT(SimplifyTreeHelper(tree->root));

    return TREE_VERIF(tree, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyTreeHelper(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (node->left)
    {
        TREE_ASSERT(SimplifyTreeHelper(node->left));
    }

    if (node->right)
    {
        TREE_ASSERT(SimplifyTreeHelper(node->right));
    }


    NodeArgType type = node->data.type;

    if (type == NodeArgType::operation)
    {
        TREE_ASSERT(SimplifyOperation(node));
    }

    else if (type == NodeArgType::function)
    {
        TREE_ASSERT(SimplifyFunction(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyOperation(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsNodeMinusWith1NumChild(node))
    {
        TREE_ASSERT(HandleSimplifyMinusWith1NumChild(node));
    }

    else if (HasNode2NumChildren(node))
    {
        TREE_ASSERT(HandleSimplifyOpearationWith2Num(node));
    }

    else if (HasNode0NumChild(node))
    {
        TREE_ASSERT(HandleSimplifyOperationWith0Child(node));
    }

    else if (HasNode1NumChild(node))
    {
        TREE_ASSERT(HandleSimplifyOperationWith1NumChild(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyFunction(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsArgLn(node))
    {
        TREE_ASSERT(HandleSimplifyLn(node));
    }

    else if (IsArgSin(node))
    {
        TREE_ASSERT(HandleSimplifySin(node));
    }

    else if (IsArgCos(node))
    {
        TREE_ASSERT(HandleSimplifyCos(node));
    }

    else if (IsArgTg(node))
    {
        TREE_ASSERT(HandleSimplifyTg(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyMinusWith1NumChild(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(IsArgOper(node));
    assert(node->data.oper == Operation::minus);
    assert(!node->right);
    assert(IsArgNum(node->left));
    assert(!node->left->left);
    assert(!node->left->right);

    TreeErr err = {};

    Number num     = node->left->data.num;
    Number new_num = -num;

    TREE_ASSERT(NodeDtor(node->left));

    _SET_NUM(node, new_num);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyOpearationWith2Num(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsArgNum(node->left));
    assert(IsArgNum(node->right));

    TreeErr err = {};

    if (!IsOperationSimplifySituationGood(node))
    {
        return NODE_VERIF(node, err);
    }

    Operation operation = node->data.oper;

    Number    firstNum  = node->left->data.num;
    Number    secondNum = node->right->data.num;

    Number    result    = MakeArithmeticOperation(firstNum, secondNum, operation);

    NodeDtor(node->left);
    NodeDtor(node->right);

    _SET_NUM(node, result);

    return NODE_VERIF(node, err);    
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyOperationWith0Child(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsNum0(node->left) || IsNum0(node->right));

    TreeErr err = {};

    if (IsNum0(node->left))
    {
        TREE_ASSERT(HandleSimplifyLeft0Num(node));
    }

    else if (IsNum0(node->right))
    {
        TREE_ASSERT(HandleSimplifyRight0Num(node));
    }

    return NODE_VERIF(node, err);    
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyOperationWith1NumChild(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(node->left->data.num == 1 || node->right->data.num == 1);

    TreeErr err = {};

    Number firstNum  = node->left->data.num;
    Number secondNum = node->right->data.num;
    
    if (firstNum == 1)
    {
        TREE_ASSERT(HandleSimplifyOperationWith1NumLeftChild(node));
    }

    else if (secondNum == 1)
    {
        TREE_ASSERT(HandleSimplifyOperationWith1NumRightChild(node));
    }

    return NODE_VERIF(node, err); 
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyOperationWith1NumLeftChild(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(node->left->data.num == 1);

    TreeErr err = {};

    Operation oper = node->data.oper;

    switch (oper)
    {
        case Operation::plus:
        case Operation::minus:
        case Operation::dive:                                                      break;
        case Operation::mul:                 TREE_ASSERT(SetNodeRightChild(node)); break;
        case Operation::power:               TREE_ASSERT(Create1NumNode(node));    break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;   break;
        default: assert(0 && "You forgot about some operation.\n");                break;
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyOperationWith1NumRightChild(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(node->right->data.num == 1);

    TreeErr err = {};

    Operation oper = node->data.oper;

    switch (oper)
    {
        case Operation::plus:
        case Operation::minus:                                                   break;
        case Operation::mul:   TREE_ASSERT(SetNodeLeftChild(node));              break;
        case Operation::dive:  TREE_ASSERT(SetNodeLeftChild(node));              break;
        case Operation::power: TREE_ASSERT(SetNodeLeftChild(node));              break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE; break;
        default: assert(0 && "You forgot about some operation.\n");              break;
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr SetNodeLeftChild(Node_t* node)
{
    assert(node);
    assert(node->left);

    TreeErr err = {};

    Node_t* temp_left  = node->left;
    Node_t* temp_right = node->right;

    TREE_ASSERT(NodeSetCopy(node, node->left));

    TREE_ASSERT(NodeDtor(temp_left));
    if (temp_right) TREE_ASSERT(NodeDtor(temp_right));

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr SetNodeRightChild(Node_t* node)
{
    assert(node);
    assert(node->right);

    TreeErr err = {};

    Node_t* temp_left  = node->left;
    Node_t* temp_right = node->right;

    TREE_ASSERT(NodeSetCopy(node, node->right));

    TREE_ASSERT(NodeDtor(temp_right));


    if (temp_left) TREE_ASSERT(NodeDtor(temp_left));

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyLeft0Num(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsNum0(node->left));

    TreeErr err = {};

    Operation operation = node->data.oper;

    switch (operation)
    {
        case Operation::mul:
        case Operation::dive:
        case Operation::power: TREE_ASSERT(Create0NumNode(node));                                   break;
        case Operation::plus:  TREE_ASSERT(SetNodeRightChild(node));                                break;
        case Operation::minus: TREE_ASSERT(SwapNode(&node->left, &node->right)); FREE(node->left)   break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;                    break;
        default: assert(0 && "You forgot about some operation.\n");                                 break;
    }

    GRAPHIC_DUMP(node);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyRight0Num(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsNum0(node->right));

    TreeErr err = {};

    Operation operation = node->data.oper;

    switch (operation)
    {
        case Operation::plus:  
        case Operation::minus: TREE_ASSERT(SetNodeLeftChild(node));                break;
        case Operation::mul:   TREE_ASSERT(Create0NumNode(node));                  break;
        case Operation::power: TREE_ASSERT(Create1NumNode(node));                  break;
        case Operation::dive:  err.err = TreeErrorType::DIVISION_BY_0;             break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;   break;
        default: assert(0 && "You forgot about some operation.\n");                break;
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyLn(Node_t* node)
{
    assert(node);
    assert(IsArgLn(node));
    assert(node->left);
    assert(!node->right);

    TreeErr err = {};

    NodeArgType type = node->left->data.type;
    Number      num  = node->left->data.num;

    if (type == NodeArgType::number && num == 1)
    {
        TREE_ASSERT(Create0NumNode(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifySin(Node_t* node)
{
    assert(node);
    assert(IsArgSin(node));
    assert(node->left);
    assert(!node->right);

    TreeErr err = {};

    NodeArgType type = node->left->data.type;
    Number      num  = node->left->data.num;

    if (type == NodeArgType::number && num == 0)
    {
        TREE_ASSERT(Create0NumNode(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyCos(Node_t* node)
{
    assert(node);
    assert(IsArgCos(node));
    assert(node->left);
    assert(!node->right);

    TreeErr err = {};

    NodeArgType type = node->left->data.type;
    Number      num  = node->left->data.num;

    if (type == NodeArgType::number && num == 0)
    {
        TREE_ASSERT(Create1NumNode(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleSimplifyTg(Node_t* node)
{
    assert(node);
    assert(IsArgTg(node));
    assert(node->left);
    assert(!node->right);

    TreeErr err = {};

    NodeArgType type = node->left->data.type;
    Number      num  = node->left->data.num;

    if (type == NodeArgType::number && num == 0)
    {
        TREE_ASSERT(Create0NumNode(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr Create0NumNode(Node_t* node)
{
    assert(node);

    TreeErr err = {};

    if (node->left)  TREE_ASSERT(NodeAndUnderTreeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeAndUnderTreeDtor(node->right));

    _SET_NUM(node, 0);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr Create1NumNode(Node_t* node)
{
    assert(node);

    TreeErr err = {};

    if (node->left)  TREE_ASSERT(NodeAndUnderTreeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeAndUnderTreeDtor(node->right));

    _SET_NUM(node, 1);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static Number  MakeArithmeticOperation(Number firstOpearnd, Number secondOperand, Operation Operator)
{
    switch (Operator)
    {
        case Operation::plus:  return firstOpearnd + secondOperand;
        case Operation::minus: return firstOpearnd - secondOperand;
        case Operation::mul:   return firstOpearnd * secondOperand;
        case Operation::power: return pow(firstOpearnd, secondOperand);
        case Operation::dive:
            assert(secondOperand != 0);
            assert((firstOpearnd % secondOperand) == 0);
            return firstOpearnd / secondOperand;
            break;

        case Operation::undefined_operation:
            assert(0 && "Undefined operation.\n");
            break;

        default:
            assert(0 && "You forgot about some operation.\n");
            break;
    }

    assert(0 && "We must not be here.\n");
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgNum(const Node_t* node)
{   
    assert(node);

    return node->data.type == NodeArgType::number;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgOper(const Node_t* node)
{   
    assert(node);

    return node->data.type == NodeArgType::operation;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool HasNode2NumChildren(const Node_t* node)
{
    assert(node);
    assert(node->left);


    return (node->right) && (IsArgNum(node->left) && IsArgNum(node->right));
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsNum0(const Node_t* node)
{
    assert(node);
    return ((node->data.num == 0) && IsArgNum(node));
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgLn(const Node_t* node)
{
    assert(node);

    bool flag = (node->data.func == Function::ln);

    return flag;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgSin(const Node_t* node)
{
    assert(node);

    bool flag = (node->data.func == Function::sin);

    return flag;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgCos(const Node_t* node)
{
    assert(node);

    bool flag = (node->data.func == Function::cos);

    return flag;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgTg(const Node_t* node)
{
    assert(node);

    bool flag = (node->data.func == Function::tg);

    return flag;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool HasNode0NumChild(const Node_t* node)
{
    assert(node);

    return ((node->left && IsNum0(node->left)) || (node->right && IsNum0(node->right)));
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeMinusWith1NumChild(const Node_t* node)
{
    assert(node);
    assert(node->left);
    
    bool flag1 = (node->data.oper == Operation::minus);
    bool flag2 = (IsArgNum(node->left));
    bool flag3 = (!node->right);

    return (flag1 && flag2 && flag3);
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool HasNode1NumChild(const Node_t* node)
{
    assert(node);
    assert(node->left);


    Number leftNum  = 0;
    Number rightNum = 0;

    if (node->left)  leftNum  = node->left ->data.num;
    if (node->right) rightNum = node->right->data.num;

    bool flag1 = ((node->left && IsArgNum(node->left)) || (node->right && IsArgNum(node->right)));
    bool flag2 = ((leftNum == 1)       || (rightNum == 1));

    return (flag1 && flag2);
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationSimplifySituationGood(const Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsArgNum(node->left));
    assert(IsArgNum(node->right));

    Operation oper       = node->data.oper;
    Number    firstNum   = node->left->data.num;
    Number    secondNum  = node->right->data.num;

    bool flag1 = (oper != Operation::dive);

    if (secondNum == 0 && !flag1)
    {   
        assert(0 && "Division by zero.");
    }

    if (secondNum == 0)
    {
        return flag1;
    }

    bool flag2 = (firstNum % secondNum == 0);

    return (flag1 || flag2);
}

//----------------------------------------------------------------------------------------------------------------------------------

static Number pow(Number firstNum, Number secondNum)
{
    assert(secondNum >= 0);

    Number res = 1;
    for (int i = 0; i < secondNum; i++)
    {
        res *= firstNum;
    }
    return res;
}

//----------------------------------------------------------------------------------------------------------------------------------
