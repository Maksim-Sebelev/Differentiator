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

    const char* input = "( ( - arcsin(x ^ 3) ) ^ arccos(1 / (2 * sqrt( 1 / tg(x) - 1) * sin( x ^ 2)) + 3 * sh(ch(cth( 1 / th(x)))) + 2 / sqrt(x)^sqrt(1/x) / th(1 / x ^ 2)))$";

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