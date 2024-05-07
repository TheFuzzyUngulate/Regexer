#ifndef BSTREE_H
#define BSTREE_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define TREEHEIGHT(X) (1 + ((X)->left == NULL ? ((X)->right == NULL ? 0 : (X)->right->height)\
    : ((X)->right == NULL ? (X)->left->height\
    : MAX((X)->left->height, (X)->right->height))))
#define TREEBALANCE(T) ((T) == NULL ? 0 :\
    ((T)->left == NULL ? ((T)->right == NULL ? 0 : -((T)->right->height)) \
    : ((T)->right == NULL ? (T)->left->height : (T)->left->height - (T)->right->height)))

typedef struct m_bstree_entry
{
    void* data;
    int height;
    struct m_bstree_entry* left;
    struct m_bstree_entry* right;
}
m_bstree_entry;

typedef struct m_bstree
{
    size_t size;                                // Size of a node
    size_t table_height;                        // Height of the tree
    m_bstree_entry *troot;                      // Root of the binary tree
    char* (*to_string)(const void*);            // Function converting node to printable
    int (*compare)(const void*, const void*);   // Function for comparing two nodes
}
m_bstree;

int m_bstree_size(m_bstree* t);
void m_bstree_print(m_bstree* t);
int m_bstree_destroy(m_bstree* t);
int m_bstree_add(m_bstree* t, void* m);
void* m_bstree_find(m_bstree* t, void* m);
int m_bstree_remove(m_bstree* t, void* m);
m_bstree* m_bstree_copy(m_bstree* src);
m_bstree* m_bstree_init(size_t size, int (*cmp)(const void*, const void*), char* (*str)(const void*));

#define m_bstree_iter_create() ((m_bstree_iter){NULL, 0, 0, NULL})
#define m_bstree_create(type, cmp, str) (m_bstree_init(sizeof(type), (cmp), (str)))

#endif