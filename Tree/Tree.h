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
    NUM_TYPE_NODES_ARG_IS_NOT_NUM,
    VAR_TYPE_NODES_ARG_IS_NOT_VAR,
    OPER_TYPE_NODES_ARG_IS_NOT_OPER,
    FUNC_TYPE_NODES_ARG_IS_NOT_FUNC,
    OPER_HAS_INCORRECT_CHILD_QUANT,
    NUM_HAS_INCORRECT_CHILD_QUANT,
    VAR_HAS_INCORRECT_CHILD_QUANT,
    FUNC_HAS_INCORRECT_CHILD_QUANT,
    UNDEFINED_NODE_TYPE,
    UNDEFINED_OPERATION_TYPE,
    UNDEFINED_FUNCTION_TYPE,
    INCORRECT_TREE_SIZE,
};

struct TreeErr
{
    TreeErrorType err;
    CodePlace place;
};

enum NodeArgType
{
    operation,
    variable,
    number,
    function,
    undefined,
};

enum Operation
{
    plus, 
    minus,
    mul, 
    dive,
    pow,
    undefined_operation
};

enum Function
{
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
    undefined_function
};

struct TreeElem_t
{
    NodeArgType type;
    const char* arg;
};


struct Node_t
{
    TreeElem_t data;
    Node_t*    right;
    Node_t*    left;
};



struct Tree_t
{
    Node_t* root;
    size_t  size;
};


TreeErr TreeCtor         (Tree_t*  tree, const char** buffer, size_t bufSize);
TreeErr TreeDtor         (Tree_t*  root, const char** buffer);
TreeErr NodeCtor         (Node_t** node, NodeArgType type, const char* arg, Node_t* left, Node_t* right);
TreeErr NodeDtor         (Node_t** node);
TreeErr NodeCopy         (Node_t** copy, const Node_t* node);
TreeErr SetNode          (Node_t** node, NodeArgType type, const char* arg, Node_t* left, Node_t* right);

TreeErr TreeVerif         (const Tree_t* tree, TreeErr* Err, const char* file, const int line, const char* func);
TreeErr NodeVerif         (const Node_t* node, TreeErr* err, const char* file, const int line, const char* func);

Operation GetOperationType     (const char* operation);
Function  GetFunctionType      (const char* function);

void PrintInfixTree   (const Tree_t* tree);

#define TREE_VERIF(TreePtr, Err) TreeVerif(TreePtr, &Err, __FILE__, __LINE__, __func__)

#define NODE_VERIF(Node, Err)    NodeVerif(Node, &Err, __FILE__, __LINE__, __func__)


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



#define TREE_ASSERT(Err) do                                 \
{                                                            \
    TreeErr ErrCopy = Err;                                    \
    TreeAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
    if (ErrCopy.err != TreeErrorType::NO_ERR)                   \
    {                                                            \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...\n");             \
        abort();                                                   \
    }                                                               \
} while (0)                                                          \


void TreeAssertPrint(TreeErr* Err, const char* File, int Line, const char* Func);


#endif
