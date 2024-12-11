#include <stdio.h>
#include "Tree/Tree.h"
#include "Tree/TreeDump.h"
#include "Differentiator/Differentiator.h"
#include "Differentiator/SimplifyTree.h"
#include "Differentiator/Taylor.h"

#include "Tree/ReadTree.h"

int main()
{
    Tree_t tree = {};

    printf("XUI = awenckjew\n");

    const char* input = "(2*x)^2$";

    TREE_ASSERT(TreeCtor(&tree, input));
    TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(Diff(&tree));
    TREE_GRAPHIC_DUMP(tree.root) ;

    TREE_ASSERT(SimplifyTree(&tree));
    TREE_GRAPHIC_DUMP(tree.root);

    // TREE_ASSERT(CalcTree(&tree, 3));
    // TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree));

    return EXIT_SUCCESS;
}