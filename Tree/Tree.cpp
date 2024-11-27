#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "Tree.h"

#include "../Onegin/onegin.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"


static Node_t*      GetNode                (const char** buffer, size_t bufSize, size_t* buffer_i);

static bool         IsntError              (const TreeErr* err);

static bool         IsOperation            (const char* const str);
static bool         IsFunction             (const char* const str);
static bool         IsVariable             (const char* const str);
static bool         IsNumber               (const char* const str);

static bool         HasntNumChild          (const Node_t* node);
static bool         HasntVarChild          (const Node_t* node);
static bool         HasOperationChildren   (const Node_t* node);
static bool         HasFuncLeftChildOnly   (const Node_t* node);
   

static TreeElem_t   GetNodeType            (const char** buffer, size_t* buffer_i);
static bool         IsLeftBracket          (const char** buffer, size_t* buffer_i);
static bool         IsRightBracket         (const char** buffer, size_t* buffer_i);
static bool         IsNull                 (const char** buffer, size_t* buffer_i);

static void        PrintInfixTreeHelper    (const Node_t* node);

static void        PrintError              (const TreeErr* err);
static TreeErr     AllNodeVerif            (const Node_t* node, TreeErr* err, size_t* treeSize);
static TreeErr     NodeVerifHelper         (const Node_t* node, TreeErr* err);
static TreeErr     TreeDtorHelper          (Node_t** node, TreeErr* Err);


