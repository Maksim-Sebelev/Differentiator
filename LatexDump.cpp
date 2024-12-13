#include <stdio.h>
#include <assert.h>

#include "LatexDump.h"



static void LatexBegin (FILE* texFile);
static void LatexEnd   (FILE* texFile);
static void LatexMain  (FILE* texFile);


void LatexDump()
{
    const char* fileName = "diff.tex";
    FILE* texFile = fopen(fileName, "wb");

    LatexBegin(texFile);

    LatexMain(texFile);

    LatexEnd(texFile);
}

static void LatexBegin(FILE* texFile)
{
    assert(texFile);

    fprintf(texFile, "\\documentclass{article}\n\n");

    fprintf(texFile, "\\begin{document}\n");


    return;
}
 
static void LatexMain(FILE* texFile)
{
    assert(texFile);

    fprintf(texFile, "пизда круто нвхуй\n");

    return;
}

static void LatexEnd(FILE* texFile)
{
    assert(texFile);

    fprintf(texFile, "\\end{document}\n");

    return;
}
