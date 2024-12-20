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

    const char* input = "arccos(x)^arcsin(x)$";

    TREE_ASSERT(TreeCtor(&tree, input));
    TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(Diff(&tree));
    TREE_GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(SimplifyTree(&tree));
    TREE_GRAPHIC_DUMP(tree.root);

    Tree_t taylor = {};
    TREE_ASSERT(Taylor(&tree, &taylor, 3));
    TREE_GRAPHIC_DUMP(taylor.root);

    TREE_ASSERT(SimplifyTree(&taylor));
    TREE_GRAPHIC_DUMP(taylor.root);

    TREE_ASSERT(TreeDtor(&taylor));
    TREE_ASSERT(TreeDtor(&tree));

    return EXIT_SUCCESS;
}