//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr TreeCtor(Tree_t* tree, const char** buffer, size_t bufSize)
{
    assert(tree);

    TreeErr Err = {};

    size_t buffer_i = 0;
    tree->root = GetNode(buffer, bufSize, &buffer_i);

    return TREE_VERIF(tree, Err);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------- TreeDtor(Tree_t* tree)

TreeErr TreeDtor(Tree_t* tree, const char** buffer)
{
    assert(tree);
    assert(tree->root);

    TreeErr Err = {};

    TREE_RETURN_IF_ERR(tree, TreeDtorHelper(&tree->root, &Err));

    tree->size = 0;
    tree->root = NULL;

    BufferDtor(buffer);

    return TREE_VERIF(tree, Err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr TreeDtorHelper(Node_t** node, TreeErr* Err)
{
    if (node == nullptr)
    {
        return *Err;
    }

    if ((*node)->left)
    {
        TreeDtorHelper(&(*node)->left,  Err);
    }

    if ((*node)->right)
    {
        TreeDtorHelper(&(*node)->right, Err);
    }

    NodeDtor(node);

    return *Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeCtor(Node_t** node, NodeArgType type, const char* arg,  Node_t* left, Node_t* right)
{
    TreeErr Err = {};

    *node = (Node_t*) calloc(1, sizeof(Node_t));

    if (*node == NULL)
    {
        Err.err = TreeErrorType::CTOR_CALLOC_RETURN_NULL;
        return Err;
    }

    (*node)->data.type = type;
    (*node)->data.arg  = arg;
    (*node)->left      = left;
    (*node)->right     = right;

    return Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeDtor(Node_t** node)
{
    assert(node);
    assert((*node)->left  == nullptr);
    assert((*node)->right == nullptr);

    TreeErr Err = {};

    (*node)->left  = nullptr;
    (*node)->right = nullptr;

    FREE(*node);

    return Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeCopy(Node_t** copy, const Node_t* node)
{
    assert(node);

    TreeErr err = {};

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

TreeErr SetNode(Node_t** node, NodeArgType type, const char* arg, Node_t* left, Node_t* right)
{
    assert(node);
    assert(*node);
    assert(arg);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(*node, err);

    (*node)->data.arg  = arg;
    (*node)->data.type = type;
    (*node)->left      = left;
    (*node)->right     = right;

    return NODE_VERIF(*node, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetNode(const char** buffer, size_t bufSize, size_t* buffer_i)
{
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

static TreeElem_t GetNodeType(const char** buffer, size_t* buffer_i)
{
    const char* arg  = buffer[*buffer_i];
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
    assert(operation);

    if (strcmp(operation, "+") == 0)
        return Operation::plus;

    if (strcmp(operation, "-") == 0)
        return Operation::minus;

    if (strcmp(operation, "*") == 0)
        return Operation::mul;

    if (strcmp(operation, "/") == 0)
        return Operation::dive;

    if (strcmp(operation, "^") == 0)
        return Operation::pow;

    return Operation::undefined_operation;
}

//--------------------------------------------------------------------------------------------------------------------------------------

Function GetFunctionType(const char* function)
{
    assert(function);

    if (strcmp(function, "ln")     == 0)
        return Function::ln;

    if (strcmp(function, "sin")    == 0)
        return Function::sin;
    
    if (strcmp(function, "cos")    == 0)
        return Function::cos;
    
    if (strcmp(function, "tg")     == 0)
        return Function::tg;

    if (strcmp(function, "ctg")    == 0)
        return Function::ctg;

    if (strcmp(function, "sh")     == 0)
        return Function::sh;

    if (strcmp(function, "ch")     == 0)
        return Function::ch;

    if (strcmp(function, "th")     == 0)
        return Function::th;

    if (strcmp(function, "cth")     == 0)
        return Function::cth;

    if (strcmp(function, "arcsin") == 0)
        return Function::arcsin;

    if (strcmp(function, "arccos") == 0)
        return Function::arccos;
    
    if (strcmp(function, "arctg")  == 0)
        return Function::arctg;
    
    if (strcmp(function, "arcctg") == 0)
        return Function::arcctg;
    
    return Function::undefined_function;
}

//--------------------------------------------------------------------------------------------------------------------------------------

static bool IsLeftBracket(const char** buffer, size_t* buffer_i)
{
    const char* bufElem = buffer[*buffer_i];
    return strcmp(bufElem, "(") == 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRightBracket(const char** buffer, size_t* buffer_i)
{
    const char* bufElem = buffer[*buffer_i];
    return strcmp(bufElem, ")") == 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNull(const char** buffer, size_t* buffer_i)
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

static bool HasntNumChild(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::number);

    return !(node->left) && !(node->right);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasntVarChild(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::variable);

    return !(node->left) && !(node->right);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasOperationChildren(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::operation);

    if (strcmp(node->data.arg, "-") == 0)
    {
        return (node->left);
    }

    return (node->left) && (node->right);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HasFuncLeftChildOnly(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::function);

    return (node->left) && !(node->right);
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

static bool IsntError(const TreeErr* err)
{
    assert(err);
    return err->err == TreeErrorType::NO_ERR;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr TreeVerif(const Tree_t* tree, TreeErr* err, const char* file, const int line, const char* func)
{

    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    RETURN_IF_FALSE(tree->root, *err);

    RETURN_IF_FALSE(IsntError(err), *err);

    size_t treeSize = 0;
    *err = AllNodeVerif(tree->root, err, &treeSize);

    RETURN_IF_FALSE(IsntError(err), *err);
    // RETURN_IF_FALSE(treeSize == tree->size, *err, err->err = TreeErrorType::INCORRECT_TREE_SIZE);

    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeVerif(const Node_t* node, TreeErr* err, const char* file, const int line, const char* func)
{
    assert(node);
    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    NodeArgType type = node->data.type;
    const char* arg  = node->data.arg;

    switch (type)
    {
        case NodeArgType::number:
        {
            RETURN_IF_FALSE(IsNumber(arg),       *err, err->err = TreeErrorType::NUM_TYPE_NODES_ARG_IS_NOT_NUM);
            RETURN_IF_FALSE(HasntNumChild(node), *err, err->err = TreeErrorType::NUM_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::variable:
        {
            RETURN_IF_FALSE(IsVariable(arg),     *err, err->err = TreeErrorType::VAR_TYPE_NODES_ARG_IS_NOT_VAR);
            RETURN_IF_FALSE(HasntVarChild(node), *err, err->err = TreeErrorType::VAR_HAS_INCORRECT_CHILD_QUANT);

            break;
        }

        case NodeArgType::operation:
        {
            RETURN_IF_FALSE(IsOperation(arg),           *err, err->err = TreeErrorType::OPER_TYPE_NODES_ARG_IS_NOT_OPER);
            RETURN_IF_FALSE(HasOperationChildren(node), *err, err->err = TreeErrorType::OPER_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::function:
        {
            RETURN_IF_FALSE(IsFunction(arg),            *err, err->err = TreeErrorType::FUNC_TYPE_NODES_ARG_IS_NOT_FUNC);
            RETURN_IF_FALSE(HasFuncLeftChildOnly(node), *err, err->err = TreeErrorType::FUNC_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::undefined:
        {
            err->err = TreeErrorType::UNDEFINED_NODE_TYPE;
            return *err;
            break;
        }

        default:
        {
            break;
        }
    }
    
    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr NodeVerifHelper(const Node_t* node, TreeErr* err)
{
    assert(node);
    assert(err);

    NodeArgType type = node->data.type;
    const char* arg  = node->data.arg;

    switch (type)
    {
        case NodeArgType::number:
        {
            RETURN_IF_FALSE(IsNumber(arg),       *err, err->err = TreeErrorType::NUM_TYPE_NODES_ARG_IS_NOT_NUM);
            RETURN_IF_FALSE(HasntNumChild(node), *err, err->err = TreeErrorType::NUM_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::variable:
        {
            RETURN_IF_FALSE(IsVariable(arg),     *err, err->err = TreeErrorType::VAR_TYPE_NODES_ARG_IS_NOT_VAR);
            RETURN_IF_FALSE(HasntVarChild(node), *err, err->err = TreeErrorType::VAR_HAS_INCORRECT_CHILD_QUANT);

            break;
        }

        case NodeArgType::operation:
        {
            RETURN_IF_FALSE(IsOperation(arg),           *err, err->err = TreeErrorType::OPER_TYPE_NODES_ARG_IS_NOT_OPER);
            RETURN_IF_FALSE(HasOperationChildren(node), *err, err->err = TreeErrorType::OPER_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::function:
        {
            RETURN_IF_FALSE(IsFunction(arg),            *err, err->err = TreeErrorType::FUNC_TYPE_NODES_ARG_IS_NOT_FUNC);
            RETURN_IF_FALSE(HasFuncLeftChildOnly(node), *err, err->err = TreeErrorType::FUNC_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::undefined:
        {
            err->err = TreeErrorType::UNDEFINED_NODE_TYPE;
            return *err;
            break;
        }

        default:
        {
            break;
        }
    }
    
    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr AllNodeVerif(const Node_t* node, TreeErr* err, size_t* treeSize)
{
    assert(node);
    assert(err);
    assert(treeSize);

    *err = NodeVerifHelper(node, err);
    RETURN_IF_FALSE(IsntError(err), *err);

    if (node->left)
    {
        (*treeSize)++;
        *err = AllNodeVerif(node->left, err, treeSize);
        RETURN_IF_FALSE(IsntError(err), *err);
    }

    if (node->right)
    {
        (*treeSize)++;
        *err = AllNodeVerif(node->right, err, treeSize);
        RETURN_IF_FALSE(IsntError(err), *err);
    }

    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const TreeErr* Err)
{
    if (IsntError(Err))
    {
        return;
    }

    if (Err->err == TreeErrorType::CTOR_CALLOC_RETURN_NULL)
    {
        COLOR_PRINT(RED, "Error: failed alocate memory in ctor.\n");
        return;
    }

    if (Err->err == TreeErrorType::INSERT_INCORRECT_SITUATION)
    {
        COLOR_PRINT(RED, "Error: undefined situation in insert.\n");
        return;
    }

    if (Err->err == TreeErrorType::DTOR_NODE_WITH_CHILDREN)
    {
        COLOR_PRINT(RED, "Error: Dtor node that childern has.\n");
        return;
    }

    if (Err->err == TreeErrorType::INCORRECT_TREE_SIZE)
    {
        COLOR_PRINT(RED, "Error: Incorrect tree size.\n");
        return;
    }

    if (Err->err == TreeErrorType::NUM_TYPE_NODES_ARG_IS_NOT_NUM)
    {
        COLOR_PRINT(RED, "Error: Node has 'number' type, but arg isn't 'number'.\n");
        return;
    }

    if (Err->err == TreeErrorType::NUM_HAS_INCORRECT_CHILD_QUANT)
    {
        COLOR_PRINT(RED, "Error: Node has 'number' type, but child quant is incorrect.\n");
        return;
    }

    if (Err->err == TreeErrorType::VAR_TYPE_NODES_ARG_IS_NOT_VAR)
    {
        COLOR_PRINT(RED, "Error: Node has 'variable' type, but arg isn't 'variable'.\n");
        return;
    }

    if (Err->err == TreeErrorType::VAR_HAS_INCORRECT_CHILD_QUANT)
    {
        COLOR_PRINT(RED, "Error: Node has 'varible' type, but child quant is incorrect.\n");
        return;
    }

    if (Err->err == TreeErrorType::OPER_TYPE_NODES_ARG_IS_NOT_OPER)
    {
        COLOR_PRINT(RED, "Error: Node has 'operation' type, but arg isn't 'operation'.\n");
        return;
    }

    if (Err->err == TreeErrorType::OPER_HAS_INCORRECT_CHILD_QUANT)
    {
        COLOR_PRINT(RED, "Error: Node has 'operation' type, but child quant is incorrect.\n");
        return;
    }

    if (Err->err == TreeErrorType::FUNC_TYPE_NODES_ARG_IS_NOT_FUNC)
    {
        COLOR_PRINT(RED, "Error: Node has 'function' type, but arg isn't 'function'.\n");
        return;
    }

    if (Err->err == TreeErrorType::FUNC_HAS_INCORRECT_CHILD_QUANT)
    {
        COLOR_PRINT(RED, "Error: Node has 'function' type, but child quant is incorrect.\n");
        return;
    }

    if (Err->err == TreeErrorType::UNDEFINED_NODE_TYPE)
    {
        COLOR_PRINT(RED, "Error: Node has undefined type.\n");
        return;
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void TreeAssertPrint(TreeErr* err, const char* file, const int line, const char* func)
{
    assert(err);
    assert(file);
    assert(func);

    if (!IsntError(err)) 
    {
        COLOR_PRINT(RED, "\nAssert made in:\n");
        PrintPlace(file, line, func);
        PrintError(err);
        PrintPlace(err->place.file, err->place.line, err->place.func);
        printf("\n");
    }
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

    printf("%s", node->data.arg);

    if (node->right)
    {
        PrintInfixTreeHelper(node->right);
    }

    printf(")");

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
