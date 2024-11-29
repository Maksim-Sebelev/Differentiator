#ifndef DIFF_DUMP_H
#define DIFF_DUMP_H

#include "../Tree/Tree.h"


void GraphicDump      (const Node_t* node, const char* file, const int line, const char* func);
void PrintAllNodeInfo (const Node_t* node, const char* file, const int line, const char* func);

#define GRAPHIC_DUMP(node) GraphicDump(node, __FILE__, __LINE__, __func__)

#define TEXT_DUMP(node) PrintAllNodeInfo(node, __FILE__, __LINE__, __func__)

#endif
