#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "DiffDump.h"
#include "../Tree/Tree.h"

static const char* GetTypeColor   (const Node_t* node);

static void DotBegin              (FILE* dotFile);
static void DotEnd                (FILE* dotFile);
static void DotBeginSubGraph1     (FILE* dotFile);
static void DotBeginSubGraph2     (FILE* dotFile);

static void DotCreateAllNodes     (FILE* dotFile, const Node_t* node);
static void DotCreateEdges        (FILE* dotFile, const Node_t* node);
static void DotCreateEdgesHelper  (FILE* dotFile, const Node_t* node);
static void DotCreateDumpPlace    (FILE* dotFile, const char* file, const int line, const char* func);

static void GraphicDumpHelper     (const Node_t* node, const char* dotFileName, const char* file, const int line, const char* func);


void GraphicDump(const Node_t* node, const char* file, const int line, const char* func)
{
    static size_t ImgQuant = 0;

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

static void DotBeginSubGraph1(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "subgraph sg1{\nrankdir=TB;\ngraph [];\n");
    return;
}

//----------------------------------------------------------------------------------------------------------------------

static void DotBeginSubGraph2(FILE* dotFile)
{
    assert(dotFile);
    fprintf(dotFile, "subgraph sg2{\nrankdir=LR;\ngraph [];\n");
    return;
}

//----------------------------------------------------------------------------------------------------------------------

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

    const char* arg       = node->data.arg;
    const char* nodeColor = GetTypeColor(node);

    fprintf(dotFile, "node%p", node);
    fprintf(dotFile, "[shape=Mrecord, style=filled, fillcolor=\"%s\"", nodeColor);
    fprintf(dotFile, "label =\"%s\", ", arg);
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
