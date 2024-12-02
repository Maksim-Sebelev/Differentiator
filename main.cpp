#include <stdio.h>
#include "Tree/Tree.h"
#include "Differentiator/Differentiator.h"
#include "Differentiator/DiffDump.h"
#include "Differentiator/SimplifyTree.h"
#include "Differentiator/Taylor.h"
#include "Onegin/onegin.h"


int main()
{
    const char* const inputFile = "test.txt";
    Tree_t tree = {};

    size_t bufSize = 0;
    const char** buffer = ReadBufferFromFile(inputFile, &bufSize);

    TREE_ASSERT(TreeCtor(&tree, buffer, bufSize));
    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(Diff(&tree));
    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(SimplifyTree(&tree));
    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(CalcTree(&tree, 13));
    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree, buffer));

    return 0;
}