//
//  PointerStack.c
//  
//
//  Created by Sambodhi Chakrabarty on 7/20/15.
//
//


#include "PointerStack.h"
#include <stdio.h>
#include <stdlib.h>

void PointerStack_init(PointerStack *pStack){
    //initialize size and capacity
    pStack->size = 0;
    pStack->capacity = INITIAL_CAPACITY;
    
    //Allocate memory for the data
    pStack->data = malloc(sizeof(char*)*pStack->capacity);
}

void PointerStack_append(PointerStack *pStack, char* value){
    //make sure there's room to expand
    PointerStack_double_capacity_if_full(pStack);
    
    // append the value and increment pStack->size
    pStack->data[pStack->size++] = value;
}

char* PointerStack_get(PointerStack *pStack, int index){
    if(index>= pStack->size || index < 0){
        exit(1);
    }
    return pStack->data[index];
}

void PointerStack_set(PointerStack *pStack, int index, char* value){
    pStack->data[index] = value;
}

void PointerStack_double_capacity_if_full(PointerStack *pStack){
    if(pStack->size >= pStack->capacity){
        //double pStack->capacity and resize the allocated memory accordingly
        pStack->capacity *=2;
        pStack->data = realloc(pStack->data, sizeof(char*)*pStack->capacity);
    }
}

void PointerStack_free(PointerStack *pStack){
    free(pStack->data);
}


char* psmalloc(PointerStack *pStack, size_t size)
{
    char* ptr = emalloc(size);
    PointerStack_append(pStack, ptr);
    
    return ptr;
}

void psfree(PointerStack *pStack)
{
    for (int i = 0; i<pStack->size; i++){
        efree(pStack->data[i]);
    }
    
    pStack->size = 0;
}