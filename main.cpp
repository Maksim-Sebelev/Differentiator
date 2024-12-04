#include <stdio.h>
#include "Tree/Tree.h"
#include "Tree/TreeDump.h"
#include "Differentiator/Differentiator.h"
#include "Differentiator/SimplifyTree.h"
#include "Differentiator/Taylor.h"
#include "Onegin/onegin.h"


int main()
{
    const char* const inputFile = "test.txt";
    Tree_t tree = {};

    // const char* input = "1/sin(x)$";
    const char* input = "ln(x)$";
    tree.root = GetTree(input);

    GRAPHIC_DUMP(tree.root);

    size_t bufSize = 0;
    const char** buffer = ReadBufferFromFile(inputFile, &bufSize);

    // TREE_ASSERT(TreeCtor(&tree, buffer, bufSize));
    // TEXT_DUMP(tree.root);
    // GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(Diff(&tree));
    // GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(SimplifyTree(&tree));

    // GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(CalcTree(&tree, 3));

    // GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree, buffer));

    return 0;
}