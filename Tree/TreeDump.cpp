#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "TreeDump.h"
#include "Tree.h"


static void DotBegin              (FILE* dotFile);
static void DotEnd                (FILE* dotFile);
// static void DotBeginSubGraph1     (FILE* dotFile);
// static void DotBeginSubGraph2     (FILE* dotFile);
static void DotCreateAllNodes     (FILE* dotFile, const Node_t* node);
static void DotCreateEdges        (FILE* dotFile, const Node_t* node);
static void DotCreateEdgesHelper  (FILE* dotFile, const Node_t* node);
static void DotCreateDumpPlace    (FILE* dotFile, const char* file, const int line, const char* func);
static void GraphicDumpHelper     (const Node_t* node, const char* dotFileName, const char* file, const int line, const char* func);

static const char* GetTypeColor       (const Node_t* node);
static const char* GetNodeTypeInStr   (const Node_t* node);
static const char* GetNodeArgInStr    (const Node_t* node);
static const char* GetVariableInStr   (Variable var);
static const char* GetOperationInStr  (Operation oper);
static const char* GetFuncInStr       (Function func);


//-------------------------------------------------------------------------------------------------------------------------------------

void GraphicDump(const Node_t* node, const char* file, const int line, const char* func)
{
    static size_t ImgQuant = 1;

    static const size_t MaxfileNameLen = 128;
    char outfile[MaxfileNameLen] = {};
    sprintf(outfile, "out%lu.png", ImgQuant);
    ImgQuant++;

    static const size_t MaxCommandLen = 256;
    char command[MaxCommandLen] = {};
    static const char* dotFileName = "tree.dot";
    sprintf(command, "dot -Tpng %s > %s", dotFileName, outfile);

    GraphicDumpHelper(node, dotFileName, file, line, func);
    system(command);

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------

static void GraphicDumpHelper(const Node_t* node, const char* dotFileName, const char* file, const int line, const char* func)
{
    assert(node);

    FILE* dotFile = fopen(dotFileName, "w");
    assert(dotFile);

    DotBegin(dotFile);

    // DotBeginSubGraph1(dotFile);

    DotCreateAllNodes(dotFile, node);
    DotCreateEdges(dotFile, node);

    // DotEnd(dotFile);

    // DotBeginSubGraph2(dotFile);
    DotCreateDumpPlace(dotFile, file, line, func);
    // DotEnd(dotFile);


    DotEnd(dotFile);

    fclose(dotFile);
    dotFile = nullptr;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void DotBegin(FILE* dotFile)
{
    assert(dotFile);
    // fprintf(dotFile, "digraph G{\ngraph [bgcolor=\"#000000\"];\n");
    fprintf(dotFile, "digraph G{\nrankdir=TB\ngraph [bgcolor=\"#000000\"];\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------

// static void DotBeginSubGraph1(FILE* dotFile)
// {
//     assert(dotFile);
//     fprintf(dotFile, "subgraph sg1{\nrankdir=TB;\ngraph [];\n");
//     return;
// }

// //----------------------------------------------------------------------------------------------------------------------

// static void DotBeginSubGraph2(FILE* dotFile)
// {
//     assert(dotFile);
//     fprintf(dotFile, "subgraph sg2{\nrankdir=LR;\ngraph [];\n");
//     return;
// }

// //----------------------------------------------------------------------------------------------------------------------

static void DotEnd(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "}\n");
    return;
}

//----------------------------------------------------------------------------------------------------------------------

static void DotCreateAllNodes(FILE* dotFile, const Node_t* node)
{
    assert(dotFile);
    assert(node);


    const char* nodeColor = GetTypeColor(node);
    fprintf(dotFile, "node%p", node);
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"%s\"", nodeColor);

    NodeArgType type = node->data.type;

    if (type == NodeArgType::number)
    {
        Number num = node->data.num;
        fprintf(dotFile, "label =\"%d\", ", num);
    }
    else
    {
        const char* arg = GetNodeArgInStr(node);
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

//-------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateEdges(FILE* dotFile, const Node_t* node)
{
    assert(dotFile);
    assert(node);

    fprintf(dotFile, "edge[color=\"#5b5b5b\"];\n");
    DotCreateEdgesHelper(dotFile, node);

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------

static void DotCreateDumpPlace(FILE* dotFile, const char* file, const int line, const char* func)
{
    assert(dotFile);

    fprintf(dotFile, "place");
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"#1771a0\",");
    fprintf(dotFile, "label  = \"Dump place:");
    fprintf(dotFile, "| file: %s ",     file);
    fprintf(dotFile, "|<f0> line: %d ",  line);
    fprintf(dotFile, "|<f1> func: %s\", ", func);
    fprintf(dotFile, "color = \"#000000\"];\n");

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTypeColor(const Node_t* node)
{
    NodeArgType type = node->data.type;

    switch (type)
    {
        case NodeArgType::number:
        {
            return "#1692bb";
            break;
        }

        case NodeArgType::operation:
        {
            return "#168237";
            break;
        }

        case NodeArgType::variable:
        {
            return "#9b2017";
            break;
        }

        case NodeArgType::function:
        {
            return "#168237";
            break;
        }

        case NodeArgType::undefined:
        {
            return "red";
            break;
        }

        default:
        {
            assert(0 && "undefined situation in GetColorType.\n");
            break;
        }
    }

    assert(0 && "undefined situation in GetColorType.\n");

    return "red";    
}

//-------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNodeTypeInStr(const Node_t* node)
{
    assert(node);

    NodeArgType type = node->data.type;

    switch (type)
    {
        case NodeArgType::number:
            return "number";
            break;

        case NodeArgType::operation:
            return "operation";
            break;

        case NodeArgType::function:
            return "function";
            break;

        case NodeArgType::variable:
            return "variable";
            break;

        case NodeArgType::undefined:
            return "undefined";
            break;

        default:
            assert(0 && "You forgot about some node type in text dump.\n");
            return "undefined";
    }
    assert(0 && "We must not be here.\n");
    return "wtf?";
}

//-------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNodeArgInStr(const Node_t* node)
{
    assert(node);
    NodeArgType type = node->data.type;

    switch (type)
    {
        case NodeArgType::number:
        {
            assert(0 && "Number is drugaya situation.\n");
            return "undefined";
            break;
        }

        case NodeArgType::operation:
        {
            Operation oper = node->data.oper;
            return GetOperationInStr(oper);
            break;
        }

        case NodeArgType::function:
        {
            Function func = node->data.func;
            return GetFuncInStr(func);
            break;

        }

        case NodeArgType::variable:
        {
            Variable var = node->data.var;
            return GetVariableInStr(var);
            break;
        }

        case NodeArgType::undefined:
        {
            return "undefined node type.";
            break;
        }

        default:
        {
            assert(0 && "you forgot about some node type.\n");
            return nullptr;
            break;
        }
    }

    assert(0 && "you forgot about some node type.\n");
    return nullptr;
}

//-------------------------------------------------------------------------------------------------------------------------------------

static const char* GetVariableInStr(Variable var)
{
    switch (var)
    {
        case Variable::x:
            return "x";

        case Variable::y:
            return "y";

        case Variable::undefined_variable:
            return "undefined";

        default:
            assert(0 && "You forgot about some variable name in graphic dump.\n");
            return "undefined";
    }

    assert(0 && "we must not be here.\n");
    return "wtf?";
}

//-------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------

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
        case Function::undefined_function:                                          return "undefined";
        default: assert(0 && "You forgot abourt some function in graphic dump.\n"); return "undefined";
    }

    assert(0 && "We must not be here.\n");
    return "wtf?";
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PrintAllNodeInfo(const Node_t* node, const char* file, const int line, const char* func)
{
    assert(file);
    assert(func);

    COLOR_PRINT(YELLOW, "\nplace:\n");
    COLOR_PRINT(YELLOW, "file [%s]\n",   file);
    COLOR_PRINT(YELLOW, "line [%d]\n",   line);
    COLOR_PRINT(YELLOW, "func [%s]\n\n", func);

    COLOR_PRINT(GREEN, "node:\n");

    if (!node)
    {
        COLOR_PRINT(RED, "'nullptr'\n\n");
        return;
    }

    COLOR_PRINT(CYAN,  "type  = '%s'\n",   GetNodeTypeInStr  (node));

    if (node->data.type == NodeArgType::number)
        COLOR_PRINT(CYAN, "arg   = '%d'\n\n", node->data.num);
    else
        COLOR_PRINT(CYAN, "arg   = '%s'\n\n", GetNodeArgInStr(node));

    COLOR_PRINT(CYAN,  "num   = '%d'\n",   node->data.num);
    COLOR_PRINT(CYAN,  "oper  = '%s'\n",   GetOperationInStr (node->data.oper));
    COLOR_PRINT(CYAN,  "func  = '%s'\n",   GetFuncInStr      (node->data.func));
    COLOR_PRINT(CYAN,  "var   = '%s'\n\n", GetVariableInStr  (node->data.var));

    COLOR_PRINT(VIOLET, "left  = %p\n", node->left);
    COLOR_PRINT(VIOLET, "right = %p\n", node->right);

    printf("\n\n");
    return;
}