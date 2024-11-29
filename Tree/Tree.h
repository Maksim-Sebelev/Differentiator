#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"


enum TreeErrorType
{
    NO_ERR,
    CTOR_CALLOC_RETURN_NULL,
    INSERT_INCORRECT_SITUATION,
    DTOR_NODE_WITH_CHILDREN,
    NUM_TYPE_NODES_ARG_IS_UNDEFINED,
    VAR_TYPE_NODES_ARG_IS_UNDEFINED,
    OPER_TYPE_NODES_ARG_IS_UNDEFINED,
    FUNC_TYPE_NODES_ARG_IS_UNDEFINED,
    NODE_IS_NUM_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED,
    NODE_IS_OPERATION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED,
    NODE_IS_FUNCTION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED,
    NODE_IS_VARIABLE_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED,
    OPER_HAS_INCORRECT_CHILD_QUANT,
    NUM_HAS_INCORRECT_CHILD_QUANT,
    VAR_HAS_INCORRECT_CHILD_QUANT,
    FUNC_HAS_INCORRECT_CHILD_QUANT,
    UNDEFINED_NODE_TYPE,
    UNDEFINED_OPERATION_TYPE,
    UNDEFINED_FUNCTION_TYPE,
    INCORRECT_TREE_SIZE,
    DIVISION_BY_0,
};


struct TreeErr
{
    TreeErrorType err;
    CodePlace place;
};


enum NodeArgType
{
    undefined,
    operation,
    variable,
    number,
    function,
};


enum Operation
{
    undefined_operation,
    plus, 
    minus,
    mul, 
    dive,
    power,
};


enum Function
{
    undefined_function,
    ln,
    sin,
    cos,
    tg,
    ctg,
    arcsin,
    arccos,
    arctg,
    arcctg,
    sh,
    ch,
    th,
    cth,
};


enum Variable
{
    undefined_variable,
    x,
    y,
};


typedef int Number;

struct TreeElem_t
{
    NodeArgType type;
    Operation   oper;
    Number      num;
    Function    func;
    Variable    var;
};



struct Node_t
{
    TreeElem_t  data;
    Node_t*     right;
    Node_t*     left;
};



struct Tree_t
{
    Node_t* root;
    size_t  size;
};


TreeErr TreeCtor         (Tree_t*  tree, const char** buffer, size_t bufSize);
TreeErr TreeDtor         (Tree_t*  root, const char** buffer);
TreeErr NodeCtor         (Node_t** node, NodeArgType type, Number num, Operation oper, Function func, Variable variable, Node_t* left, Node_t* right);
TreeErr NodeDtor         (Node_t*  node);
TreeErr NodeCopy         (Node_t** copy, const Node_t* node);
TreeErr SetNode          (Node_t*  node, NodeArgType type, Number num, Operation oper, Function func, Variable var, Node_t* left, Node_t* right);
TreeErr NodeSetCopy      (Node_t* copy, const Node_t* node);

TreeErr TreeVerif         (const Tree_t* tree, TreeErr* Err, const char* file, const int line, const char* func);
TreeErr NodeVerif         (const Node_t* node, TreeErr* err, const char* file, const int line, const char* func);

Number    GetNumber            (const char* number);
Operation GetOperationType     (const char* operation);
Function  GetFunctionType      (const char* function);
Variable  GetVariableType      (const char* variable);

#define _NUM(  node, val               )     TREE_ASSERT(NodeCtor(node, NodeArgType::number,    val, Operation::undefined_operation, Function::undefined_function, Variable::undefined_variable, nullptr,         nullptr))
#define _OPER( node, oper, left, right )     TREE_ASSERT(NodeCtor(node, NodeArgType::operation, 0,   oper,                           Function::undefined_function, Variable::undefined_variable, left,            right))
#define _FUNC( node, func, left        )     TREE_ASSERT(NodeCtor(node, NodeArgType::function,  0,   Operation::undefined_operation, func,                         Variable::undefined_variable, left,            nullptr))
#define _VAR(  node, var               )     TREE_ASSERT(NodeCtor(node, NodeArgType::variable,  0,   Operation::undefined_operation, Function::undefined_function, var,                          nullptr,         nullptr))

#define _SET_NUM(  node, val               ) TREE_ASSERT(SetNode (node, NodeArgType::number,    val, Operation::undefined_operation, Function::undefined_function, Variable::undefined_variable, nullptr,         nullptr))
#define _SET_OPER( node, oper, left, right ) TREE_ASSERT(SetNode (node, NodeArgType::operation, 0,   oper,                           Function::undefined_function, Variable::undefined_variable, left,            right))
#define _SET_FUNC( node, func, left        ) TREE_ASSERT(SetNode (node, NodeArgType::function,  0,   Operation::undefined_operation, func,                         Variable::undefined_variable, left,            nullptr))
#define _SET_VAR(  node, var               ) TREE_ASSERT(SetNode (node, NodeArgType::variable,  0,   Operation::undefined_operation, Function::undefined_function, var,                          nullptr,         nullptr))

#define _SET_OPER_ONLY( node, oper )         TREE_ASSERT(SetNode (node, NodeArgType::operation, 0,   oper,                           Function::undefined_function, Variable::undefined_variable, (node)->left,    (node)->right))
#define _SET_FUNC_ONLY( node, func )         TREE_ASSERT(SetNode (node, NodeArgType::function,  0,   Operation::undefined_operation, func,                         Variable::undefined_variable, (node)->left,    nullptr))
#define _SET_VAR_ONLY(  node, var  )         TREE_ASSERT(SetNode (node, NodeArgType::variable,  0,   Operation::undefined_operation, Function::undefined_function, var,                          nullptr,         nullptr))


#define TREE_VERIF(TreePtr, Err) TreeVerif(TreePtr, &Err, __FILE__, __LINE__, __func__)

#define NODE_VERIF(Node, Err)    NodeVerif(Node,    &Err, __FILE__, __LINE__, __func__)


#define TREE_RETURN_IF_ERR(TreePtr, Err) do                          \
{                                                                     \
    TreeErr ErrCopy = Err;                                             \
    TreeVerif(TreePtr, &ErrCopy, __FILE__, __LINE__, __func__);         \
    if (ErrCopy.err != NO_ERR)                                           \
    {                                                                     \
        return ErrCopy;                                                    \
    }                                                                       \
} while (0)                                                                  \


#define NODE_RETURN_IF_ERR(Node, Err) do                             \
{                                                                     \
    TreeErr ErrCopy = Err;                                             \
    NodeVerif(Node, &ErrCopy, __FILE__, __LINE__, __func__);            \
    if (ErrCopy.err != NO_ERR)                                           \
    {                                                                     \
        return ErrCopy;                                                    \
    }                                                                       \
} while (0)                                                                  \



#define TREE_ASSERT(Err) do                                          \
{                                                                     \
    TreeErr ErrCopy = Err;                                             \
    if (ErrCopy.err != TreeErrorType::NO_ERR)                           \
    {                                                                    \
        TreeAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);          \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...\n");                      \
        abort();                                                            \
    }                                                                        \
} while (0)                                                                   \


void TreeAssertPrint(TreeErr* Err, const char* File, int Line, const char* Func);


#endif
