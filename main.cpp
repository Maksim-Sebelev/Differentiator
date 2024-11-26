#include <stdio.h>
#include "Tree/Tree.h"
#include "Differentiator/Differentiator.h"
#include "Differentiator/DiffDump.h"


int main()
{
    const char* const inputFile = "test.txt";
    Tree_t tree = {};

    TREE_ASSERT(TreeCtor(&tree, inputFile));

    // GRAPHIC_DUMP(&tree);

    Node_t* root = tree.root;
    GRAPHIC_DUMP(tree.root);


    DIFF_ASSERT(Diff(&tree.root));

    PrintPrefTree(&tree);

    printf("\n");

    PrintInfixTree(&tree);

    GRAPHIC_DUMP(tree.root);

    TREE_ASSERT(TreeDtor(&tree));

    return 0;
}
