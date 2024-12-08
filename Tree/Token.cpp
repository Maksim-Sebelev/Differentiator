#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Token.h"
#include "Tree.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------

struct Pointers
{
    size_t ip; // input pointer
    size_t tp; // token pointer
    size_t lp; // line pointer (line in input file)
    size_t sp; // str pointer (pos in line)
};

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokenCtor(Token_t* token, TokenType type, void* value, size_t fileLine, size_t linePos);

static void HandleNumber     (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleOperation  (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleLetter     (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleBracket    (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleEndSymbol  (const char* input, Token_t* tokenArr, Pointers* pointer);
static void HandleVariable   (                   Token_t* tokenArr, Variable variable, Pointers* pointer, size_t olp_sp);
static void HandleFunction   (                   Token_t* tokenArr, Function function, Pointers* pointer, size_t old_sp);


static bool IsPassSymbol       (char c, Pointers* pointer);

static bool IsEndSymbol        (const char* input, size_t pointer);
static bool IsNumSymbol        (const char* input, size_t pointer);
static bool IsOperationSymbol  (const char* input, size_t pointer);
static bool IsLetterSymbol     (const char* input, size_t pointer);
static bool IsBracketSymbol    (const char* input, size_t pointer);

static bool IsSpace            (char c);
static bool IsSlashN           (char c);

static Number    GetNumber        (const char* input,     Pointers* pointer);
static Operation GetOperation     (const char* operation, Pointers* pointer, size_t* operationSize);
static Function  GetFunction      (const char* word, size_t wordSize);
static Variable  GetVariable      (const char* word, size_t wordSize);

//------------------------------------------------------------------------------------------------------------------------------------------------------

Token_t* ReadInputStr(const char* input, size_t* tokenArrSize)
{
    assert(input);
    assert(tokenArrSize);


    size_t inputLen = strlen(input);

    Token_t* tokenArr = (Token_t*) calloc(inputLen, sizeof(*tokenArr));

    Pointers pointer = {};

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


//------------------------------------------------------------------------------------------------------------------------------------------------------

void TokenDtor(Token_t* tokenArr)
{
    assert(tokenArr);

    FREE(tokenArr);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokenCtor(Token_t* token, TokenType type, void* value, size_t fileLine, size_t linePos)
{
    assert(token);
    assert(value);

    token->type = type;

    token->place.line        = fileLine + 1;
    token->place.placeInLine = linePos  + 1;

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

//------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------------------------------------------------

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

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleVariable(Token_t* tokenArr, Variable variable, Pointers* pointer, size_t wordSize)
{
    assert(tokenArr);
    assert(pointer);

    TokenCtor(&tokenArr[pointer->tp], TokenType::Variable_t, &variable, pointer->lp, pointer->sp);

    pointer->tp++;
    pointer->sp += wordSize;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleFunction(Token_t* tokenArr, Function function, Pointers* pointer, size_t wordSize)
{
    assert(tokenArr);
    assert(pointer);

    TokenCtor(&tokenArr[pointer->tp], TokenType::Function_t, &function, pointer->lp, pointer->sp);

    pointer->tp++;
    pointer->sp += wordSize;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static Number GetNumber(const char* input, Pointers* pointer)
{
    assert(pointer);
    assert(IsNumSymbol(input, pointer->ip));

    Number number = 0;

    do
    {
        number = 10 * number + input[pointer->ip] - '0';
        pointer->ip++;
        pointer->sp++;
    } 
    while (IsNumSymbol(input, pointer->ip));

    return number;    
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsEndSymbol(const char* input, size_t pointer)
{
    assert(input);

    return input[pointer] == '$';
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNumSymbol(const char* input, size_t pointer)
{
    assert(input);

    return ('0' <= input[pointer]) && (input[pointer] <= '9');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOperationSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '^');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLetterSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    bool flag1 = ('a' <= c && c <= 'z');
    bool flag2 = ('A' <= c && c <= 'Z');
    return (flag1 || flag2);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsBracketSymbol(const char* input, size_t pointer)
{
    assert(input);

    char c = input[pointer];
    bool flag = (c == '(') || (c == ')');
    return flag;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSpace(char c)
{
    return (c == ' ');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSlashN(char c)
{
    return (c == '\n');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPassSymbol(char c, Pointers* pointer)
{
    assert(pointer);
    RETURN_IF_TRUE(IsSpace(c),  true, pointer->sp++, pointer->ip++);
    RETURN_IF_TRUE(IsSlashN(c), true, pointer->lp++, pointer->ip++, pointer->sp = 0);
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------


#define STRCMP(oper) (strncmp(operation + pointer->ip, oper, strlen(oper)) == 0)

static Operation GetOperation(const char* operation, Pointers* pointer, size_t* operationSize)
{
    assert(operation);

    RETURN_IF_TRUE(STRCMP("+"), Operation::plus , *operationSize = 1);
    RETURN_IF_TRUE(STRCMP("-"), Operation::minus, *operationSize = 1);
    RETURN_IF_TRUE(STRCMP("*"), Operation::mul  , *operationSize = 1);
    RETURN_IF_TRUE(STRCMP("/"), Operation::dive , *operationSize = 1);
    RETURN_IF_TRUE(STRCMP("^"), Operation::power, *operationSize = 1);

    return Operation::undefined_operation;
} 

#undef STRCMP

//--------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(function) (strncmp(word, function, wordSize) == 0)

static Function GetFunction(const char* word, size_t wordSize)
{
    assert(word);

    RETURN_IF_TRUE(STRNCMP("ln")    , Function::ln    );
    RETURN_IF_TRUE(STRNCMP("sin")   , Function::sin   );
    RETURN_IF_TRUE(STRNCMP("cos")   , Function::cos   );
    RETURN_IF_TRUE(STRNCMP("tg")    , Function::tg    );
    RETURN_IF_TRUE(STRNCMP("ctg")   , Function::ctg   );
    RETURN_IF_TRUE(STRNCMP("sh")    , Function::sh    );
    RETURN_IF_TRUE(STRNCMP("ch")    , Function::ch    );
    RETURN_IF_TRUE(STRNCMP("th")    , Function::th    );
    RETURN_IF_TRUE(STRNCMP("cth")   , Function::cth   );
    RETURN_IF_TRUE(STRNCMP("arcsin"), Function::arcsin);
    RETURN_IF_TRUE(STRNCMP("arccos"), Function::arccos);
    RETURN_IF_TRUE(STRNCMP("arctg") , Function::arctg );
    RETURN_IF_TRUE(STRNCMP("arcctg"), Function::arcctg);

    return Function::undefined_function;
}

#undef STRNCMP

//--------------------------------------------------------------------------------------------------------------------------------------

#define STRNCMP(variable) (strncmp(word, variable, wordSize) == 0)

static Variable GetVariable(const char* word, size_t wordSize)
{
    assert(word);

    RETURN_IF_TRUE(STRNCMP("x"), Variable::x);
    RETURN_IF_TRUE(STRNCMP("y"), Variable::y);

    return Variable::undefined_variable;
}

#undef STRNCMP    

//--------------------------------------------------------------------------------------------------------------------------------------
