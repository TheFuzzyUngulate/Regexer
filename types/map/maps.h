#ifndef MAPS_H
#define MAPS_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "..\list\lists.h"
#include "..\bstree\bstree.h"

typedef struct m_hashentry
{
    void* value;
    unsigned int key;
}
m_hashentry;

typedef struct m_dumpentry
{
    char* key;
    void* value; 
}
m_dumpentry;

typedef struct m_hashmap
{
    size_t vsize;                               // Size of hashmap values
    m_bstree* tree;                             // Hashmap tree size
    m_bstree* dump;                             // Place to dump collisions
    unsigned long (*hash)(unsigned char* str);  // Hash function of hashmap
}
m_hashmap;

void m_hashmap_print(m_hashmap* m);
int m_hashmap_destroy(m_hashmap* m);
void* m_hashmap_get_by_key(m_hashmap *m, char* key);
void* m_hashmap_get_by_value(m_hashmap *m, void* val);
int m_hashmap_remove_entry(m_hashmap *m, void* res);
int m_hashmap_add_entry(m_hashmap *m, char* key, void* val);
m_hashmap* m_hashmap_init(int vsize, unsigned long (*hash)(unsigned char* str), char* (*tostr)(const void*));

#define m_hashmap_create(type, hash, tstr) (m_hashmap_init(sizeof((type)), (hash), (tstr)))

#endif