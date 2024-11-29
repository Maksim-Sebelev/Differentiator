#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "Tree.h"

#include "../Onegin/onegin.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"
#include "../Differentiator/DiffDump.h"

static bool         IsError                        (const TreeErr* err);

static bool         IsOperation                    (const char* const str);
static bool         IsFunction                     (const char* const str);
static bool         IsVariable                     (const char* const str);
static bool         IsNumber                       (const char* const str);

static bool         HasntNumChild                  (const Node_t* node);
static bool         HasntVarChild                  (const Node_t* node);
static bool         HasOperationChildren           (const Node_t* node);
static bool         HasFuncLeftChildOnly           (const Node_t* node);

static bool         IsNodeOperTypeUndef            (const Node_t* node);
static bool         IsNodeFuncTypeUndef            (const Node_t* node);
static bool         IsNodeVariableTypeUndef        (const Node_t* node);
static void         IsOnlyOneNodeTypeNotUndefined  (const Node_t* node, TreeErr* err);



static Node_t*      GetNode                (const char** buffer, size_t bufSize, size_t* buffer_i);

static TreeErr      SetNodeType            (Node_t* node, const char* arg);
static NodeArgType  GetNodeType            (const char* arg);
static bool         IsLeftBracket          (const char** buffer, size_t* buffer_i);
static bool         IsRightBracket         (const char** buffer, size_t* buffer_i);
static bool         IsNull                 (const char** buffer, size_t* buffer_i);


static TreeErr      NodeVerifForUnfinishedNode (const Node_t* node, TreeErr* err, const char* file, const int line, const char* func);
static void         PrintError                 (const TreeErr* err);
static TreeErr      AllNodeVerif               (const Node_t* node, size_t* treeSize);
static TreeErr      TreeDtorHelper             (Node_t* node, TreeErr* Err);

