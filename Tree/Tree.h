#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "../Common/ColorPrint.h"


struct ErrPlace
{
    const char* File;
    int         Line;
    const char* Func;
};


struct TreeErrorType
{
    unsigned int IsFatalError              : 1;
    unsigned int CtorCallocNull            : 1;
    unsigned int InsertIncorrectSituation  : 1;
    unsigned int DtorNodeThatChildrenHas   : 1;
    unsigned int InputFileSyntaxisError    : 1;

    ErrPlace      Place;
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
    ln,
    undefined_operation
};

struct TreeElem_t
{
    NodeArgType type;
    char*        arg;
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


TreeErrorType TreeCtor         (Tree_t* tree, const char* file);
TreeErrorType TreeDtor         (Tree_t* root);
TreeErrorType NodeCtor         (Node_t** node, const NodeArgType type, char* arg,  Node_t* left, Node_t* right);
TreeErrorType NodeDtor         (Node_t*  node);
TreeErrorType NodeCopy         (Node_t** copy, const Node_t* node);

Operation GetOperationType     (const char* operation);

void          PrintPrefTree    (const Tree_t* tree);
void          PrintPostTree    (const Tree_t* tree);
void          PrintInfixTree   (const Tree_t* tree);
void          PrintSortTree    (const Tree_t* tree);


#define TREE_VERIF(TreePtr, Err) TreeVerif(TreePtr, &Err, __FILE__, __LINE__, __func__)


#define TREE_RETURN_IF_ERR(TreePtr, Err) do                          \
{                                                                     \
    TreeErrorType ErrCopy = Err;                                       \
    TreeVerif(TreePtr, &ErrCopy, __FILE__, __LINE__, __func__);         \
    if (ErrCopy.IsFatalError == 1)                                       \
    {                                                                     \
        return ErrCopy;                                                    \
    }                                                                       \
} while (0)                                                                  \



#define TREE_ASSERT(Err) do                                 \
{                                                            \
    TreeErrorType ErrCopy = Err;                              \
    TreeAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
    if (ErrCopy.IsFatalError == 1)                              \
    {                                                            \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...\n");             \
        abort();                                                   \
    }                                                               \
} while (0)                                                          \



void TreeAssertPrint(TreeErrorType* Err, const char* File, int Line, const char* Func);

#endif
