#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "CleanTree.h"
#include "../Tree/Tree.h"
#include "../Onegin/onegin.h"
#include "DiffDump.h"

static TreeErr CleanTreeHelper                         (Node_t* node);
static TreeErr CleanOperation                          (Node_t* node);

static TreeErr HandleCleanMinusWith1NumChild           (Node_t* node);

static TreeErr HandleCleanOpearationWith2Num           (Node_t* node);
static TreeErr HandleCleanOperationWith0Child          (Node_t* node);

static TreeErr HandleCleanLeft0                        (Node_t* node);
static TreeErr HandleCleanRight0Num                    (Node_t* node);

static TreeErr Create0NumNode                          (Node_t* node);
static TreeErr Create1NumNode                          (Node_t* node);

static TreeErr HandleCleanOperationWith1NumChild      (Node_t* node);

static TreeErr HandleCleanOperationWith1NumLeftChild   (Node_t* node);
static TreeErr HandleCleanOperationWith1NumRightChild  (Node_t* node);

static TreeErr SetNodeLeftChild                        (Node_t* node);
static TreeErr SetNodeRightChild                       (Node_t* node);

static TreeErr CleanFunction                           (Node_t* node);

static TreeErr HandleCleanLn                           (Node_t* node);

static bool    IsArgNum                                (const Node_t* node);
static bool    IsArgOper                               (const Node_t* node);
static bool    HasNode2NumChildren                     (const Node_t* node);
static bool    HasNode0NumChild                        (const Node_t* node);
static bool    IsNum0                                  (const Node_t* node);
static bool    IsArgLn                                 (const Node_t* node);
static bool    IsOperationCleanSituationGood           (const Node_t* node);
static bool    IsNodeMinusWith1NumChild                (const Node_t* node);
static bool    HasNode1NumChild                        (const Node_t* node);

static Number  MakeArithmeticOperation                 (Number firstOpearnd, Number secondOperand, Operation Operator);
static Number  pow                                     (Number firstOperand, Number secondOperand);

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

    if (type == NodeArgType::operation)
    {
        TREE_ASSERT(CleanOperation(node));
    }

    else if (type == NodeArgType::function)
    {
        TREE_ASSERT(CleanFunction(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanOperation(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsNodeMinusWith1NumChild(node))
    {
        TREE_ASSERT(HandleCleanMinusWith1NumChild(node));
    }

    else if (HasNode2NumChildren(node))
    {
        TREE_ASSERT(HandleCleanOpearationWith2Num(node));
    }

    else if (HasNode0NumChild(node))
    {
        TREE_ASSERT(HandleCleanOperationWith0Child(node));
    }

    else if (HasNode1NumChild(node))
    {
        TREE_ASSERT(HandleCleanOperationWith1NumChild(node));
    }

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr CleanFunction(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsArgLn(node))
    {
        TREE_ASSERT(HandleCleanLn(node));
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

static TreeErr HandleCleanOperationWith0Child(Node_t* node)
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

static TreeErr HandleCleanOperationWith1NumChild(Node_t* node)
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
        TREE_ASSERT(HandleCleanOperationWith1NumLeftChild(node));
    }

    else if (secondNum == 1)
    {
        TREE_ASSERT(HandleCleanOperationWith1NumRightChild(node));
    }

    return NODE_VERIF(node, err); 
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanOperationWith1NumLeftChild(Node_t* node)
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
            return NODE_VERIF(node, err);
            break;

        case Operation::minus:
            return NODE_VERIF(node, err);
            break;

        case Operation::dive:
            return NODE_VERIF(node, err);
            break;

        case Operation::mul:
            TREE_ASSERT(SetNodeRightChild(node));
            return NODE_VERIF(node, err);
            break;
        
        case Operation::power:
            TREE_ASSERT(Create1NumNode(node));
            return NODE_VERIF(node, err);
            break;
        
        case Operation::undefined_operation:
            err.err = UNDEFINED_OPERATION_TYPE;
            return NODE_VERIF(node, err);
            break;

        default:
            assert(0 && "You forgot about some operation in CleanTree.cpp, func - Clean1NumChild.\n");
            return NODE_VERIF(node, err);
            break;
    }

    assert(0 && "We must be here.\n");
    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanOperationWith1NumRightChild(Node_t* node)
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
            return NODE_VERIF(node, err);
            break;

        case Operation::minus:
            return NODE_VERIF(node, err);
            break;

        case Operation::mul:
            TREE_ASSERT(SetNodeLeftChild(node));
            return NODE_VERIF(node, err);
            break;
        
        case Operation::dive:
            TREE_ASSERT(SetNodeLeftChild(node));
            return NODE_VERIF(node, err);
            break;
        
        case Operation::power:
            TREE_ASSERT(SetNodeLeftChild(node));
            return NODE_VERIF(node, err);
            break;
        
        case Operation::undefined_operation:
            err.err = UNDEFINED_OPERATION_TYPE;
            return NODE_VERIF(node, err);
            break;

        default:
            assert(0 && "You forgot about some operation in CleanTree.cpp, func - Clean1NumChild.\n");
            return NODE_VERIF(node, err);
            break;
    }

    assert(0 && "We must be here.\n");
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

    return NODE_VERIF(node, err);
}

//----------------------------------------------------------------------------------------------------------------------------------

static TreeErr HandleCleanLn(Node_t* node)
{
    assert(node);
    assert(IsArgLn(node));
    assert(node->left);
    assert(!node->right);
    assert(IsArgNum(node->left));

    TreeErr err = {};

    Number num = node->left->data.num;

    if (num == 1)
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

    if (node->left)  TREE_ASSERT(NodeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeDtor(node->right));

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

    if (node->left)  TREE_ASSERT(NodeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeDtor(node->right));

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
            assert((firstOpearnd % secondOperand) == 0);
            return firstOpearnd / secondOperand;
            break;

        case Operation::power:
            return pow(firstOpearnd, secondOperand);
            break;

        case Operation::undefined_operation:
            assert(0 && "Undefined operation.\n");
            return 0;
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

static bool IsArgLn(const Node_t* node)
{
    assert(node);

    return (node->data.func == Function::ln);
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

static bool HasNode1NumChild(const Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);

    Number leftNum  = node->left ->data.num;
    Number rightNum = node->right->data.num;

    bool flag1 = (IsArgNum(node->left) || IsArgNum(node->right));
    bool flag2 = (leftNum == 1 || rightNum == 1);

    return (flag1 && flag2);
}

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

    Number res = 1;
    for (int i = 0; i < secondNum; i++)
    {
        res *= firstNum;
    }
    return res;
}

//----------------------------------------------------------------------------------------------------------------------------------








