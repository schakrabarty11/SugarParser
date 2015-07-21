//
//  PointerStack.h
//  
//
//  Created by Sambodhi Chakrabarty on 7/20/15.
//
//

#ifndef ____PointerStack__
#define ____PointerStack__

#include <stdio.h>
#define INITIAL_CAPACITY 100


typedef struct {
    char* content;
} CharacterArray;

typedef struct{
    char** data;
    size_t size;
    size_t capacity;
} PointerStack;

void PointerStack_init(PointerStack *pStack);

void PointerStack_append(PointerStack *pStack, char* value);

char* PointerStack_get(PointerStack *pStack, int index);

void PointerStack_set(PointerStack *pStack, int index, char* value);

void PointerStack_double_capacity_if_full(PointerStack *pStack);

void PointerStack_free(PointerStack * pStack);

char* psmalloc(PointerStack *pStack, size_t size);

void psfree(PointerStack *pStack);





#endif /* defined(____PointerStack__) */
