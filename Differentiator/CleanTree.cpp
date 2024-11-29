#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "CleanTree.h"
#include "../Tree/Tree.h"
#include "../Onegin/onegin.h"
#include "DiffDump.h"

static TreeErr CleanTreeHelper                      (Node_t* node);
static TreeErr CleanOperation                       (Node_t* node, bool* WasChange);

static TreeErr HandleCleanMinusWith1NumChild        (Node_t* node);

static TreeErr HandleCleanOpearationWith2Num        (Node_t* node);
static TreeErr HandleCleanOpearationWith0Child      (Node_t* node);

static TreeErr HandleCleanLeft0                     (Node_t* node);
static TreeErr HandleCleanRight0Num                 (Node_t* node);

static TreeErr Create0NumNode                          (Node_t* node);
static TreeErr Create1NumNode                          (Node_t* node);



static bool    IsArgNum                             (const Node_t* node);
static bool    IsArgOper                            (const Node_t* node);
static bool    IsArgFunc                            (const Node_t* node);
static bool    IsArgVar                             (const Node_t* node);
static bool    HasNode2NumChildren                  (const Node_t* node);
static bool    HasNode0NumChild                        (const Node_t* node);
static bool    IsNum0                               (const Node_t* node);
static bool    IsOperationCleanSituationGood        (const Node_t* node);
static bool    IsNodeMinusWith1NumChild             (const Node_t* node);

static Number  MakeArithmeticOperation              (Number firstOpearnd, Number secondOperand, Operation Operator);
static Number  pow                                  (Number firstOperand, Number secondOperand);

//----------------------------------------------------------------------------------------------------------------------------------

