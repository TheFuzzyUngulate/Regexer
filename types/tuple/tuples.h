#ifndef TUPLES_H
#define TUPLES_H
#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "..\list\lists.h"

typedef struct
m_tuple
{
    int    count;
    m_list items;
    void*  content;
}
m_tuple;

m_tuple* m_tuple_init(int count, ...);

#define NUMARGS(...) ((int)sizeof((int[]){ __VA_ARGS__ })/sizeof(int))
#define m_tuple_create(...) (m_tuple_init(NUMARGS(__VA_ARGS__), __VA_ARGS__))

#endif