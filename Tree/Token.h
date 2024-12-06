#ifndef TOKEN_H
#define TOKEN_H


#include "Tree.h"


struct FilePlace
{
    size_t line;
    size_t placeInLine;
};


enum Bracket
{
    left = '(',
    right = ')'
};


enum EndSymbol
{
    end = '$'
};


union TokenData
{
    Number    number;
    Variable  variable;
    Operation operation;
    Function  function;
    Bracket   bracket;
    EndSymbol end;
};


enum TokenType
{
    Number_t = 1,
    Variable_t,
    Operation_t,
    Function_t,
    Bracket_t,
    EndSymbol_t
};


struct Token_t
{
    TokenType type;
    TokenData data;
    FilePlace place;
};



Token_t* ReadInputStr   (const char* input, size_t* tokenArrSize);
void     TokenDtor      (Token_t* tokenArr);


#endif
