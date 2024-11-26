#include "Tree.h"

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "../Onegin/onegin.h"
#include "../Common/ColorPrint.h"


static Node_t*      GetNode           (char** buffer, size_t bufSize, size_t* buffer_i);

static bool         IsOperation       (const char* const str);
static bool         IsFunction        (const char* const str);
static bool         IsVariable        (const char* const str);
static bool         IsNumber          (const char* const str);


static TreeElem_t   GetNodeType            (char** buffer, size_t* buffer_i);
static bool         IsLeftBracket       (char** buffer, size_t* buffer_i);
static bool         IsRightBracket      (char** buffer, size_t* buffer_i);
static bool         IsNull              (char** buffer, size_t* buffer_i);

static const char*  GetTypeInStrFormat  (NodeArgType type);


static void        PrintPrefTreeHelper   (const Node_t* node);
static void        PrintInfixTreeHelper  (const Node_t* node);

static void PrintError                 (const TreeErrorType* Err);
static void CodePlaceCtor              (      TreeErrorType* Err, const char* File, int Line, const char* Func);
static void PrintPlace                 (                          const char* File, int Line, const char* Function);

static TreeErrorType TreeVerif         (const Tree_t* tree, TreeErrorType* Err, const char* File, int Line, const char* Func);
static TreeErrorType TreeDtorHelper    (      Node_t* node, TreeErrorType* Err);

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType TreeCtor(Tree_t* tree, const char* file)
{
    assert(tree);
    assert(file);

    TreeErrorType Err = {};

    size_t bufSize = 0;
    char** buffer = ReadBufferFromFile(file, &bufSize);

    for (size_t i = 0; i < bufSize; i++)
    {
        printf("buf[%2lu] '%s'\n", i, buffer[i]);
    }
    printf("\n\n");


    size_t buffer_i = 0;
    tree->root = GetNode(buffer, bufSize, &buffer_i);

    BufferDtor(buffer);
    return TREE_VERIF(tree, Err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType TreeDtor(Tree_t* tree)
{
    assert(tree);
    assert(tree->root);

    TreeErrorType Err = {};

    TREE_RETURN_IF_ERR(tree, TreeDtorHelper(tree->root, &Err));

    tree->size = 0;
    tree->root = NULL;
    return TREE_VERIF(tree, Err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErrorType TreeDtorHelper(Node_t* node, TreeErrorType* Err)
{
    if (node == NULL)
    {
        return *Err;
    }

    TreeDtorHelper(node->left,  Err);
    TreeDtorHelper(node->right, Err);

    NodeDtor(node);

    return *Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType NodeCtor(Node_t** node, const NodeArgType type, char* arg,  Node_t* left, Node_t* right)
{
    TreeErrorType Err = {};

    *node = (Node_t*) calloc(1, sizeof(Node_t));

    if (*node == NULL)
    {
        Err.IsFatalError   = 1;
        Err.CtorCallocNull = 1;
        assert(0);
        return Err;
    }

    (*node)->data.type = type;
    (*node)->data.arg  = arg;
    (*node)->left      = left;
    (*node)->right     = right;

    return Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType NodeDtor(Node_t* node)
{
    assert(node);

    TreeErrorType Err = {};

    node->left  = NULL;
    node->right = NULL;
    FREE(node->data.arg);

    FREE(node);

    return Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType NodeCopy(Node_t** copy, const Node_t* node)
{
    assert(node);

    TreeErrorType err = {};

    TREE_ASSERT(NodeCtor(copy, node->data.type, node->data.arg, nullptr, nullptr));

    if (*copy == nullptr)
    {
        assert(0);
        return err;
    }

    if (node->left)
    {
        TREE_ASSERT(NodeCopy(&(*copy)->left,  node->left));
    }

    if (node->right)
    {
        TREE_ASSERT(NodeCopy(&(*copy)->right, node->right));
    }

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetNode(char** buffer, size_t bufSize, size_t* buffer_i)
{
    printf("buffer_i = %lu\n", *buffer_i);

    if (IsNull(buffer, buffer_i))
    {
        (*buffer_i)++;
        return nullptr;
    }

    Node_t* node = {};
    NodeCtor(&node, NodeArgType::undefined, nullptr, nullptr, nullptr);

    if (IsLeftBracket(buffer, buffer_i))
    {
        (*buffer_i)++;
        node->data = GetNodeType(buffer, buffer_i);
    }

    node->left  = GetNode(buffer, bufSize, buffer_i);
    node->right = GetNode(buffer, bufSize, buffer_i);

    if (IsRightBracket(buffer, buffer_i))
    {
        (*buffer_i)++;
        return node;
    }

    assert(false && "SYNTAXIS ERROR\n");
    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeElem_t GetNodeType(char** buffer, size_t* buffer_i)
{
    char* arg = strdup(buffer[*buffer_i]);
    NodeArgType type = NodeArgType::undefined;
    (*buffer_i)++;

    if (IsVariable(arg))
    {
        type = NodeArgType::variable;
    }

    else if (IsNumber(arg))
    {
        type = NodeArgType::number;
    }

    else if (IsOperation(arg))
    {
        type = NodeArgType::operation;
    }

    else if (IsFunction(arg))
    {
        type = NodeArgType::function;
    }

    else
    {
        printf("arg = '%s'\n", arg);
        assert(0 && "undefined NodeArgType.\n");
    }

    return {type, arg};
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

Operation GetOperationType(const char* operation)
{
    if (strcmp(operation, "+") == 0)
    {
        return Operation::plus;
    }

    if (strcmp(operation, "-") == 0)
    {
        return Operation::minus;
    }

    if (strcmp(operation, "*") == 0)
    {
        return Operation::mul;
    }

    if (strcmp(operation, "/") == 0)
    {
        return Operation::dive;
    }

    if (strcmp(operation, "^") == 0)
    {
        return Operation::pow;
    }

    return Operation::undefined_operation;
}

//--------------------------------------------------------------------------------------------------------------------------------------

static bool IsLeftBracket(char** buffer, size_t* buffer_i)
{
    const char* bufElem = buffer[*buffer_i];
    return strcmp(bufElem, "(") == 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRightBracket(char** buffer, size_t* buffer_i)
{
    const char* bufElem = buffer[*buffer_i];
    return strcmp(bufElem, ")") == 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNull(char** buffer, size_t* buffer_i)
{
    const char* bufElem = buffer[*buffer_i];
    return strcmp(bufElem, "nil") == 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsVariable(const char* const str)
{
    assert(str);

    if (strlen(str) != 1)
    {
        return false;
    }

    char c = str[0];
    return (c == 'x');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNumber(const char* const str)
{
    assert(str);

    char* strEndPtr = nullptr;

    strtol(str, &strEndPtr, 10);

    return ((int) strlen(str) == strEndPtr - str);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperation(const char* const str)
{
    assert(str);

    if (strcmp(str, "+") == 0)
        return true;

    if (strcmp(str, "-") == 0)
        return true;
    
    if (strcmp(str, "*") == 0)
        return true;

    if (strcmp(str, "/") == 0)
        return true;

    if (strcmp(str, "^") == 0)
        return true;

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsFunction(const char* const str)
{
    if (strcmp(str, "ln") == 0)
        return true;

    if (strcmp(str, "sin") == 0)
        return true;

    if (strcmp(str, "cos") == 0)
        return true;

    if (strcmp(str, "tg") == 0)
        return true;

    if (strcmp(str, "ctg") == 0)
        return true;

    if (strcmp(str, "sh") == 0)
        return true;

    if (strcmp(str, "ch") == 0)
        return true;

    if (strcmp(str, "th") == 0)
        return true;

    if (strcmp(str, "cth") == 0)
        return true;

    if (strcmp(str, "arcsin") == 0)
        return true;

    if (strcmp(str, "arccos") == 0)
        return true;

    if (strcmp(str, "arctg") == 0)
        return true;

    if (strcmp(str, "arcctg") == 0)
        return true;

    if (strcmp(str, "arcsh") == 0)
        return true;

    if (strcmp(str, "arcch") == 0)
        return true;

    if (strcmp(str, "arcth") == 0)
        return true;

    if (strcmp(str, "arccth") == 0)
        return true;

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErrorType TreeVerif(const Tree_t* /* tree */, TreeErrorType* Err, const char* File, int Line, const char* Func)
{
    CodePlaceCtor(Err, File, Line, Func);


    return *Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const TreeErrorType* Err)
{
    if (Err->IsFatalError == 0)
    {
        COLOR_PRINT(RED, "print err no err.\n");        
        return;
    }

    if (Err->CtorCallocNull == 1)
    {
        COLOR_PRINT(RED, "Error: failed alocate memory in ctor.\n");
    }

    if (Err->InsertIncorrectSituation == 1)
    {
        COLOR_PRINT(RED, "Error: undefined situation in insert.\n");
    }

    if (Err->DtorNodeThatChildrenHas == 1)
    {
        COLOR_PRINT(RED, "Error: Dtor node that childern has.\n");
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Function)
{
    COLOR_PRINT(WHITE, "File [%s]\nLine [%d]\nFunc [%s]\n", File, Line, Function);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void TreeAssertPrint(TreeErrorType* Err, const char* File, int Line, const char* Func)
{
    if (Err->IsFatalError == 1) 
    {
        COLOR_PRINT(RED, "\nAssert made in:\n");
        PrintPlace(File, Line, Func);
        PrintError(Err);
        PrintPlace(Err->Place.File, Err->Place.Line, Err->Place.Func);
        printf("\n");
    }
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void CodePlaceCtor(TreeErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTypeInStrFormat(NodeArgType type)
{
    switch (type)
    {
    case NodeArgType::number:
        return "num";
        break;
    
    case NodeArgType::operation:
        return "opr";
        break;

    case NodeArgType::variable:
        return "var";
        break;
    
    default:
        return "und";
        break;
    }
    return "und";
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void PrintPrefTree(const Tree_t* tree)
{
    assert(tree);

    COLOR_PRINT(GREEN, "Pref tree:\n");
    PrintPrefTreeHelper(tree->root);
    printf("\n");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintPrefTreeHelper(const Node_t* node)
{
    if (node == NULL)
    {
        return;
    }
    
    printf("( ");

    printf("'%s, type = %s' ", node->data.arg, GetTypeInStrFormat(node->data.type));

    if (node->left)
    {
        PrintPrefTreeHelper(node->left);
    }

    else
    {
        printf("(*) ");
    }


    if (node->right)
    {
        PrintPrefTreeHelper(node->right);
    }

    else
    {
        printf("(*) ");
    }


    printf(") ");

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void PrintInfixTree(const Tree_t* tree)
{
    assert(tree);

    COLOR_PRINT(GREEN, "infix tree:\n");
    PrintInfixTreeHelper(tree->root);
    printf("\n");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintInfixTreeHelper(const Node_t* node)
{
    if (node == NULL)
    {
        return;
    }
    
    printf("(");

    if (node->left)
    {
        PrintInfixTreeHelper(node->left);
    }

    else
    {
        // printf("(*) ");-
    }

    printf("%s", node->data.arg);


    if (node->right)
    {
        PrintInfixTreeHelper(node->right);
    }

    // else
    // {
    //     printf("(*) ");
    // }


    printf(")");

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
