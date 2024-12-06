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

    const char* input = "x+2+x+3*sin(x+4)*5$";

    TREE_ASSERT(TreeCtor(&tree, input));

    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(Diff(&tree));
    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(SimplifyTree(&tree));
    GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(CalcTree(&tree, 3));
    // GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree));

    return EXIT_SUCCESS;
}