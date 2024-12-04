#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "Tree.h"

#include "../Onegin/onegin.h"
#include "../Common/ColorPrint.h"
#include "../Common/GlobalInclude.h"
#include "TreeDump.h"

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


static Node_t* GetAddSub   (const char* input, size_t* pointer);
static Node_t* GetMulDiv   (const char* input, size_t* pointer);
static Node_t* GetBracket   (const char* input, size_t* pointer);
static Node_t* GetPow   (const char* input, size_t* pointer);
static Node_t* GetVariable   (const char* input, size_t* pointer);
static Node_t* GetNumber   (const char* input, size_t* pointer);
static Node_t* GetFunction   (const char* input, size_t* pointer);

static Node_t* GetFuncPattern(const char* input, size_t* pointer, const char* func);

static Node_t* GetMinus  (const char* input, size_t* pointer);
static Node_t* GetLn     (const char* input, size_t* pointer);
static Node_t* GetSin    (const char* input, size_t* pointer);
static Node_t* GetCos    (const char* input, size_t* pointer);
static Node_t* GetTg     (const char* input, size_t* pointer);
static Node_t* GetCtg    (const char* input, size_t* pointer);
static Node_t* GetSh     (const char* input, size_t* pointer);
static Node_t* GetCh     (const char* input, size_t* pointer);
static Node_t* GetTh     (const char* input, size_t* pointer);
static Node_t* GetCth    (const char* input, size_t* pointer);
static Node_t* GetArccos (const char* input, size_t* pointer);
static Node_t* GetArcsin (const char* input, size_t* pointer);
static Node_t* GetArctg  (const char* input, size_t* pointer);
static Node_t* GetArcctg (const char* input, size_t* pointer);

static void SyntaxError  (const char* input, size_t pointer, const char* file, const int line, const char* func);

static bool IsEndSymbol           (const char* input, size_t pointer);
static bool IsOperation           (const char* input, size_t pointer);
static bool IsNumSymbol           (const char* input, size_t* pointer);
static bool IsVarSymbol           (const char* input, size_t* pointer);
static bool IsAddSub              (const char* input, size_t* pointer);
static bool IsMulDiv              (const char* input, size_t* pointer);
static bool IsPow                 (const char* input, size_t* pointer);
static bool IsLeftBracket         (const char* input, size_t* pointer);
static bool IsRightBracket        (const char* input, size_t* pointer);
static bool IsOperationBeforeMinus(const char* input, size_t* pointer);


static Node_t*      GetNode                (const char** buffer, size_t bufSize, size_t* buffer_i);

static TreeErr      SetNodeType            (Node_t* node, const char* arg);
static NodeArgType  GetNodeType            (const char* arg);
static bool         IsLeftBracket          (const char** buffer, size_t* buffer_i);
static bool         IsRightBracket         (const char** buffer, size_t* buffer_i);
static bool         IsNull                 (const char** buffer, size_t* buffer_i);


static TreeErr      NodeVerifForUnfinishedNode (const Node_t* node, TreeErr* err, const char* file, const int line, const char* func);
static void         PrintError                 (const TreeErr* err);
static TreeErr      AllNodeVerif               (const Node_t* node, size_t* treeSize);


#define UNFINISHED_NODE_VERIF(node, err) NodeVerifForUnfinishedNode(node, &err, __FILE__, __LINE__, __func__)


#define SYNTAX_ERR(input, pointer) SyntaxError(input, pointer, __FILE__, __LINE__, __func__)

//------------------------------------------------------------------------------------------------------------------------------------------------------

