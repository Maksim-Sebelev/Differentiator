#ifndef ONEGIN_H
#define ONEGIN_H

#include <stdio.h>
#include <malloc.h>

// функция ReadBufferFromFile возвращает указатель на массив строк,
// который строится внутри нее. Значение агрумента bufSize после исполнения
// функции ReadBufferFromFile будет равным количеству строк в массиве,
// на который она возвращает указатель.

// strtoi преобразовывает строку в число,
// в случае если не вся строка является числом, программа будет завершена аварийно.

// если флаги компиляции позволяют не освобождать память 
// перед завершением программы, функция BufferDtor может не использоваться
// в протвном случае - BufferDtor должна быть использована.

const char** ReadBufferFromFile (const char* const file, size_t* bufSize);
int    strtoi             (const char* const str);
void   BufferDtor         (const char** buffer);

#define FREE(ptr) free(ptr); ptr = NULL;

#endif
