#ifndef TREE_DUMP_H
#define TREE_DUMP_H

#include "Tree.h"
#include "Token.h"

void TokenDump        (const Token_t* tokenArr, size_t arrSize, const char* file, const int line, const char* func);
void GraphicDump      (const Node_t* node,                       const char* file, const int line, const char* func);
void PrintAllNodeInfo (const Node_t* node,                       const char* file, const int line, const char* func);

#define GRAPHIC_DUMP(node) GraphicDump(node, __FILE__, __LINE__, __func__)

#define TOKEN_DUMP(tokenArr, arrSize) TokenDump(tokenArr, arrSize, __FILE__, __LINE__, __func__)

#define TEXT_DUMP(node) PrintAllNodeInfo(node, __FILE__, __LINE__, __func__)

#endif
