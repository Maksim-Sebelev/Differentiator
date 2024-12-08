#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "ReadTree.h"
#include "../Common/GlobalInclude.h"
#include "Tree.h"
#include "TreeDump.h"


//=============================== Tokens (Read Tree)  =======================================================================================================================================================================================

struct Pointers
{
    size_t ip; // input pointer
    size_t tp; // token pointer
    size_t lp; // line pointer (line in input file)
    size_t sp; // str pointer (pos in line)
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokenCtor        (Token_t* token, TokenType type, void* value, size_t fileLine, size_t linePos);

static void HandleNumber       (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleOperation    (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleLetter       (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleBracket      (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleEndSymbol    (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleVariable     (                   Token_t* tokenArr, Variable  variable, Pointers* pointer, size_t olp_sp);
static void HandleFunction     (                   Token_t* tokenArr, Function  function, Pointers* pointer, size_t old_sp);


static bool IsPassSymbol       (char c, Pointers* pointer);
static bool IsSpace            (char c);
static bool IsSlashN           (char c);


static bool IsEndSymbol        (const char* input, size_t pointer);
static bool IsNumSymbol        (const char* input, size_t pointer);
static bool IsOperationSymbol  (const char* input, size_t pointer);
static bool IsLetterSymbol     (const char* input, size_t pointer);
static bool IsBracketSymbol    (const char* input, size_t pointer);


static Number    UpdateNumber      (Number number, const char* input, Pointers* pointer);

static Number    GetNumber        (const char* input,     Pointers* pointer);
static Operation GetOperation     (const char* operation, Pointers* pointer, size_t* operationSize);
static Function  GetFunction      (const char* word, size_t wordSize);
static Variable  GetVariable      (const char* word, size_t wordSize);

//=============================== Tokens (Read Tree) End =================tt======================================================================================================================================================================
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//=============================== Syntax Err =================================================================================================================================================================================================================

static Node_t* GetNumber            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetVariable          (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetAddSub            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetMulDiv            (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetBracket           (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetPow               (const Token_t* token, size_t* tp, const char* input);

static Node_t* GetFunction          (const Token_t* token, size_t* tp, const char* input);
static Node_t* GetMinus             (const Token_t* token, size_t* tp, const char* input);


static Number    GetTokenNumber     (const Token_t* token, const size_t* tp);
static Variable  GetTokenVariable   (const Token_t* token, const size_t* tp);
static Operation GetTokenOperation  (const Token_t* token, const size_t* tp);
static Function  GetTokenFunction   (const Token_t* token, const size_t* tp);

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
static bool IsTokenMinus            (const Token_t* token, const size_t* tp);
static bool IsOperationToken        (const Token_t* token,       size_t  tp);

//=============================== Recursive Descent (Build Tree) End =======================================================================================================================================================================
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//=============================== Syntax Err ===============================================================================================================================================================================================

static const char* FindNline        (const char* str, size_t nLine, size_t* lineSize);
static void        strnprintf       (const char* str, size_t n);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define SYNTAX_ERR_FOR_TOKEN(token,               input, msg) SyntaxError((token).place.line, (token).place.placeInLine, input, msg, __FILE__, __LINE__, __func__)
#define SYNTAX_ERR(          errLine, errLinePos, input, msg) SyntaxError(errLine,            errLinePos,                input, msg, __FILE__, __LINE__, __func__)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SyntaxError(size_t errLine, size_t errLinePos, const char* input, const char* msg, const char* file, const int line, const char* func)
{
    assert(input);
    assert(msg);
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "\nSyntaxErr detected in:\n");
    PrintPlace(file, line, func);


    assert(errLine >= 1);
    assert(errLinePos >= 1);


    size_t lineSize = 0;
    const char* LineWithErr = FindNline(input, errLine, &lineSize);

    printf("\n\"");
    strnprintf(LineWithErr, lineSize);
    printf("\"\n");

    for (size_t i = 0; i < errLinePos; i++) printf(" ");

    COLOR_PRINT(RED, "^");
    COLOR_PRINT(WHITE, "\nIn line::%lu::%lu", errLine, errLinePos);
    COLOR_PRINT(WHITE, " %s\n", msg);


    COLOR_PRINT(VIOLET, "\nabort() in 3, 2, 1...\n");
    exit(0);

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* FindNline(const char* str, size_t nLine, size_t* lineSize)
{
    assert(str);

    nLine--;

    size_t lp = 0;  // line pointer
    size_t sp = 0;  // str  pointer
    size_t strLen = strlen(str);

    for (lp = 0; lp < nLine && lp < strLen; sp++)
    {
        if (str[sp] == '\n') lp++;
    }

    if (lp >= strLen)
    {
        assert(0 && "str doesn't have stolko lines\n");
    }

    size_t old_sp = sp;

    while (str[sp] != '\n' && str[sp] != '\0') sp++;

    *lineSize = sp - old_sp;

    return str + old_sp;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void strnprintf(const char* str, size_t n)
{
    assert(str);

    for (size_t i = 0; i < n; i++)
    {
        printf("%c", str[i]);
    }

    return;
}


//=============================== Syntax Err End =====================================================================================================================================================
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//=============================== Tokens (Read Tree) =======================================================================================================================================================================================

Token_t* ReadInputStr(const char* input, size_t* tokenArrSize)
{
    assert(input);
    assert(tokenArrSize);


    size_t inputLen = strlen(input);

    Token_t* tokenArr = (Token_t*) calloc(inputLen, sizeof(*tokenArr));

    Pointers pointer = {0, 0, 1, 1};

    while (pointer.ip < inputLen)
    {
        while (IsPassSymbol(input[pointer.ip], &pointer));

        if      (IsNumSymbol       (input, pointer.ip))    HandleNumber    (input, tokenArr, &pointer);
        else if (IsOperationSymbol (input, pointer.ip))    HandleOperation (input, tokenArr, &pointer);
        else if (IsLetterSymbol    (input, pointer.ip))    HandleLetter    (input, tokenArr, &pointer);
        else if (IsBracketSymbol   (input, pointer.ip))    HandleBracket   (input, tokenArr, &pointer);
        else if (IsEndSymbol       (input, pointer.ip))    HandleEndSymbol (input, tokenArr, &pointer);
        else                                               assert(0 && "undefined symbol.");
    }

    *tokenArrSize = pointer.tp;
    assert(*tokenArrSize > 0);
    assert(tokenArr);
    tokenArr = (Token_t*) realloc(tokenArr, *tokenArrSize * sizeof(Token_t));
    assert(tokenArr);

    return tokenArr;
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TokenDtor(Token_t* tokenArr)
{
    assert(tokenArr);

    FREE(tokenArr);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokenCtor(Token_t* token, TokenType type, void* value, size_t fileLine, size_t linePos)
{
    assert(token);
    assert(value);

    token->type = type;

    token->place.line        = fileLine;
    token->place.placeInLine = linePos;

    switch (type)
    {
        case TokenType::Number_t:    token->data.number    = *(Number*)    value;   break;
        case TokenType::Variable_t:  token->data.variable  = *(Variable*)  value;   break;
        case TokenType::Operation_t: token->data.operation = *(Operation*) value;   break;
        case TokenType::Function_t:  token->data.function  = *(Function*)  value;   break;
        case TokenType::Bracket_t:   token->data.bracket   = *(Bracket*)   value;   break;
        case TokenType::EndSymbol_t: token->data.end       = *(EndSymbol*) value;   break;
        default: assert(0 && "undefined token type symbol.");                       break;
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleNumber(const char* input, Token_t* tokenArr, Pointers* pointer)
{
    assert(tokenArr);
    assert(pointer);

    size_t old_sp = pointer->sp;
    Number number = GetNumber(input, pointer);
    TokenCtor(&tokenArr[pointer->tp], TokenType::Number_t, &number, pointer->lp, old_sp);
    pointer->tp++;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleOperation(const char* input, Token_t* tokenArr, Pointers* pointer)
{
    assert(tokenArr);
    assert(pointer);

    size_t operationSize = 0;
    Operation operation = GetOperation(input, pointer, &operationSize);
    TokenCtor(&tokenArr[pointer->tp], TokenType::Operation_t, &operation, pointer->lp, pointer->sp);
    pointer->tp++;
    pointer->ip++;
    pointer->sp += operationSize;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleLetter(const char* input, Token_t* tokenArr, Pointers* pointer)
{
    assert(tokenArr);
    assert(pointer);

    size_t old_ip = pointer->ip;
    do
    {
        pointer->ip++;
    }
    while (IsLetterSymbol(input, pointer->ip));

    const char* word = input + old_ip;
    const size_t wordSize = pointer->ip - old_ip;
    assert(word);

    Function function = GetFunction(word, wordSize);

    if (function != Function::undefined_function)
    {
        HandleFunction(tokenArr, function, pointer, wordSize);
        return;
    }

    Variable variable = GetVariable(word, wordSize);

    if (variable != Variable::undefined_variable)
    {
        HandleVariable(tokenArr, variable, pointer, wordSize);
        return;
    }

    assert(0 && "undefined word in input.");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleBracket(const char* input, Token_t* tokenArr, Pointers* pointer)
{
    assert(tokenArr);
    assert(pointer);
    assert(IsBracketSymbol(input, pointer->ip));

    Bracket bracket = (Bracket) input[pointer->ip];
    pointer->ip++;

    TokenCtor(&tokenArr[pointer->tp], TokenType::Bracket_t, &bracket, pointer->lp, pointer->sp);
    
    pointer->tp++;
    pointer->sp++;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleEndSymbol(const char* input, Token_t* tokenArr, Pointers* pointer)
{
    assert(tokenArr);
    assert(pointer);
    assert(IsEndSymbol(input, pointer->ip));

    EndSymbol end = (EndSymbol) input[pointer->ip];
    pointer->ip++;

    TokenCtor(&tokenArr[pointer->tp], TokenType::EndSymbol_t, &end, pointer->lp, pointer->sp);
    pointer->tp++;
    pointer->sp++;

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleVariable(Token_t* tokenArr, Variable variable, Pointers* pointer, size_t wordSize)
{
    assert(tokenArr);
    assert(pointer);

    TokenCtor(&tokenArr[pointer->tp], TokenType::Variable_t, &variable, pointer->lp, pointer->sp);

    pointer->tp++;
    pointer->sp += wordSize;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleFunction(Token_t* tokenArr, Function function, Pointers* pointer, size_t wordSize)
{
    assert(tokenArr);
    assert(pointer);

    TokenCtor(&tokenArr[pointer->tp], TokenType::Function_t, &function, pointer->lp, pointer->sp);

    pointer->tp++;
    pointer->sp += wordSize;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number GetNumber(const char* input, Pointers* pointer)
{
    assert(pointer);
    assert(IsNumSymbol(input, pointer->ip));

    Number number = 0;

    do
    {
        number = UpdateNumber(number, input, pointer);
    } 
    while (IsNumSymbol(input, pointer->ip));

    return number;    
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number UpdateNumber(Number number, const char* input, Pointers* pointer)
{
    assert(input);
    assert(pointer);

    char new_digit_char = input[pointer->ip];

    assert('0' <= new_digit_char);
    assert('9' >= new_digit_char);

    Number new_digit = new_digit_char - '0';

    assert(0 <= new_digit);
    assert(9 >= new_digit);


    if ((INT_MAX - new_digit) / 10 < number)
    {
        SYNTAX_ERR(pointer->lp, pointer->sp, input, "Integer overflow");
    }

    pointer->ip++;
    pointer->sp++;
    
    return 10 * number + new_digit;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static bool IsEndSymbol(const char* input, size_t pointer)
{
    assert(input);

    return input[pointer] == '$';
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNumSymbol(const char* input, size_t pointer)
{
    assert(input);

    return ('0' <= input[pointer]) && (input[pointer] <= '9');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '^');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLetterSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    bool flag1 = ('a' <= c && c <= 'z');
    bool flag2 = ('A' <= c && c <= 'Z');
    return (flag1 || flag2);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsBracketSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    bool flag = (c == '(') || (c == ')');
    return flag;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSpace(char c)
{
    return (c == ' ');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSlashN(char c)
{
    return (c == '\n');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPassSymbol(char c, Pointers* pointer)
{
    assert(pointer);
    RETURN_IF_TRUE(IsSpace(c),  true, pointer->sp++, pointer->ip++);
    RETURN_IF_TRUE(IsSlashN(c), true, pointer->lp++, pointer->ip++, pointer->sp = 0);
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(oper) (strncmp(operation + pointer->ip, oper, strlen(oper)) == 0)

static Operation GetOperation(const char* operation, Pointers* pointer, size_t* operationSize)
{
    assert(operation);

    for (size_t operation_i = 0; operation_i < DefaultOperationsQuant; operation_i++)
    {
        const char* name = DefaultOperations[operation_i].name;
        Operation   oper = DefaultOperations[operation_i].value;
    
        RETURN_IF_TRUE(STRNCMP(name), oper, *operationSize = 1);
    }

    return Operation::undefined_operation;
} 

#undef STRNCMP

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(function) (strncmp(word, function, wordSize) == 0)

static Function GetFunction(const char* word, size_t wordSize)
{
    assert(word);

    for (size_t function_i = 0; function_i < DefaultFunctionsQuant; function_i++)
    {
        const char* name = DefaultFunctions[function_i].name;
        Function    func = DefaultFunctions[function_i].value;
    
        RETURN_IF_TRUE(STRNCMP(name), func);
    }

    return Function::undefined_function;
}

#undef STRNCMP

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(variable) (strncmp(word, variable, wordSize) == 0)

static Variable GetVariable(const char* word, size_t wordSize)
{
    assert(word);

    RETURN_IF_TRUE(STRNCMP("x"), Variable::x);
    RETURN_IF_TRUE(STRNCMP("y"), Variable::y);

    return Variable::undefined_variable;
}

#undef STRNCMP    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//=============================== Tokens (Read Tree) End ===================================================================================================================================================================================
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//=============================== Recursive Descent (Build Tree) ==========================================================================================================================================================================================

Node_t* GetTree(const Token_t* tokens, const char* input)
{
    assert(tokens);

    size_t tp = 0;
    Node_t* node = GetAddSub(tokens, &tp, input);

    if (!IsTokenEnd(tokens, &tp))
        SYNTAX_ERR_FOR_TOKEN(tokens[tp], input, "expected '$'");

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
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected number");
    
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
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expetcted variable name");

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
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected math expression");
    
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
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected math expression");

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
            SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected ')'");

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

    if (IsTokenMinus(token, tp))
    {
        return GetMinus(token, tp, input);
    }


    if (type != TokenType::Function_t)
    {
        return GetBracket(token, tp, input);
    }

    Function function = GetTokenFunction(token, tp);

    (*tp)++;  

    if (!IsTokenLeftBracket(token, tp))
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected '('");

    (*tp)++;  

    Node_t* node = GetAddSub(token, tp, input);

    if (!IsTokenRightBracket(token, tp))
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "expected ')'");

    (*tp)++;  

    Node_t* funcNode = {};

    _FUNC(&funcNode, function, node);
    
    TREE_ASSERT(SwapNode(&node, &funcNode));

    return node;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Node_t* GetMinus(const Token_t* token, size_t* tp, const char* input)
{
    assert(token);
    assert(tp);

    if (IsOperationBeforeMinus(token, tp))
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "Operation before '-'");

    (*tp)++;  

    size_t old_tp = *tp;

    Node_t* node = GetMulDiv(token, tp, input);

    if (old_tp == *tp)
        SYNTAX_ERR_FOR_TOKEN(token[*tp], input, "Nothig after '-'");

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

    Operation operation = token[*tp].data.operation;

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

static bool IsOperationToken(const Token_t* token, size_t tp)
{
    assert(token);

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

//=============================== Recursive Descent (Build Tree) End =================================================================================================================================

// const char* ReadFile(const char* file)
// {
//     assert(file);

//     FILE* input = fopen(file, "w");

//     assert(input);

//     size_t fileSize = CalcFileSize(file);

//     char* inputStr = (char*) calloc(fileSize, sizeof(char));

//     size_t freadReturn = fread(inputStr, fileSize, sizeof(char), input);
//     assert(freadReturn == fileSize);

//     return inputStr;
// }

// static size_t CalcFileSize(const char* file)
// {
//     assert(file);

//     struct stat buf = {};
//     stat(file, &buf);
//     return (size_t) buf.st_size;
// }

#undef SYNTAX_ERR
