#ifndef LISTS_H
#define LISTS_H
#pragma once

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct m_list {
    int count;                                                          // Current count of list objects
    int capacity;                                                       // Current list capacity
    void* array;                                                        // Container of list
    size_t size;                                                        // Size of element type
} m_list;

void m_list_destroy(m_list* m);
m_list* m_list_copy(m_list* src);
int m_list_pop(m_list *m);
int m_list_append(m_list *m, void* i);
int m_list_clear(m_list *m);
void* m_list_get(m_list *m, int index);
int m_list_remove(m_list* m, int index);
void* m_list_to_array(m_list* m);
void* m_list_back(m_list* m);
int m_list_replace(m_list* m, int index, void* i);
int m_list_extend(m_list* tar, m_list* src);
m_list* m_list_init(size_t size);

#define m_list_create(type) (m_list_init(sizeof(type)))

#endif