#ifndef TREE_GRAPHIC_DUMP_H
#define TREE_GRAPHIC_DUMP_H

#include "Tree.h"
#include "ReadTree.h"

void TokenGraphicDump (const Token_t* tokenArr, size_t arrSize, const char* file, const int line, const char* func);
void TokenTextDump    (const Token_t* token, size_t tokenNum,   const char* file, const int line, const char* func);

void TreeDump         (const Node_t* node,                      const char* file, const int line, const char* func);
void NodeTextDump     (const Node_t* node,                      const char* file, const int line, const char* func);


#define TREE_GRAPHIC_DUMP(node) TreeDump     (node, __FILE__, __LINE__, __func__)
#define TEXT_NODE_DUMP(   node) NodeTextDump (node, __FILE__, __LINE__, __func__)

#define TOKEN_GRAPHIC_DUMP(tokenArr, arrSize)  TokenGraphicDump(tokenArr, arrSize,  __FILE__, __LINE__, __func__)
#define TOKEN_TEXT_DUMP(   token,    tokenNum) TokenTextDump   (token,    tokenNum, __FILE__, __LINE__, __func__)



#endif
