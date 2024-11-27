#ifndef ONEGIN_H
#define ONEGIN_H

#include <stdio.h>
#include <malloc.h>

char** ReadBufferFromFile (const char* const file, size_t* bufSize);
int    strtoi             (const char* const str);
void   BufferDtor         (char** buffer);

#define FREE(ptr) free(ptr); ptr = nullptr;

#endif
