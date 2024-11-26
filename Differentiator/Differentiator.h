#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <stdlib.h>
#include "../Common/GlobalInclude.h"
#include "../Tree/Tree.h" 

enum DiffErrorType
{
    NO_ERR,
    NUM_HAS_CHILD,
    VAR_HAS_CHILD,
    OPR_HAS_NOT_CHILDS,
    UNDEFINED_NODE_TYPE,
    NOT_CORRECT_TYPE_IN_THIS_OPERATION_TYPE_DIFF_FUNC,
    NODE_COPY_CALLOC_NULL,
};


struct DiffErr
{
    DiffErrorType err;
    CodePlace     place;
};


DiffErr DiffVerif  (const Node_t* node, DiffErr* err, const char* const file, const int line, const char* const func);
DiffErr Diff       (Node_t** node);


void DiffAssertPrint(const DiffErr* const err, const char* const file, const int line, const char* const func);

#define DIFF_ASSERT(Err) do                                    \
{                                                               \
    DiffErr errCopy = Err;                                       \
    if (errCopy.err != DiffErrorType::NO_ERR)                     \
    {                                                              \
        DiffAssertPrint(&errCopy, __FILE__, __LINE__, __func__);    \
        abort();                                                     \
    }                                                                 \
} while (0)                                                            \


#define DIFF_RETURN_IF_ERR(node, err) do                \
{                                                        \
    DiffErr errCopy = err;                                \
    if (errCopy.err != DiffErrorType::NO_ERR)              \
    {                                                       \
        return DIFF_VERIF(node, errCopy);                    \
    }                                                         \
} while (0)                                                    \


#define DIFF_VERIF(node, err) DiffVerif(node, &err, __FILE__, __LINE__, __func__)

#endif
