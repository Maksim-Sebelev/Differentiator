#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "TreeDump.h"
#include "Tree.h"
#include "ReadTree.h"
#include "../Common/GlobalInclude.h"


static void TokenGraphicDumpHelper(const Token_t* tokenArr, size_t arrSize, const char* dotFileName, const char* file, const int line, const char* func);

static void DotTokenBegin    (FILE* dotFile);
static void CreateAllTokens  (const Token_t* tokenArr, size_t arrSize, FILE* dotFile);
static void CreateToken      (const Token_t* token,    size_t pointer, FILE* dotFile);

static const char* GetTokenColor     (const Token_t* token);
static const char* GetTokenTypeInStr (const Token_t* token);
static const char* GetTokenDataInStr (const Token_t* token);


static void DotNodeBegin          (FILE* dotFile);
static void DotEnd                (FILE* dotFile);
static void DotCreateAllNodes     (FILE* dotFile, const Node_t* node);
static void DotCreateEdges        (FILE* dotFile, const Node_t* node);
static void DotCreateEdgesHelper  (FILE* dotFile, const Node_t* node);
static void DotCreateDumpPlace    (FILE* dotFile, const char* file, const int line, const char* func);
static void TreeDumpHelper     (const Node_t* node, const char* dotFileName, const char* file, const int line, const char* func);

static const char* GetNodeColor       (const Node_t* node);
static const char* GetNodeTypeInStr   (const Node_t* node);
static const char* GetNodeDataInStr    (const Node_t* node);
static const char* GetVariableInStr   (Variable var);
static const char* GetOperationInStr  (Operation oper);
static const char* GetFuncInStr       (Function func);


//=============================== Token Dump =============================================================================================================================================

