#ifndef STACK_H
#define STACK_H
#pragma once

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct
m_stack
{
    int    count;
    size_t size;
    void*  content;
    int    capacity;
}
m_stack;

void* m_stack_pop(m_stack* m);
m_stack _m_stack_init(size_t size);
int m_stack_push(m_stack* m, void* item);
void m_stack_delete(m_stack* m);
#define m_stack_init(t) (_m_stack_init(sizeof(t)))
#define m_stack_tos(stack) ((stack).content + ((stack).size * ((stack).count - 1)))

#endif