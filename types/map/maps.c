#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maps.h"

unsigned long
m_hashmap_def_hashfunc(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;
    
    return hash;
}

static int m_hashentry_comp(const void* a, const void *b)
{
    m_hashentry* am = (m_hashentry*)a;
    m_hashentry* bm = (m_hashentry*)b;
    if (am->key > bm->key) return 1;
    if (am->key < bm->key) return -1;
    return 0;
}

static int m_dump_comp(const void* a, const void* b)
{
    m_dumpentry* ad = (m_dumpentry*)a;
    m_dumpentry* bd = (m_dumpentry*)b;
    return strcmp(ad->key, bd->key);
}

static char* m_dump_to_string(const void* a) {return (char*)a;}

static inline void 
m_hashentry_print(m_bstree_entry* m, char* (*tostr)(const void*), int INDENT)
{
    if (m != NULL) {
        m_hashentry* h = (m_hashentry*)m->data;
        if (h != NULL) {
            for (int i = 0; i < INDENT; ++i) 
                printf(" ");
            printf("key: %i, value: %s\n", h->key, tostr(h->value));
            m_hashentry_print(m->left, tostr, INDENT+1);
            m_hashentry_print(m->right, tostr, INDENT+1);
        }
    }
}

void m_hashmap_print(m_hashmap* m) {
    m_hashentry_print(m->tree->troot, m->tree->to_string, 0);
}

static void*
m_hashmap_def_item_copy(m_hashmap* m, void* src)
{
    void* dst = malloc(m->vsize);
    memcpy(dst, src, m->vsize);
    return dst;
}

static int
m_hashmap_def_item_destr(m_hashmap* m, void* src)
{
    free(src);
    return 0;
}

m_hashmap* m_hashmap_init(int vsize, 
                      unsigned long (*hash)(unsigned char* str),
                      char* (*tostr)(const void*))
{
    m_hashmap* map = (m_hashmap*)malloc(sizeof(m_hashmap));
    map->hash      = hash == NULL ? m_hashmap_def_hashfunc : hash;
    map->vsize     = vsize;
    map->tree      = m_bstree_init(sizeof(m_hashentry), m_hashentry_comp, tostr);
    map->dump      = m_bstree_init(sizeof(m_dumpentry), m_dump_comp, m_dump_to_string);
    
    return map;
}

int m_hashmap_destroy(m_hashmap* m) {
    m_bstree_destroy(m->tree);
    m_bstree_destroy(m->dump);
    free(m);
    return 0;
}

void* m_hashmap_get_by_key(m_hashmap *m, char* key)
{
    m_hashentry* e = (m_hashentry*)malloc(sizeof(m_hashentry));
    e->key = m->hash(key);
    printf("Finding %s, with hash %u\n", key, e->key);
    void* ret = m_bstree_find(m->tree, e);
    if (ret == NULL) {
        printf("Didn't find it??\n");
        m_dumpentry* d = (m_dumpentry*)malloc(sizeof(m_dumpentry));
        d->key = key;
        ret = m_bstree_find(m->dump, d);
        return (!ret) ? NULL : ((m_dumpentry*)ret)->value;
    } else return ((m_hashentry*)ret)->value;
}

void* m_hashmap_get_by_value(m_hashmap *m, void* val)
{
    return NULL;
}

int m_hashmap_remove_entry(m_hashmap *m, void* res)
{
    return 0;
}

int m_hashmap_add_entry(m_hashmap *m, char* key, void* val)
{
    m_hashentry* e = (m_hashentry*)malloc(sizeof(m_hashentry));
    e->key = m->hash(key);
    e->value = malloc(m->vsize);
    memcpy(e->value, val, m->vsize);
    if (m_bstree_add(m->tree, e) == -1)
    {
        m_hashentry* e2 = (m_hashentry*)m_bstree_find(m->tree, e);
        if (e2->key != e->key) {
            m_dumpentry* d = (m_dumpentry*)malloc(sizeof(m_dumpentry));
            d->key = key;
            d->value = malloc(m->vsize);
            memcpy(d->value, val, m->vsize);
            m_bstree_add(m->dump, d);
        }
    }
}