#ifndef GLOBAL_INCLUDE_H
#define GLOBAL_INCLUDE_H

#include <stdio.h>

struct CodePlace
{
    const char* file;
    int         line;
    const char* func;
};

void PrintPlace     (                  const char* const file, const int line, const char* const func);
void CodePlaceCtor  (CodePlace* place, const char* const file, const int line, const char* const func);

#define FREE(ptr) free(ptr); ptr = NULL;

#endif
