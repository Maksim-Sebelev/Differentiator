#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Token.h"
#include "Tree.h"
#include "TreeDump.h"
#include "RecursiveDescent.h"

static Node_t* GetNumber            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetVariable          (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetAddSub            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetMulDiv            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetBracket           (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetPow               (const Token_t* token, size_t* tp, const char* input);

static Node_t* GetFunction          (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetMinus             (const Token_t* token, size_t* tp, const char* input);


static Number    GetTokenNumber     (const Token_t* token, size_t* tp);
static Variable  GetTokenVariable   (const Token_t* token, size_t* tp);
static Operation GetTokenOperation  (const Token_t* token, size_t* tp);
static Function  GetTokenFunction   (const Token_t* token, size_t* tp);

static bool IsTokenEnd              (const Token_t* token, const size_t* tp);
static bool IsTokenNum              (const Token_t* token, const size_t* tp);
static bool IsTokenVariable         (const Token_t* token, const size_t* tp);
static bool IsTokenOperation        (const Token_t* token, const size_t* tp);
static bool IsTokenFunction         (const Token_t* token, const size_t* tp);

static bool IsAddSub                (const Token_t* token, const size_t* tp);
static bool IsMulDiv                (const Token_t* token, const size_t* tp);
static bool IsPow                   (const Token_t* token, const size_t* tp);
static bool IsTokenLeftBracket      (const Token_t* token, const size_t* tp);
static bool IsTokenRightBracket     (const Token_t* token, const size_t* tp);
static bool IsOperationBeforeMinus  (const Token_t* token, const size_t* tp);
static bool IsOperationToken        (const Token_t* token, const size_t  tp);
static bool IsTokenMinus            (const Token_t* token, const size_t* tp);

static void        SyntaxError (const Token_t* token, const char* input, const char* msg, const char* file, const int line, const char* func);
static const char* FindNline   (const char* str, size_t nLine);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SyntaxError(const Token_t* token, const char* input, const char* msg, const char* file, const int line, const char* func)
{
    assert(token);
    assert(msg);
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "\nSyntaxErr detected in:\n");
    PrintPlace(file, line, func);

    size_t errLine = token->place.line;
    size_t errPos  = token->place.placeInLine;


    COLOR_PRINT(RED, "\n%s\n", msg);
    COLOR_PRINT(WHITE, "line::%lu::%lu;\n\n", errLine, errPos);

    const char* LineWithErr = FindNline(input, errLine);

    printf("%s", LineWithErr);

    for (size_t i = 0; i < errPos; i++) printf(" ");

    printf("^ %s\n", msg);


    COLOR_PRINT(VIOLET, "\nabort() in 3, 2, 1...\n");
    exit(0);

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define SYNTAX_ERR(token, input, msg) SyntaxError(token, input, msg, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* FindNline(const char* str, size_t nLine)
{
    assert(str);

    nLine--;

    size_t lp = 0;  // line pointer
    size_t sp = 0;  // str  pointer
    size_t strLen = strlen(str);

    for (lp = 1; lp < nLine && lp < strLen; sp++)
    {
        if (str[sp] == '\n') lp++;
    }

    if (lp >= strLen)
    {
        assert(0 && "str doesn't have stolko lines\n");
    }

    return str + sp;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Node_t* GetTree(const Token_t* tokens, const char* input)
{
    assert(tokens);

    size_t tp = 0;
    Node_t* node = GetAddSub(tokens, &tp, input);

    if (!IsTokenEnd(tokens, &tp))
        SYNTAX_ERR(&tokens[tp], input, "Expected '$'");

    tp++;

    TreeErr err = {};

    TREE_ASSERT(NODE_VERIF(node, err));

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetNumber(const Token_t* token, size_t* tp, const char* input)
{
    assert(token);
    assert(tp);

    if (!IsTokenNum(token, tp))
        SYNTAX_ERR(&token[*tp], input, "Expected number");

    Number val = GetTokenNumber(token, tp);
    (*tp)++;  

    Node_t* node = {};
    _NUM(&node, val);

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetVariable(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);
    
    if (!IsTokenVariable(token, tp))
        SYNTAX_ERR(&token[*tp], input, "Expetcted variable name");

    Variable variable = GetTokenVariable(token, tp);
    (*tp)++;


    Node_t* node = {};
    _VAR(&node, variable);

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetAddSub(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);

    size_t old_tp = *tp;

    Node_t* node = GetMulDiv(token, tp, input);

    if (old_tp == *tp)
        SYNTAX_ERR(&token[*tp], input, "Expected math expression");
    
    while(IsAddSub(token, tp))
    {
        Operation operation = GetTokenOperation(token, tp);
        (*tp)++;  

        Node_t* node2 = GetMulDiv(token, tp, input);
        Node_t* new_node = {};

        if (operation == Operation::plus)
        {
            _ADD(&new_node, node, node2);
        }
    
        else if (operation == Operation::minus)
        {
            _SUB(&new_node, node, node2);
        }
    
        else
        {
            assert(0 && "incorrect operation type");
        }

        TREE_ASSERT(SwapNode(&node, &new_node));
    }

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetMulDiv(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);

    size_t old_tp = *tp;

    Node_t* node = GetPow(token, tp, input);

    if (old_tp == *tp)
        SYNTAX_ERR(&token[*tp], input, "Expected math expression");

    while (IsMulDiv(token, tp))
    {
        Operation operation = GetTokenOperation(token, tp);
        (*tp)++;  

        Node_t* node2 = GetPow(token, tp, input);
        Node_t* new_node = {};
    
        if (operation == Operation::mul)
        {
            _MUL(&new_node, node, node2);
        }

        else if (operation == Operation::dive)
        {
            _DIV(&new_node, node, node2);
        }

        else
        {
            assert(0 && "not a */ operation in get mul div");
        }

        TREE_ASSERT(SwapNode(&node, &new_node));
    }

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetBracket(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);

    if (IsTokenLeftBracket(token, tp))
    {
        (*tp)++;  
        Node_t* node = GetAddSub(token, tp, input);
    
        if (!IsTokenRightBracket(token, tp)) 
            SYNTAX_ERR(&token[*tp], input, "Expected ')'");

        (*tp)++;  
        return node;
    }

    RETURN_IF_TRUE(IsTokenVariable(token, tp), GetVariable(token, tp, input));

    return GetNumber(token, tp, input);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetPow(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);

    Node_t* node = GetFunction(token, tp, input);
    while(IsPow(token, tp))
    {
        (*tp)++;  
        Node_t* node2 = GetFunction(token, tp, input);

        Node_t* new_node = {};
        _POW(&new_node, node, node2);
        TREE_ASSERT(SwapNode(&node, &new_node));
    }

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetFunction(const Token_t* token, size_t* tp, const char* input)
{
    assert(tp);
    assert(token);

    TokenType type = token[*tp].type;

    if (type != TokenType::Function_t)
    {
        return GetBracket(token, tp, input);
    }

    if (IsTokenMinus(token, tp))
    {
        return GetMinus(token, tp, input);
    }

    Function function = GetTokenFunction(token, tp);

    TOKEN_TEXT_DUMP(token, *tp);

    (*tp)++;  

    if (!IsTokenLeftBracket(token, tp))
        SYNTAX_ERR(&token[*tp], input, "Expected '('");

    (*tp)++;  

    Node_t* node = GetAddSub(token, tp, input);

    if (!IsTokenRightBracket(token, tp))
        SYNTAX_ERR(&token[*tp], input, "Expected ')'");

    (*tp)++;  

    Node_t* funcNode = {};

    _FUNC(&funcNode, function, node);

    TEXT_NODE_DUMP(funcNode);


    TREE_ASSERT(SwapNode(&node, &funcNode));



    printf("\n\n\n\n\n\n\n");


    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetMinus(const Token_t* token, size_t* tp, const char* input)
{
    assert(token);
    assert(tp);

    if (IsOperationBeforeMinus(token, tp))
        SYNTAX_ERR(&token[*tp], input, "Operation before '-'");

    (*tp)++;  

    size_t old_tp = *tp;

    Node_t* node = GetMulDiv(token, tp, input);

    if (old_tp == *tp)
        SYNTAX_ERR(&token[*tp], input, "Nothig after '-'");

    Node_t* new_node = {};
    _SUB(&new_node, node, nullptr);

    TREE_ASSERT(SwapNode(&node, &new_node));

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenEnd(const Token_t* token, const size_t* tp)
{   
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    return (type == TokenType::EndSymbol_t);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenNum(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    return (type == TokenType::Number_t);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenVariable(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    return (type == TokenType::Variable_t);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenOperation(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    return (type == TokenType::Operation_t);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenFunction(const Token_t* token, const size_t* tp)
{   
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    return (type == TokenType::Function_t);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsAddSub(const Token_t* token, const size_t* tp)
{   
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    RETURN_IF_FALSE(type == TokenType::Operation_t, false);

    Operation operation = token[*tp].data.operation;

    return (operation == Operation::plus) || (operation == Operation::minus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsMulDiv(const Token_t* token, const size_t* tp)
{   
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    RETURN_IF_FALSE(type == TokenType::Operation_t, false);

    Operation operation = token[*tp].data.operation;

    return (operation == Operation::mul) || (operation == Operation::dive);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPow(const Token_t* token, const size_t* tp)
{   
    assert(token);
    assert(tp);
    assert(token + *tp);

    Token_t tokenCopy = token[*tp];

    TokenType type = tokenCopy.type;

    RETURN_IF_FALSE(type == TokenType::Operation_t, false);

    Operation operation = tokenCopy.data.operation;

    return (operation == Operation::power);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenLeftBracket (const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    RETURN_IF_FALSE(type == TokenType::Bracket_t, false);

    Bracket bracket = token[*tp].data.bracket;

    return (bracket == Bracket::left);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenMinus(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    RETURN_IF_FALSE(type == TokenType::Operation_t, false);

    Operation operation = token->data.operation;

    return (operation == Operation::minus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenRightBracket(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[*tp].type;

    RETURN_IF_FALSE(type == TokenType::Bracket_t, false);

    Bracket bracket = token[*tp].data.bracket;

    return (bracket == Bracket::right);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationBeforeMinus(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);
    assert(IsOperationToken(token, *tp));
    assert(token[*tp].data.operation == Operation::minus);

    return (*tp >= 1 && IsOperationToken(token, *tp - 1));
    
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationToken(const Token_t* token, const size_t tp)
{
    assert(token);
    assert(tp);

    TokenType type = token[tp].type;

    return (type == TokenType::Operation_t);   
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number GetTokenNumber(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);
    assert(IsTokenNum(token, tp));

    Number number = token[*tp].data.number;
    return number;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Variable GetTokenVariable(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);
    assert(IsTokenVariable(token, tp));

    Variable variable = token[*tp].data.variable;

    return variable;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Operation GetTokenOperation(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);
    assert(IsTokenOperation(token, tp));

    Operation operation = token[*tp].data.operation;
    return operation;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Function GetTokenFunction(const Token_t* token, const size_t* tp)
{
    assert(token);
    assert(tp);
    assert(IsTokenFunction(token, tp));

    Function function = token[*tp].data.function;
    return function;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef SYNTAX_ERR
