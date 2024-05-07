#include <stdio.h>
#include <stdlib.h>
#include "tuples.h"

m_tuple* m_tuple_init(int count, ...)
{
    int size       = 0;
    int total      = 0;
    void* ptr      = NULL;
    m_list* lst    = m_list_create(int, NULL);
    m_list* buf    = NULL;
    m_tuple* tuple = (m_tuple*)malloc(sizeof(m_tuple));

    va_list valist;
    va_start(valist, count);

    for (int i = 0; i < count; ++i) {
        size  = va_arg(valist, int);
        total = total + size;
        m_list_append(lst, &size);
    }

    tuple->count   = count;
    buf            = m_list_init(sizeof(char*), NULL);
    tuple->items   = *buf;
    tuple->content = malloc(total);
    ptr            = tuple->content;

    for (int i = 0; i < count; ++i) {
        m_list_append(&(tuple->items), ptr);
        ptr = ptr + *((int*)m_list_get(lst, i));
    }

    va_end(valist);
    m_list_destroy(buf);
    m_list_destroy(lst);
    
    return tuple;
}