#define UNFINISHED_NODE_VERIF(node, err) NodeVerifForUnfinishedNode(node, &err, __FILE__, __LINE__, __func__)

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

    TREE_ASSERT(TreeDtorHelper(tree->root, &Err));

    tree->size = 0;
    tree->root = NULL;

    BufferDtor(buffer);

    return TREE_VERIF(tree, Err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr TreeDtorHelper(Node_t* node, TreeErr* Err)
{
    if (node == nullptr)
    {
        return *Err;
    }

    if (node->left)
    {
        TreeDtorHelper(node->left,  Err);
    }

    if (node->right)
    {
        TreeDtorHelper(node->right, Err);
    }

    NodeDtor(node);

    return *Err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeCtor(Node_t** node, NodeArgType type, Number num, Operation oper, Function func, Variable var, Node_t* left, Node_t* right)
{
    TreeErr err = {};

    *node = (Node_t*) calloc(1, sizeof(Node_t));

    if (*node == NULL)
    {
        err.err = TreeErrorType::CTOR_CALLOC_RETURN_NULL;
        return NODE_VERIF(*node, err);
    }

    (*node)->data.type = type;

    (*node)->data.num  = num;
    (*node)->data.oper = oper;
    (*node)->data.func = func;
    (*node)->data.var  = var;

    (*node)->left      = left;
    (*node)->right     = right;


    return NODE_VERIF(*node, err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeDtor(Node_t* node)
{
    assert(node);

    TreeErr err = {};

    // node->left  = nullptr;
    // node->right = nullptr;

    FREE(node);

    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeCopy(Node_t** copy, const Node_t* node)
{
    assert(node);

    TreeErr err = {};

    NodeArgType type = node->data.type;
    Number      num  = node->data.num;
    Operation   oper = node->data.oper;
    Function    func = node->data.func;
    Variable    var  = node->data.var;

    TREE_ASSERT(NodeCtor(copy, type, num, oper, func, var, node->left, node->right));


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

TreeErr NodeSetCopy(Node_t* copy, const Node_t* node)
{
    assert(node);

    TreeErr err = {};

    NodeArgType type  = node->data.type;
    Number      num   = node->data.num;
    Operation   oper  = node->data.oper;
    Function    func  = node->data.func;
    Variable    var   = node->data.var;
    Node_t*     left  = node->left;
    Node_t*     right = node->right;

    TREE_ASSERT(SetNode(copy, type, num, oper, func, var, left, right));

    return NODE_VERIF(copy, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------

TreeErr SetNode(Node_t* node, NodeArgType type, Number num, Operation oper, Function func, Variable var, Node_t* left, Node_t* right)
{
    assert(node);

    TreeErr err = {};
    NODE_RETURN_IF_ERR(node, err);

    node->data.type = type;
    node->data.num  = num;
    node->data.oper = oper;
    node->data.func = func;
    node->data.var  = var;
    node->left      = left;
    node->right     = right;

    return NODE_VERIF(node, err);
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
    node = (Node_t*) calloc(1, sizeof(Node_t));

    if (IsLeftBracket(buffer, buffer_i))
    {
        (*buffer_i)++;
        const char* arg = buffer[*buffer_i];
        TREE_ASSERT(SetNodeType(node, arg));
        (*buffer_i)++;
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

static TreeErr SetNodeType(Node_t* node, const char* arg)
{
    assert(arg);
    assert(node);

    TreeErr err = {};

    NodeArgType type = GetNodeType(arg);
    node->data.type = type;

    switch (type)
    {
        case NodeArgType::number:
            node->data.num  = GetNumber(arg);
            break;

        case NodeArgType::operation:
            node->data.oper = GetOperationType(arg);
            break;

        case NodeArgType::function:
            node->data.func = GetFunctionType(arg);
            break;

        case NodeArgType::variable:
            node->data.var  = GetVariableType(arg);
            break;

        case NodeArgType::undefined:
            err.err = TreeErrorType::UNDEFINED_NODE_TYPE;
            break;

        default:
            assert(0 && "You forgot about some node type in tree.cpp\n");
            break;
    }
    

    return UNFINISHED_NODE_VERIF(node, err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static NodeArgType GetNodeType(const char* arg)
{
    NodeArgType type = NodeArgType::undefined;

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
        COLOR_PRINT(RED, "arg = '%s'\nundefined arg.\n", arg);
        assert(0 && "undefined NodeArgType.\n");
    }

    return type;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

Number GetNumber(const char* number)
{
    assert(number);
    assert(IsNumber(number));

    return (Number) strtol(number, nullptr, 10);
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
        return Operation::power;

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

    if (strcmp(function, "cth")    == 0)
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

Variable GetVariableType(const char* variable)
{
    if (strcmp(variable, "x") == 0)
        return Variable::x;
    
    if (strcmp(variable, "y") == 0)
        return Variable::y;
    
    return Variable::undefined_variable;
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
    return (c == 'x') || (c == 'y');
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

    if (node->data.oper == Operation::minus)
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

static bool IsError(const TreeErr* err)
{
    assert(err);
    return err->err != TreeErrorType::NO_ERR;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr TreeVerif(const Tree_t* tree, TreeErr* err, const char* file, const int line, const char* func)
{

    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    RETURN_IF_FALSE(tree->root, *err);

    RETURN_IF_TRUE(IsError(err), *err);

    size_t treeSize = 0;
    *err = AllNodeVerif(tree->root, &treeSize);

    RETURN_IF_TRUE(IsError(err), *err);
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

    switch (type)
    {
        case NodeArgType::number:
        {
            RETURN_IF_FALSE(HasntNumChild(node), *err, err->err = TreeErrorType::NUM_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::variable:
        {
            RETURN_IF_FALSE(IsNodeVariableTypeUndef(node), *err, err->err = TreeErrorType::VAR_TYPE_NODES_ARG_IS_UNDEFINED);
            RETURN_IF_FALSE(HasntVarChild          (node), *err, err->err = TreeErrorType::VAR_HAS_INCORRECT_CHILD_QUANT);

            break;
        }

        case NodeArgType::operation:
        {
            RETURN_IF_FALSE(IsNodeOperTypeUndef (node), *err, err->err = TreeErrorType::OPER_TYPE_NODES_ARG_IS_UNDEFINED);
            RETURN_IF_FALSE(HasOperationChildren(node), *err, err->err = TreeErrorType::OPER_HAS_INCORRECT_CHILD_QUANT);
            break;
        }

        case NodeArgType::function:
        {
            RETURN_IF_FALSE(IsNodeFuncTypeUndef (node), *err, err->err = TreeErrorType::FUNC_TYPE_NODES_ARG_IS_UNDEFINED);
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
            assert(0 && "you forgot about some node type in NodeVerif in tree.cpp.\n");
            break;
        }
    }
    
    IsOnlyOneNodeTypeNotUndefined(node, err);

    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr NodeVerifForUnfinishedNode(const Node_t* node, TreeErr* err, const char* file, const int line, const char* func)
{
    assert(node);
    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    IsOnlyOneNodeTypeNotUndefined(node, err);

    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeOperTypeUndef(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::operation);

    return node->data.oper != Operation::undefined_operation;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeFuncTypeUndef(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::function);

    return node->data.func != Function::undefined_function;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNodeVariableTypeUndef(const Node_t* node)
{
    assert(node);
    assert(node->data.type == NodeArgType::variable);

    return node->data.var != Variable::undefined_variable;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void IsOnlyOneNodeTypeNotUndefined(const Node_t* node, TreeErr* err)
{
    assert(node);
    assert(err);

    NodeArgType type = node->data.type;
    Number      num  = node->data.num;
    Operation   oper = node->data.oper;
    Function    func = node->data.func;
    Variable    var  = node->data.var;

    bool num_flag   = (num == 0);
    bool oper_flag  = (oper == Operation::undefined_operation);
    bool func_flag  = (func == Function:: undefined_function);
    bool var_flag   = (var  == Variable:: undefined_variable);

    bool BigNumFlag  = (oper_flag && func_flag && var_flag);
    bool BigOperFlag = (num_flag  && func_flag && var_flag);
    bool BigFuncFlag = (num_flag  && oper_flag && var_flag);
    bool BigVarFlag  = (num_flag  && func_flag && oper_flag); 


    if (type == NodeArgType::number    && !BigNumFlag)
    {
        err->err = NODE_IS_NUM_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED;
        return;
    }

    if (type == NodeArgType::operation && !BigOperFlag)
    {
        err->err = NODE_IS_OPERATION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED;
        return;
    }

    if (type == NodeArgType::function  && !BigFuncFlag)
    {
        err->err = NODE_IS_FUNCTION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED;
        return;
    }

    if (type == NodeArgType::variable  && !BigVarFlag)
    {
        err->err = NODE_IS_VARIABLE_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED;
        return;
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static TreeErr AllNodeVerif(const Node_t* node, size_t* treeSize)
{
    assert(node);
    assert(treeSize);

    TreeErr err = {};

    TREE_ASSERT(NODE_VERIF(node, err));

    if (node->left)
    {
        (*treeSize)++;
        err = AllNodeVerif(node->left, treeSize);
        RETURN_IF_TRUE(IsError(&err), err);
    }

    if (node->right)
    {
        (*treeSize)++;
        err = AllNodeVerif(node->right, treeSize);
        RETURN_IF_TRUE(IsError(&err), err);
    }

    return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const TreeErr* Err)
{
    assert(Err);
    TreeErrorType err = Err->err;

    switch (err)
    {
        case TreeErrorType::NO_ERR:
            return;
            break;
    
        case TreeErrorType::CTOR_CALLOC_RETURN_NULL:
            COLOR_PRINT(RED, "Error: failed alocate memory in ctor.\n");
            break;

        case TreeErrorType::INSERT_INCORRECT_SITUATION:
            COLOR_PRINT(RED, "Error: undefined situation in insert.\n");
            break;

        case TreeErrorType::DTOR_NODE_WITH_CHILDREN:
            COLOR_PRINT(RED, "Error: Dtor node that childern has.\n");
            break;
        
        case TreeErrorType::INCORRECT_TREE_SIZE:
            COLOR_PRINT(RED, "Error: Incorrect tree size.\n");
            break;

        case TreeErrorType::NUM_TYPE_NODES_ARG_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'number' type, but arg is undefined.\n");
            break;

        case TreeErrorType::NUM_HAS_INCORRECT_CHILD_QUANT:
            COLOR_PRINT(RED, "Error: Node has 'number' type, but child quant is incorrect.\n");
            break;
        
        case TreeErrorType::VAR_TYPE_NODES_ARG_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'variable' type, but arg is undefined.\n");
            break;

        case TreeErrorType::VAR_HAS_INCORRECT_CHILD_QUANT:
            COLOR_PRINT(RED, "Error: Node has 'varible' type, but child quant is incorrect.\n");
            break;

        case TreeErrorType::OPER_TYPE_NODES_ARG_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'operation' type, but arg is undefinef.\n");
            break;

        case TreeErrorType::OPER_HAS_INCORRECT_CHILD_QUANT:
            COLOR_PRINT(RED, "Error: Node has 'operation' type, but child quant is incorrect.\n");
            break;

        case TreeErrorType::FUNC_TYPE_NODES_ARG_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'function' type, but arg is undefinef type.\n");
            break;

        case TreeErrorType::FUNC_HAS_INCORRECT_CHILD_QUANT:
            COLOR_PRINT(RED, "Error: Node has 'function' type, but child quant is incorrect.\n");
            break;

        case TreeErrorType::UNDEFINED_NODE_TYPE:
            COLOR_PRINT(RED, "Error: Node has undefined type.\n");
            break;

        case TreeErrorType::NODE_IS_NUM_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'number' type, but not only her 'number' type is defined.\n");
            break;

        case TreeErrorType::NODE_IS_OPERATION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'operation' type, but not only her 'operation' type is defined.\n");
            break;

        case TreeErrorType::NODE_IS_FUNCTION_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'function' type, but not only her 'function' type is defined.\n");
            break;

        case TreeErrorType::NODE_IS_VARIABLE_TYPE_BUT_OTHER_NODE_TYPED_IS_UNDEFINED:
            COLOR_PRINT(RED, "Error: Node has 'variable' type, but not only her 'variable' type is defined.\n");
            break;
        
        case TreeErrorType::UNDEFINED_FUNCTION_TYPE:
            COLOR_PRINT(RED, "Error: Node has 'function' type, bit it is undefined.\n");
            break;

        case TreeErrorType::UNDEFINED_OPERATION_TYPE:
            COLOR_PRINT(RED, "Error: Node has 'operation' type, bit it is undefined.\n");
            break;

        case TreeErrorType::DIVISION_BY_0:
            COLOR_PRINT(RED, "Error: division by 0.\n");
            break;

        default:
            assert(0 && "you forgot about some error in print error.\n");
            return;
            break;
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void TreeAssertPrint(TreeErr* err, const char* file, const int line, const char* func)
{
    assert(err);
    assert(file);
    assert(func);
    COLOR_PRINT(RED, "\nAssert made in:\n");
    PrintPlace(file, line, func);
    PrintError(err);
    PrintPlace(err->place.file, err->place.line, err->place.func);
    printf("\n");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef UNFINISHED_NODE_VERIF
