#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "SimplifyTree.h"
#include "../Tree/Tree.h"
#include "../Tree/Tree.h"
#include "../Tree/TreeDump.h"
#include "MathFunctions.h"

static TreeErr SimplifyTreeHelper                                  (Node_t* node);
static TreeErr SimplifyOperation                                   (Node_t* node);

static TreeErr SimplifyNodeTypeSubWith1ChildTypeNum                (Node_t* node);

static TreeErr SimplifyNodeTypeOpearationWith2ChildrenTypeNum      (Node_t* node);
static TreeErr SimplifyNodeTypeOperationWithChildTypeNumVal0       (Node_t* node);

static TreeErr SimplifyLeftChildTypeNumVal0                        (Node_t* node);
static TreeErr SimplifyRightChildTypeNumVal0                       (Node_t* node);

static TreeErr ReamakeNodeToTypeNumVal0                            (Node_t* node);
static TreeErr ReamakeNodeToTypeNumVal1                            (Node_t* node);

static TreeErr SimplifyNodeTypeOperationWithChildTypeNumVal1   (Node_t* node);

static TreeErr SimplifyNodeTypeOperationWithLeftChildTypeNumVal1   (Node_t* node);
static TreeErr SimplifyNodeTypeOperationWithRightChildTypeNumVal1  (Node_t* node);

static TreeErr SetNodeLeftChild                                    (Node_t* node);
static TreeErr SetNodeRightChild                                   (Node_t* node);


static TreeErr SimplifyFunction                                    (Node_t* node);
static TreeErr SimplifyFunctionPattern                             (Node_t* node, Function function, bool* WasChange);

static double (*GetMathFunction                                    (Function function)) (double);

static bool    IsTypeNum                                           (const Node_t* node);
static bool    IsTypeOperation                                     (const Node_t* node);
static bool    IsTypeFunction                                      (const Node_t* node);
static bool    HasNode2ChilrenTypesNum                             (const Node_t* node);
static bool    HasNodeChildTypeNumVal0                             (const Node_t* node);
static bool    IsNodeTypeNumAndVal0                                (const Node_t* node);
static bool    IsSomeToSimplifyInNodeTypeOperation                 (const Node_t* node);
static bool    IsNodeMinusWith1NumChild                            (const Node_t* node);
static bool    HasNode1ChildTypeNumVal1                            (const Node_t* node);

static Number  MakeArithmeticOperation             (Number firstOpearnd, Number secondOperand, Operation Operator);

