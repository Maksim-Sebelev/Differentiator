#include <stdio.h>
#include "Tree/Tree.h"
#include "Tree/TreeDump.h"
#include "Differentiator/Differentiator.h"
#include "Differentiator/SimplifyTree.h"
#include "Differentiator/Taylor.h"

#include "Tree/RecursiveDescent.h"

int main()
{
    Tree_t tree = {};

    const char* input = "x*(x+21$";

    TREE_ASSERT(TreeCtor(&tree, input));
    TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(Diff(&tree));
    TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(SimplifyTree(&tree));
    TREE_GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(CalcTree(&tree, 3));
    // TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree));

    return EXIT_SUCCESS;
}