Node_t* GetTree(const char* input)
{
    assert(input);
    size_t pointer = 0;
    Node_t* node = GetAddSub(input, &pointer);

    if (!IsEndSymbol(input, pointer)) 
        SYNTAX_ERR(input, pointer);

    pointer++;
    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr TreeDtor(Tree_t* tree, const char** buffer)
{
    assert(tree);
    assert(tree->root);

    TreeErr Err = {};

    TREE_ASSERT(NodeAndUnderTreeDtor(tree->root));

    tree->size = 0;
    tree->root = nullptr;

    return TREE_VERIF(tree, Err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

TreeErr NodeAndUnderTreeDtor(Node_t* node)
{
    TreeErr err = {};

    if (node == nullptr)
    {
        return err;
    }

    if (node->left)
    {
        NodeAndUnderTreeDtor(node->left);
    }

    if (node->right)
    {
        NodeAndUnderTreeDtor(node->right);
    }

    NodeDtor(node);

    return err;
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

    node->left  = nullptr;
    node->right = nullptr;

    

    FREE(node);

    assert(!node);

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

TreeErr SwapNode(Node_t** node1, Node_t** node2)
{
    assert(node1);
    assert(node2);

    TreeErr err = {};

    Node_t* temp = *node1;
    *node1 = *node2;
    *node2 = temp;

    err = NODE_VERIF(*node1, err);
    if (IsError(&err)) return err;
    return NODE_VERIF(*node2, err);
}

//===================================== Read Tree ==================================================================================


static Node_t* GetNumber(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);
    Number val = 0;
    size_t old_pointer = *pointer;

    while (IsNumSymbol(input, pointer))
    {
        val = 10 * val + input[*pointer] - '0';
        (*pointer)++;   
    }

    if (*pointer == old_pointer) SYNTAX_ERR(input, *pointer);

    Node_t* node = {};
    _NUM(&node, val);

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetVariable(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);
    Node_t* node = {};
    size_t old_pointer = *pointer;

    if (IsVarSymbol(input, pointer))
    {
        _VAR(&node, Variable::x);
        (*pointer)++;
    }

    if (*pointer == old_pointer) SYNTAX_ERR(input, *pointer);

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetAddSub(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);
    Node_t* node = GetMulDiv(input, pointer);
    while(IsAddSub(input, pointer))
    {
        char op = input[*pointer];

        (*pointer)++;

        Node_t* node2 = GetMulDiv(input, pointer);
        Node_t* new_node = {};

        if (op == '+')
            _ADD(&new_node, node, node2);
        else
            _SUB(&new_node, node, node2);

        TREE_ASSERT(SwapNode(&node, &new_node));
    }

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetMulDiv(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);
    Node_t* node = GetPow(input, pointer);

    while (IsMulDiv(input, pointer))
    {
        char operation = input[*pointer];

        (*pointer)++;
        Node_t* node2 = GetPow(input, pointer);
        Node_t* new_node = {};
    
        if (operation == '*')
            _MUL(&new_node, node, node2);
        else if (operation == '/')
            _DIV(&new_node, node, node2);
        else 
            assert(0 && "not a */ operation in get mul div.");

        TREE_ASSERT(SwapNode(&node, &new_node));
    }
    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetBracket(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    if (IsLeftBracket(input, pointer))
    {
        (*pointer)++;
        Node_t* node = GetAddSub(input, pointer);
    
        if (!IsRightBracket(input, pointer))
            SYNTAX_ERR(input, *pointer);

        (*pointer)++;
        return node;
    }

    RETURN_IF_TRUE(IsVarSymbol(input, pointer), GetVariable(input, pointer));

    return GetNumber(input, pointer);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetPow(const char* input, size_t* pointer) // pow
{
    assert(pointer);
    assert(input);

    Node_t* node = GetFunction(input, pointer);
    while(IsPow(input, pointer))
    {
        (*pointer)++;
        Node_t* node2 = GetFunction(input, pointer);

        Node_t* new_node = {};
        _POW(&new_node, node, node2);
        TREE_ASSERT(SwapNode(&node, &new_node));
    }

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(func) (strncmp(input + *pointer, func, strlen(func)) == 0)

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetFunction(const char* input, size_t* pointer) // funtion
{
    assert(pointer);
    assert(input);


    RETURN_IF_TRUE(STRNCMP("-"),      GetMinus   (input, pointer));
    RETURN_IF_TRUE(STRNCMP("ln"),     GetLn      (input, pointer));
    RETURN_IF_TRUE(STRNCMP("sin"),    GetSin     (input, pointer));
    RETURN_IF_TRUE(STRNCMP("cos"),    GetCos     (input, pointer));
    RETURN_IF_TRUE(STRNCMP("tg"),     GetTg      (input, pointer));
    RETURN_IF_TRUE(STRNCMP("ctg"),    GetCtg     (input, pointer));
    RETURN_IF_TRUE(STRNCMP("sh"),     GetSh      (input, pointer));
    RETURN_IF_TRUE(STRNCMP("ch"),     GetCh      (input, pointer));
    RETURN_IF_TRUE(STRNCMP("th"),     GetTh      (input, pointer));
    RETURN_IF_TRUE(STRNCMP("cth"),    GetCth     (input, pointer));
    RETURN_IF_TRUE(STRNCMP("arcsin"), GetArcsin  (input, pointer));
    RETURN_IF_TRUE(STRNCMP("arccos"), GetArccos  (input, pointer));
    RETURN_IF_TRUE(STRNCMP("arctg"),  GetArctg   (input, pointer));
    RETURN_IF_TRUE(STRNCMP("arcctg"), GetArcctg  (input, pointer));

    return GetBracket(input, pointer);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------


static Node_t* GetFuncPattern(const char* input, size_t* pointer, const char* func)
{
    assert(input);
    assert(pointer);
    assert(func);
    assert(STRNCMP(func));

    Function funcType    = GetFunctionType(func);
    size_t   funcNameLen = strlen(func);

    *pointer += funcNameLen;

    if (!IsLeftBracket(input, pointer))
        SYNTAX_ERR(input, *pointer);

    (*pointer)++;

    Node_t* node = GetAddSub(input, pointer);

    if (!IsRightBracket(input, pointer)) 
        SYNTAX_ERR(input, *pointer);

    (*pointer)++;


    Node_t* funcNode = {};
    _FUNC(&funcNode, funcType, node);

    TREE_ASSERT(SwapNode(&funcNode, &node));

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef STRNCMP

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetLn(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "ln";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetSin(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "sin";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetCos(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "cos";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetTg(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "tg";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetCtg(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "ctg";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetSh(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "sh";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetCh(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "ch";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetTh(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "th";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetCth(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "cth";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetArcsin(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "arcsin";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetArccos(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "arccos";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetArctg(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "arctg";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetArcctg(const char* input, size_t* pointer)
{
    assert(pointer);
    assert(input);

    const char* func = "arcctg";

    return GetFuncPattern(input, pointer, func);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetMinus(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);
    assert(input[*pointer] == '-');

    if (IsOperationBeforeMinus(input, pointer)) SYNTAX_ERR(input, *pointer);
    (*pointer)++;

    size_t old_pointer = *pointer;

    Node_t* node = GetMulDiv(input, pointer);

    if (old_pointer == *pointer) SYNTAX_ERR(input, *pointer);

    Node_t* new_node = {};
    _SUB(&new_node, node, nullptr);

    TREE_ASSERT(SwapNode(&node, &new_node));

    return node;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRCMP(oper) (strcmp(operation, oper) == 0)

Operation GetOperationType(const char* operation)
{
    assert(operation);

    RETURN_IF_TRUE(STRCMP("+"), Operation::plus);
    RETURN_IF_TRUE(STRCMP("-"), Operation::minus);
    RETURN_IF_TRUE(STRCMP("*"), Operation::mul);
    RETURN_IF_TRUE(STRCMP("/"), Operation::dive);
    RETURN_IF_TRUE(STRCMP("^"), Operation::power);

    return Operation::undefined_operation;
}

#undef STRCMP

//--------------------------------------------------------------------------------------------------------------------------------------

#define STRCMP(func) (strcmp(function, func) == 0)

Function GetFunctionType(const char* function)
{
    assert(function);

    RETURN_IF_TRUE(STRCMP("ln")    , Function::ln    );
    RETURN_IF_TRUE(STRCMP("sin")   , Function::sin   );
    RETURN_IF_TRUE(STRCMP("cos")   , Function::cos   );
    RETURN_IF_TRUE(STRCMP("tg")    , Function::tg    );
    RETURN_IF_TRUE(STRCMP("ctg")   , Function::ctg   );
    RETURN_IF_TRUE(STRCMP("sh")    , Function::sh    );
    RETURN_IF_TRUE(STRCMP("ch")    , Function::ch    );
    RETURN_IF_TRUE(STRCMP("th")    , Function::th    );
    RETURN_IF_TRUE(STRCMP("cth")   , Function::cth   );
    RETURN_IF_TRUE(STRCMP("arcsin"), Function::arcsin);
    RETURN_IF_TRUE(STRCMP("arcos") , Function::arccos);
    RETURN_IF_TRUE(STRCMP("arctg") , Function::arctg );
    RETURN_IF_TRUE(STRCMP("arcctg"), Function::arcctg);


    return Function::undefined_function;
}

#undef STRCMP

//--------------------------------------------------------------------------------------------------------------------------------------

#define STRCMP(var) (strcmp(variable, var) == 0)

Variable GetVariableType(const char* variable)
{


    RETURN_IF_TRUE(STRCMP("x"), Variable::x);
    RETURN_IF_TRUE(STRCMP("y"), Variable::y);

    return Variable::undefined_variable;
}

#undef STRCMP    

//--------------------------------------------------------------------------------------------------------------------------------------


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
        return (node->left);

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

#define STRCMP(func) (strcmp(str, func) == 0)

static bool IsFunction(const char* const str)
{

    if (
    STRCMP("ln")     ||
    STRCMP("sin")    ||
    STRCMP("cos")    ||
    STRCMP("tg")     ||
    STRCMP("ctg")    ||
    STRCMP("sh")     ||
    STRCMP("ch")     ||
    STRCMP("th")     ||
    STRCMP("cth")    ||
    STRCMP("arcsin") ||
    STRCMP("arccos") ||
    STRCMP("arctg")  ||
    STRCMP("arcctg") ||
    STRCMP("arcsh")  ||
    STRCMP("arcch")  ||
    STRCMP("arcth")  ||
    STRCMP("arccth")
    ) 
        return true;

    return false;
}

#undef STRCMP

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
    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    RETURN_IF_FALSE(node, *err);

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
    
        case TreeErrorType::NODE_NULL:
            COLOR_PRINT(RED, "Error: node is nullptr and now this is bad.\n");
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

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void SyntaxError(const char* input, size_t pointer, const char* file, const int line, const char* func)
{
    COLOR_PRINT(RED, "\nSyntaxErr detected in:\n");
    PrintPlace(file, line, func);
    
    COLOR_PRINT(RED, "\nSyntaxErr:\n");
    const char* str = "input: \"";
    size_t strSize = strlen(str);

    COLOR_PRINT(CYAN, "%s%s\"\n", str, input);
    for (size_t i = 0; i < pointer + strSize; i++) printf(" ");

    COLOR_PRINT(RED, "^ p = %lu\n\n", pointer);

    exit(0);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsEndSymbol(const char* input, size_t pointer)
{
    assert(input);

    return input[pointer] == '$';
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNumSymbol(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return ('0' <= input[*pointer]) && (input[*pointer] <= '9');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsVarSymbol(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return input[*pointer] == 'x';
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsAddSub(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (input[*pointer] == '+') || (input[*pointer] == '-');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsMulDiv(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (input[*pointer] == '*') || (input[*pointer] == '/');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPow(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (input[*pointer] == '^');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLeftBracket(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (input[*pointer] == '(');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRightBracket(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (input[*pointer] == ')');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperation(const char* input, size_t pointer)
{
    char c = input[pointer];
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '^');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationBeforeMinus(const char* input, size_t* pointer)
{
    assert(input);
    assert(pointer);

    return (*pointer >= 1 && IsOperation(input, *pointer - 1));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef UNFINISHED_NODE_VERIF
#undef SYNTAX_ERR