void TokenTextDump(const Token_t* tokenArr, size_t tokenNum, const char* file, const int line, const char* func)
{
    assert(tokenArr);
    assert(file);
    assert(file);

    COLOR_PRINT(GREEN, "\nToken Dump:\n\n");

    PRINT_PLACE(BLUE, file, line, func);

    COLOR_PRINT(GREEN, "\n\ntoken[%lu]:\n", tokenNum);

    COLOR_PRINT(CYAN, "type: '%s'\n", GetTokenTypeInStr(&tokenArr[tokenNum]));
    COLOR_PRINT(CYAN, "data: '%s'\n", GetTokenDataInStr(&tokenArr[tokenNum]));

    COLOR_PRINT(GREEN, "\nToken Dump End.\n\n\n");

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TokenGraphicDump(const Token_t* tokenArr, size_t arrSize, const char* file, const int line, const char* func)
{
    assert(tokenArr);
    assert(file);
    assert(func);

    static size_t ImgQuant = 1;

    static const size_t MaxfileNameLen = 128;
    char outfile[MaxfileNameLen] = {};
    sprintf(outfile, "token%lu.png", ImgQuant);
    ImgQuant++;

    static const size_t MaxCommandLen = 256;
    char command[MaxCommandLen] = {};
    static const char* dotFileName = "token.dot";
    sprintf(command, "dot -Tpng %s > %s", dotFileName, outfile);
    
    TokenGraphicDumpHelper(tokenArr, arrSize, dotFileName, file, line, func);
    system(command);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokenGraphicDumpHelper(const Token_t* tokenArr, size_t arrSize, const char* dotFileName, const char* file, const int line, const char* func)
{
    assert(tokenArr);
    assert(file);
    assert(func);

    FILE* dotFile = fopen(dotFileName, "w");
    assert(dotFile);

    DotTokenBegin(dotFile);

    DotCreateDumpPlace(dotFile, file, line, func);

    CreateAllTokens(tokenArr, arrSize, dotFile);
    DotEnd(dotFile);

    fclose(dotFile);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotTokenBegin(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "digraph G{\nrankdir=TB\ngraph [bgcolor=\"#000000\"];\nsize = \"100 5\"\n");
    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CreateAllTokens(const Token_t* tokenArr, size_t arrSize, FILE* dotFile)
{
    assert(tokenArr);
    assert(dotFile);

    for (size_t i = 0; i < arrSize; i++)
    {
        CreateToken(&tokenArr[i], i, dotFile);
    }

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CreateToken(const Token_t* token, size_t pointer, FILE* dotFile)
{
    assert(dotFile);

    if (!token)
    {
        fprintf(dotFile, "token%lu", pointer);
        fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"#0125fe\", ");
        fprintf(dotFile, "label = \"nullptr\"");
        fprintf(dotFile, "color = \"#777777\"];\n");
        return;
    }

    const char* tokenColor = GetTokenColor(token);

    assert(tokenColor);

    TokenType type = token->type;

    fprintf(dotFile, "token%lu", pointer);
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"%s\",", tokenColor);

    const char* tokenType = GetTokenTypeInStr(token);
    assert(tokenType);

    fprintf(dotFile, "label = \" { %s | ", tokenType);

    if (type == TokenType::Number_t)
    {
        fprintf(dotFile, "%d | ", token->data.number);
    }

    else
    {
        const char* tokenData  = GetTokenDataInStr(token);
        assert(tokenData);
        fprintf(dotFile, "%s | ", tokenData);
    }

    fprintf(dotFile, " token[%lu] | ", pointer);
    fprintf(dotFile, " input::%lu::%lu } \", ", token->place.line, token->place.placeInLine);
    fprintf(dotFile, "color = \"#777777\"];\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTokenColor(const Token_t* token)
{
    assert(token);

    TokenType type = token->type;

    switch (type)
    {
        case TokenType::Number_t:      return "#1cb9ff";
        case TokenType::Variable_t:    return "#f31807";
        case TokenType::Operation_t:   return "#00ca2c";
        case TokenType::Function_t:    return "#0cf108";
        case TokenType::Bracket_t:     return "#e69c0c";
        case TokenType::EndSymbol_t:   return "#ffffff";
        default: assert(0 && "undefined token type."); break; 
    }

    assert(0 && "we must not be here");
    return "wtf";
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTokenTypeInStr(const Token_t* token)
{
    assert(token);

    TokenType type = token->type;

    switch (type)
    {
        case TokenType::Number_t:    return "number";
        case TokenType::Variable_t:  return "variable";
        case TokenType::Operation_t: return "operation";
        case TokenType::Function_t:  return "function";
        case TokenType::Bracket_t:   return "bracket";
        case TokenType::EndSymbol_t: return "end";
        default: assert(0 && "nudefindef type."); return "undefined";
    }

    return "undefined";
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTokenDataInStr(const Token_t* token)
{
    assert(token);

    TokenType type = token->type;

    switch (type)
    {
        case TokenType::Number_t:
        {    
            assert(0 && "Number is drugaja situation.");
            return "undefined";
        }

        case TokenType::Variable_t:
        {
            Variable variable = token->data.variable;
            switch (variable)
            {
                case Variable::x: return "x";
                case Variable::y: return "y";
                case Variable::undefined_variable:
                default: assert(0 && "undefined variable."); break;
            }
            break;
        }
    
        case TokenType::Operation_t:
        {
            Operation operation = token->data.operation;
            return GetOperationInStr(operation); //
        }
        case TokenType::Function_t:
        {
            Function function = token->data.function;
            return GetFuncInStr(function); //
        }
         
        case TokenType::Bracket_t:
        {
            Bracket bracket = token->data.bracket;
            switch (bracket)
            {
                case Bracket::left: return "(";
                case Bracket::right: return ")";
                default: assert(0 && "undefined bracket type."); break;
            }
            break;
        }
    
        case TokenType::EndSymbol_t:
        {
            EndSymbol end = token->data.end;
            switch (end)
            {
                case EndSymbol::end: return "$";
                default: assert(0 && "undefined end syn=mbol.");
            }
            break;
        }
        default: assert(0 && "undefined type."); return "undefined";
    }

    return "undefined";
}

//=============================== Token Dump End =============================================================================================================================================


//=============================== Tree Dump ==================================================================================================================================================

void NodeTextDump(const Node_t* node, const char* file, const int line, const char* func)
{
    assert(file);
    assert(func);

    COLOR_PRINT(GREEN, "\nNode Dump:\n\n");

    PRINT_PLACE(YELLOW, file, line, func);
    
    COLOR_PRINT(GREEN, "\nnode:\n");

    if (!node)
    {
        COLOR_PRINT(RED, "'nullptr'\n\n");
        return;
    }

    COLOR_PRINT(CYAN,  "type = '%s'\n", GetNodeTypeInStr  (node));

    NodeArgType type = node->type;

    if (type == NodeArgType::number)
    {
        COLOR_PRINT(CYAN, "data = '%d'\n", node->data.num);
    }

    else
    {
        COLOR_PRINT(CYAN,  "data = '%s'\n", GetNodeDataInStr(node));
    }

    COLOR_PRINT(VIOLET, "left  = %p\n", node->left);
    COLOR_PRINT(VIOLET, "right = %p\n", node->right);

    COLOR_PRINT(GREEN, "\nNode Dump End.\n\n");

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TreeDump(const Node_t* node, const char* file, const int line, const char* func)
{
    assert(node);
    assert(file);
    assert(func);

    static size_t ImgQuant = 1;

    static const size_t MaxfileNameLen = 128;
    char outfile[MaxfileNameLen] = {};
    sprintf(outfile, "tree%lu.png", ImgQuant);
    ImgQuant++;

    static const size_t MaxCommandLen = 256;
    char command[MaxCommandLen] = {};
    static const char* dotFileName = "tree.dot";
    sprintf(command, "dot -Tpng %s > %s", dotFileName, outfile);

    TreeDumpHelper(node, dotFileName, file, line, func);
    system(command);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void TreeDumpHelper(const Node_t* node, const char* dotFileName, const char* file, const int line, const char* func)
{
    assert(node);
    assert(dotFileName);
    assert(file);
    assert(func);
    assert(true || line);

    FILE* dotFile = fopen(dotFileName, "w");
    assert(dotFile);

    DotNodeBegin(dotFile);

    DotCreateDumpPlace(dotFile, file, line, func);

    DotCreateAllNodes(dotFile, node);
    DotCreateEdges(dotFile, node);

    DotEnd(dotFile);

    fclose(dotFile);
    dotFile = nullptr;

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotNodeBegin(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "digraph G{\nrankdir=TB\ngraph [bgcolor=\"#000000\"];\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateAllNodes(FILE* dotFile, const Node_t* node)
{
    assert(dotFile);
    assert(node);


    const char* nodeColor = GetNodeColor(node);
    fprintf(dotFile, "node%p", node);
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"%s\"", nodeColor);

    NodeArgType type = node->type;

    if (type == NodeArgType::number)
    {
        Number num = node->data.num;
        fprintf(dotFile, "label =\"%d\", ", num);
    }
    else
    {
        const char* arg = GetNodeDataInStr(node);
        fprintf(dotFile, "label =\"%s\", ", arg);
    }

    fprintf(dotFile, "color = \"#777777\"];\n");

    if (node->left)
    {
        DotCreateAllNodes(dotFile, node->left);
    }

    if (node->right)
    {
        DotCreateAllNodes(dotFile, node->right);
    }

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateEdges(FILE* dotFile, const Node_t* node)
{
    assert(dotFile);
    assert(node);

    fprintf(dotFile, "edge[color=\"#373737\"];\n");
    DotCreateEdgesHelper(dotFile, node);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateEdgesHelper(FILE* dotFile, const Node_t* node)
{
    assert(dotFile);
    assert(node);

    if (node->left)
    {
        fprintf(dotFile, "node%p->node%p;\n", node, node->left);

        DotCreateEdgesHelper(dotFile, node->left);
    }

    if (node->right)
    {
        fprintf(dotFile, "node%p->node%p;\n", node, node->right);

        DotCreateEdgesHelper(dotFile, node->right);
    }

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static const char* GetNodeColor(const Node_t* node)
{
    assert(node);

    NodeArgType type = node->type;

    switch (type)
    {
        case NodeArgType::number:    return "#1662b7";
        case NodeArgType::operation: return "#177d20";
        case NodeArgType::variable:  return "#832316";
        case NodeArgType::function:  return "#218617";
        case NodeArgType::undefined: return "red";
        default:
            assert(0 && "undefined situation in GetColorType.\n");
            break;
    }

    assert(0 && "undefined situation in GetColorType.\n");

    return "red";    
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNodeTypeInStr(const Node_t* node)
{
    assert(node);

    NodeArgType type = node->type;

    switch (type)
    {
        case NodeArgType::number:
            return "number";

        case NodeArgType::operation:
            return "operation";

        case NodeArgType::function:
            return "function";

        case NodeArgType::variable:
            return "variable";

        case NodeArgType::undefined:
            return "undefined";

        default:
            assert(0 && "You forgot about some node type in text dump.\n");
            return "undefined";
    }
    assert(0 && "We must not be here.\n");
    return "wtf?";
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNodeDataInStr(const Node_t* node)
{
    assert(node);

    NodeArgType type = node->type;

    switch (type)
    {
        case NodeArgType::number:
        {        
            assert(0 && "Number is drugaya situation.\n");
            return "undefined";
        }

        case NodeArgType::operation:
        {
            Operation oper = node->data.oper;
            return GetOperationInStr(oper);
        }

        case NodeArgType::function:
        {
            Function func = node->data.func;
            return GetFuncInStr(func);
        }

        case NodeArgType::variable:
        {
            Variable var = node->data.var;
            return GetVariableInStr(var);
        }

        case NodeArgType::undefined:
            return "undefined node type.";

        default:
            assert(0 && "you forgot about some node type.");
            return "undefined";
    }

    assert(0 && "you forgot about some node type.\n");
    return "undefined";
}

//=============================== Tree Dump End =============================================================================================================================================


//=============================== GLobal Graphic Dump Function ==============================================================================================================================

static void DotEnd(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "}\n");
    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateDumpPlace(FILE* dotFile, const char* file, const int line, const char* func)
{
    assert(dotFile);
    assert(file);
    assert(func);

    fprintf(dotFile, "place");
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"#1771a0\", pos=\"0,1!\",");
    fprintf(dotFile, "label = \" { Dump place: | file: [%s] | line: [%d] | func: [%s] | autor: Maksimka | I'm not gay } \"", file, line, func);
    fprintf(dotFile, "color = \"#000000\"];\n");

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetVariableInStr(Variable var)
{
    switch (var)
    {
        case Variable::x: return "x";
        case Variable::y: return "y";
        case Variable::undefined_variable: return "undefined";
        default:
            assert(0 && "You forgot about some variable name in graphic dump.\n");
            return "undefined";
    }

    assert(0 && "we must not be here.\n");
    return "wtf?";
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetOperationInStr(Operation oper)
{
    switch (oper)
    {
        case Operation::plus:    return "+";
        case Operation::minus:   return "-";
        case Operation::mul:     return "*";
        case Operation::dive:    return "/";
        case Operation::power:   return "^";
        case Operation::undefined_operation:                                        return "undefined";
        default: assert(0 && "You forgot about some opeartion in graphic dump.\n"); return "undefined";
    }

    assert(0 && "We must not be here.\n");
    return "wtf?";
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetFuncInStr(Function func)
{
    switch (func)
    {
        case Function::ln:       return "ln";
        case Function::sin:      return "sin";
        case Function::cos:      return "cos";
        case Function::tg:       return "tg";
        case Function::ctg:      return "ctg";
        case Function::sh:       return "sh";
        case Function::ch:       return "ch";
        case Function::th:       return "th";
        case Function::cth:      return "cth";
        case Function::arcsin:   return "arcsin";
        case Function::arccos:   return "arccos";
        case Function::arctg:    return "arctg";
        case Function::arcctg:   return "arccrg";
        case Function::undefined_function: return "undefined";
        default: assert(0 && "You forgot abourt some function in graphic dump.\n"); return "undefined";
    }

    assert(0 && "We must not be here.\n");
    return "wtf?";
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