static const Number eps = 0.0000000001;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr SimplifyTree(Tree_t* tree)
{
    assert(tree);

    TreeErr err = {};

    TREE_ASSERT(SimplifyTreeHelper(tree->root));

    return TREE_VERIF(tree, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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


    NodeArgType type = node->type;

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyFunction(Node_t* node)
{
    assert(node);
    assert(IsTypeFunction(node));
    assert(node->left);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);


    NodeArgType functionArgType = node->left->type;

    RETURN_IF_FALSE(functionArgType == NodeArgType::number, NODE_VERIF(node, err));

    bool WasChange = false;

    for (size_t function_i = 0; function_i < DefaultFunctionsQuant; function_i++)
    {
        Function function = DefaultFunctions[function_i].value;

        TREE_ASSERT(SimplifyFunctionPattern(node, function, &WasChange));

        if (WasChange) return NODE_VERIF(node, err);
    }

    assert(0 && "You forgot about some function");

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyFunctionPattern(Node_t* node, Function function, bool* WasChange)
{
    assert(node);
    assert(IsTypeFunction(node));
    assert(node->left);
    assert(IsTypeNum(node->left));
    assert(WasChange);

    TreeErr err = {};

    Function nodeFunction = node->data.func;

    RETURN_IF_FALSE(nodeFunction == function, NODE_VERIF(node, err));

    double (*mathFunction)(double) = GetMathFunction(function);
    Number   funcArg               = node->left->data.num;
    Number   newData               = mathFunction(funcArg);

    TREE_ASSERT(NodeDtor(node->left));
    _SET_NUM(node, newData);
    
    *WasChange = true;

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static double (*GetMathFunction(Function function)) (double)
{
    switch (function)
    {
        case Function::Sqrt:   return sqrt;
        case Function::Ln:     return log;
        case Function::Sin:    return sin;
        case Function::Cos:    return cos;
        case Function::Tg:     return tan;
        case Function::Ctg:    return ctg;
        case Function::Sh:     return sinh;
        case Function::Ch:     return cosh;
        case Function::Th:     return tanh;
        case Function::Cth:    return ctgh;
        case Function::Arcsin: return asin;
        case Function::Arccos: return acos;
        case Function::Arctg:  return atan;
        case Function::Arcctg: return actg;
        case Function::undefined_function:
        default: assert(0 && "undefined function type"); break;
    }

    assert(0 && "we must be here");
    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyOperation(Node_t* node)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    if (IsNodeMinusWith1NumChild(node))
    {
        TREE_ASSERT(SimplifyNodeTypeSubWith1ChildTypeNum(node));
    }

    else if (HasNode2ChilrenTypesNum(node))
    {
        TREE_ASSERT(SimplifyNodeTypeOpearationWith2ChildrenTypeNum(node));
    }

    else if (HasNodeChildTypeNumVal0(node))
    {
        TREE_ASSERT(SimplifyNodeTypeOperationWithChildTypeNumVal0(node));
    }

    else if (HasNode1ChildTypeNumVal1(node))
    {
        TREE_ASSERT(SimplifyNodeTypeOperationWithChildTypeNumVal1(node));
    }

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeSubWith1ChildTypeNum(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(IsTypeOperation(node));
    assert(node->data.oper == Operation::minus);
    assert(!node->right);
    assert(IsTypeNum(node->left));
    assert(!node->left->left);
    assert(!node->left->right);

    TreeErr err = {};

    Number num     = node->left->data.num;
    Number new_num = -num;

    TREE_ASSERT(NodeDtor(node->left));

    _SET_NUM(node, new_num);

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeOpearationWith2ChildrenTypeNum(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsTypeNum(node->left));
    assert(IsTypeNum(node->right));

    TreeErr err = {};

    if (!IsSomeToSimplifyInNodeTypeOperation(node))
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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeOperationWithChildTypeNumVal0(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsNodeTypeNumAndVal0(node->left) || IsNodeTypeNumAndVal0(node->right));

    TreeErr err = {};

    if (IsNodeTypeNumAndVal0(node->left))
    {
        TREE_ASSERT(SimplifyLeftChildTypeNumVal0(node));
    }

    else if (IsNodeTypeNumAndVal0(node->right))
    {
        TREE_ASSERT(SimplifyRightChildTypeNumVal0(node));
    }

    return NODE_VERIF(node, err);    
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeOperationWithChildTypeNumVal1(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert((IsTypeNum(node->left) && IsDoubleEqual(node->left->data.num, 1, eps)) || (IsTypeNum(node->right) && IsDoubleEqual(node->right->data.num, 1, eps)));

    TreeErr err = {};

    if (IsTypeNum(node->left) && (IsDoubleEqual(node->left->data.num, 1, eps)))
    {
        TREE_ASSERT(SimplifyNodeTypeOperationWithLeftChildTypeNumVal1(node));
    }

    else if (IsTypeNum(node->right) && (IsDoubleEqual(node->right->data.num, 1, eps)))
    {
        TREE_ASSERT(SimplifyNodeTypeOperationWithRightChildTypeNumVal1(node));
    }

    return NODE_VERIF(node, err); 
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeOperationWithLeftChildTypeNumVal1(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsDoubleEqual(node->left->data.num, 1, eps));

    TreeErr err = {};

    Operation oper = node->data.oper;

    switch (oper)
    {
        case Operation::plus:
        case Operation::minus:
        case Operation::dive:                                                      break;
        case Operation::mul:    TREE_ASSERT(SetNodeRightChild(node));              break;
        case Operation::power:  TREE_ASSERT(ReamakeNodeToTypeNumVal1(node));       break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;   break;
        default: assert(0 && "You forgot about some operation.\n");                break;
    }

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyNodeTypeOperationWithRightChildTypeNumVal1(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsDoubleEqual(node->right->data.num, 1, eps));

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyLeftChildTypeNumVal0(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsNodeTypeNumAndVal0(node->left));

    TreeErr err = {};

    Operation operation = node->data.oper;

    switch (operation)
    {
        case Operation::mul:
        case Operation::dive:
        case Operation::power: TREE_ASSERT(ReamakeNodeToTypeNumVal0(node));                         break;
        case Operation::plus:  TREE_ASSERT(SetNodeRightChild(node));                                break;
        case Operation::minus: TREE_ASSERT(SwapNode(&node->left, &node->right)); FREE(node->right); break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;                    break;
        default: assert(0 && "You forgot about some operation.\n");                                 break;
    }

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr SimplifyRightChildTypeNumVal0(Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsNodeTypeNumAndVal0(node->right));

    TreeErr err = {};

    Operation operation = node->data.oper;

    switch (operation)
    {
        case Operation::plus:  
        case Operation::minus: TREE_ASSERT(SetNodeLeftChild(node));                break;
        case Operation::mul:   TREE_ASSERT(ReamakeNodeToTypeNumVal0(node));        break;
        case Operation::power: TREE_ASSERT(ReamakeNodeToTypeNumVal1(node));        break;
        case Operation::dive:  err.err = TreeErrorType::DIVISION_BY_0;             break;
        case Operation::undefined_operation: err.err = UNDEFINED_OPERATION_TYPE;   break;
        default: assert(0 && "You forgot about some operation.\n");                break;
    }

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr ReamakeNodeToTypeNumVal0(Node_t* node)
{
    assert(node);

    TreeErr err = {};

    if (node->left)  TREE_ASSERT(NodeAndUnderTreeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeAndUnderTreeDtor(node->right));

    _SET_NUM(node, 0);

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr ReamakeNodeToTypeNumVal1(Node_t* node)
{
    assert(node);

    TreeErr err = {};

    if (node->left)  TREE_ASSERT(NodeAndUnderTreeDtor(node->left));
    if (node->right) TREE_ASSERT(NodeAndUnderTreeDtor(node->right));

    _SET_NUM(node, 1);

    return NODE_VERIF(node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number  MakeArithmeticOperation(Number firstOpearnd, Number secondOperand, Operation Operator)
{
    switch (Operator)
    {
        case Operation::plus:  return firstOpearnd + secondOperand;
        case Operation::minus: return firstOpearnd - secondOperand;
        case Operation::mul:   return firstOpearnd * secondOperand;
        case Operation::power: return pow(firstOpearnd, secondOperand);
        case Operation::dive:
            assert(!IsDoubleEqual(secondOperand, 0, eps));
            return firstOpearnd / secondOperand;

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTypeNum(const Node_t* node)
{   
    assert(node);

    return node->type == NodeArgType::number;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTypeOperation(const Node_t* node)
{
    assert(node);

    return node->type == NodeArgType::operation;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTypeFunction(const Node_t* node)
{
    assert(node);

    return node->type == NodeArgType::function;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasNode2ChilrenTypesNum(const Node_t* node)
{
    assert(node);

    return (node->left) && (node->right) && (IsTypeNum(node->left) && IsTypeNum(node->right));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeTypeNumAndVal0(const Node_t* node)
{
    assert(node);
    return ((IsDoubleEqual(node->data.num, 0, eps)) && IsTypeNum(node));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasNodeChildTypeNumVal0(const Node_t* node)
{
    assert(node);

    return ((node->left && IsNodeTypeNumAndVal0(node->left)) || (node->right && IsNodeTypeNumAndVal0(node->right)));
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeMinusWith1NumChild(const Node_t* node)
{
    assert(node);
    assert(node->left);
    
    bool flag1 = (node->data.oper == Operation::minus);
    bool flag2 = (IsTypeNum(node->left));
    bool flag3 = (!node->right);

    return (flag1 && flag2 && flag3);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasNode1ChildTypeNumVal1(const Node_t* node)
{
    assert(node);

    bool flag1 = false;
    bool flag2 = false;

    if (node->left && node->left->type == NodeArgType::number)
    {
        Number number = node->left->data.num;
        if (IsDoubleEqual(number, 1, eps)) flag1 = true;
    }

    if (node->right && node->right->type == NodeArgType::number)
    {
        Number number = node->right->data.num;
        if (IsDoubleEqual(number, 1, eps)) flag2 = true;
    }

    return flag1 || flag2;

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSomeToSimplifyInNodeTypeOperation(const Node_t* node)
{
    assert(node);
    assert(node->left);
    assert(node->right);
    assert(IsTypeOperation(node));
    assert(IsTypeNum(node->left));
    assert(IsTypeNum(node->right));

    // Operation oper       = node->data.oper;
    // Number    firstNum   = node->left->data.num;
    // Number    secondNum  = node->right->data.num;

    // bool flag1 = (oper != Operation::dive);

    // if (IsDoubleEqual(secondNum, 0, eps) && !flag1)
    // {   
    //     assert(0 && "Division by zero.");
    // }

    // RETURN_IF_TRUE(IsDoubleEqual(secondNum, 0, eps), flag1);



    // bool flag2 = (firstNum % IsDoubleEqual(secondNum, 1, eps));

    // return (flag1 || flag2);
    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