TreeErr CleanTree(Tree_t* tree)
{
    assert(tree);

    TreeErr err = {};

    TREE_ASSERT(CleanTreeHelper(tree->root));

    return TREE_VERIF(tree, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanTreeHelper(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (node->left)
    {
        TREE_ASSERT(CleanTreeHelper(node->left));
    }

    if (node->right)
    {
        TREE_ASSERT(CleanTreeHelper(node->right));
    }


    NodeArgType type = node->data.type;
    bool WasChange = false;

    if (type == NodeArgType::operation)
    {
        TREE_ASSERT(CleanOperation(node, &WasChange));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanOperation(Node_t* node, bool* WasChange)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsNodeMinusWith1NumChild(node))
    {
        TREE_ASSERT(HandleCleanMinusWith1NumChild(node));
        *WasChange = true;
    }

    else if (HasNode2NumChildren(node))
    {
        TREE_ASSERT(HandleCleanOpearationWith2Num(node));
        *WasChange = true;
    }

    else if (HasNode0NumChild(node))
    {
        TREE_ASSERT(HandleCleanOpearationWith0Child(node));
        *WasChange = true;
    }


    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanMinusWith1NumChild(Node_t* node)
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

static TreeErr HandleCleanOpearationWith2Num(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsArgNum(node->left));
    assert(IsArgNum(node->right));

    TreeErr err = {};
    TEXT_DUMP(node);

    if (!IsOperationCleanSituationGood(node))
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

static TreeErr HandleCleanOpearationWith0Child(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsArgOper(node));
    assert(IsNum0(node->left) || IsNum0(node->right));

    TreeErr err = {};

    if (IsNum0(node->left))
    {
        TREE_ASSERT(HandleCleanLeft0(node));
    }

    else if (IsNum0(node->right))
    {
        TREE_ASSERT(HandleCleanRight0Num(node));
    }

    return NODE_VERIF(node, err);    
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanLeft0(Node_t* node)
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
        case Operation::plus:
        {
            TREE_ASSERT(NodeDtor(node->left));
            Node_t* temp = node->right;
            TREE_ASSERT(NodeSetCopy(node, node->right));
            TREE_ASSERT(NodeDtor(temp));
            break;
        }

        case Operation::minus:
        {
            TREE_ASSERT(NodeDtor(node->left));
            break;
        }

        case Operation::mul:
        {
            TREE_ASSERT(Create0NumNode(node));
            break;
        }

        case Operation::dive:
        {
            TREE_ASSERT(Create0NumNode(node));
            break;
        }

        case Operation::power:
        {
            TREE_ASSERT(Create0NumNode(node));
            break;
        }

        case Operation::undefined_operation:
        {
            err.err = UNDEFINED_OPERATION_TYPE;
            return NODE_VERIF(node, err);
            break;
        }

        default:
        {
            assert(0 && "You forgot about some operation in handleCleanLeft0.\n");
            break;
        }
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanRight0Num(Node_t* node)
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
        {

            TREE_ASSERT(NodeDtor(node->right));
            Node_t* temp = node->left;
            TREE_ASSERT(NodeSetCopy(node, node->left));
            TREE_ASSERT(NodeDtor(temp));
            break;
        }

        case Operation::minus:
        {
            TREE_ASSERT(NodeDtor(node->right));
            node->right = nullptr;
            break;
        }

        case Operation::mul:
        {
            TREE_ASSERT(Create0NumNode(node));
            break;
        }

        case Operation::dive:
        {
            err.err = TreeErrorType::DIVISION_BY_0;
            return NODE_VERIF(node, err);
            break;
        }

        case Operation::power:
        {
            TREE_ASSERT(Create1NumNode(node));
            break;
        }

        case Operation::undefined_operation:
        {
            err.err = UNDEFINED_OPERATION_TYPE;
            return NODE_VERIF(node, err);
            break;
        }

        default:
        {
            assert(0 && "You forgot about some operation in handleCleanLeft0.\n");
            break;
        }
    }

    // GRAPHIC_DUMP(node);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr Create0NumNode(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);

    TreeErr err = {};

    TREE_ASSERT(NodeDtor(node->left));
    TREE_ASSERT(NodeDtor(node->right));
    _SET_NUM(node, 0);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr Create1NumNode(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);

    TreeErr err = {};

    TREE_ASSERT(NodeDtor(node->left));
    TREE_ASSERT(NodeDtor(node->right));
    _SET_NUM(node, 1);

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static Number MakeArithmeticOperation(Number firstOpearnd, Number secondOperand, Operation Operator)
{
    switch (Operator)
    {
        case Operation::plus:
            return firstOpearnd + secondOperand;
            break;

        case Operation::minus:
            return firstOpearnd - secondOperand;
            break;

        case Operation::mul:
            return firstOpearnd * secondOperand;
            break;

        case Operation::dive:
            assert(secondOperand != 0);
            assert(firstOpearnd >= secondOperand);
            assert(firstOpearnd % secondOperand == 0);
            return firstOpearnd / secondOperand;
            break;

        case Operation::power:
            return pow(firstOpearnd, secondOperand);
            break;

        case Operation::undefined_operation:
            assert(0 && "Undefined operation.\n");
            break;

        default:
            assert(0 && "You forgot about some operation.\n");
            return 0;
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

static bool IsArgFunc(const Node_t* node)
{   
    assert(node);

    return node->data.type == NodeArgType::function;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsArgVar(const Node_t* node)
{   
    assert(node);

    return node->data.type == NodeArgType::variable;
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool HasNode2NumChildren(const Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);

    return (IsArgNum(node->left) && IsArgNum(node->right));
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsNum0(const Node_t* node)
{
    assert(node);
    return ((node->data.num == 0) && IsArgNum(node));
}

//----------------------------------------------------------------------------------------------------------------------------------

static bool HasNode0NumChild(const Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    
    return (IsNum0(node->left) || IsNum0(node->right));
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

// static bool HasNode1NumChild(const Node_t* node)
// {

// }

//----------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationCleanSituationGood(const Node_t* node)
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

    TEXT_DUMP(node);
    TEXT_DUMP(node->left);
    TEXT_DUMP(node->right);


    bool flag1 = (oper != Operation::dive);

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

    int res = 1;
    for (size_t i = 0; i < secondNum; i++)
    {
        res *= firstNum;
    }
    return res;
}

//----------------------------------------------------------------------------------------------------------------------------------








