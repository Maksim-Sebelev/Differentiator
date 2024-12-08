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

#define PRINT_PLACE(color, file, line, func) printf(color); PrintPlace(file, line, func); printf(RESET)


#define RETURN_IF_FALSE(flag, returnElem, ...) \
if (!(flag))                                    \
{                                                \
    __VA_ARGS__;                                  \
    return returnElem;                             \
}                                                   \

#define RETURN_IF_TRUE(flag, returnElem, ...) \
if ((flag))                                    \
{                                                \
    __VA_ARGS__;                                  \
    return returnElem;                             \
}     

#endif
