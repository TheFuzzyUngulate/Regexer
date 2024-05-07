#include "bstree.h"

static inline int
m_bstree_entry_size(m_bstree_entry* t)
{
    if (t != NULL) {
        int s = 1;
        if (t->left != NULL)
            s += m_bstree_entry_size(t->left);
        if (t->right != NULL)
            s += m_bstree_entry_size(t->right);
        return s;
    } else return 0;
}

static inline m_bstree_entry* 
m_bstree_get_max(m_bstree_entry* t)
{
    if (t != NULL) {
        m_bstree_entry* old = t;
        m_bstree_entry* max = t;
        while (max->right != NULL) {
            old = max;
            max = max->right;
        }
        if (old != max)
            old->right = NULL;
        return max;
    } return NULL;
}

static inline m_bstree_entry* 
m_bstree_get_min(m_bstree_entry* t)
{
    if (t != NULL) {
        m_bstree_entry* old = t;
        m_bstree_entry* min = t;
        while (min->left != NULL) {
            old = min;
            min = min->left;
        }
        if (old != min)
            old->right = NULL;
        return min;
    } return NULL;
}

static inline void 
m_bstree_entry_print(m_bstree_entry* t, char* (*tostr)(const void*), int INDENT)
{
    if (t != NULL) {
        for (int i = 0; i < INDENT; ++i) 
            printf(" ");
        printf("data: %s\n", tostr(t->data));
        m_bstree_entry_print(t->left, tostr, INDENT+1);
        m_bstree_entry_print(t->right, tostr, INDENT+1);
    }
}

static inline m_bstree_entry*
m_bstree_right_rotate(m_bstree_entry* y)
{
    m_bstree_entry* x   = y->left;
    m_bstree_entry* tmp = x->right;

    x->right  = y;
    y->left   = tmp;
    x->height = TREEHEIGHT(x);
    y->height = TREEHEIGHT(y);

    return x;
}

static inline m_bstree_entry*
m_bstree_left_rotate(m_bstree_entry* x)
{
    m_bstree_entry* y   = x->right;
    m_bstree_entry* tmp = y->left;

    y->left   = x;
    x->right  = tmp;
    x->height = TREEHEIGHT(x);
    y->height = TREEHEIGHT(y);

    return y;
}

void m_bstree_print(m_bstree* t) {
    m_bstree_entry_print(t->troot, t->to_string, 0);
}

int m_bstree_size(m_bstree* t)
{
    int s = 0;
    m_bstree_entry *k = t->troot;
    return m_bstree_entry_size(k);
}

static inline m_bstree_entry*
m_bstree_entry_init(m_bstree* t, void* m)
{
    m_bstree_entry* n = malloc(sizeof(m_bstree_entry));
    n->data = malloc(t->size);
    memcpy(n->data, m, t->size);
    n->left = NULL;
    n->right = NULL;
    n->height = 1;
    return n;
}

static m_bstree_entry*
m_bstree_entry_add(m_bstree* t, m_bstree_entry* k, void* m)
{
    int cmp, bal;

    if (k == NULL) return m_bstree_entry_init(t, m);

    cmp = t->compare(m, k->data);
    if (cmp < 0) {
        k->left = m_bstree_entry_add(t, k->left, m);
        if (k->left == NULL) return NULL;
    }
    else if (cmp > 0) {
        k->right = m_bstree_entry_add(t, k->right, m);
        if (k->right == NULL) return NULL;
    }
    else return NULL;

    bal       = TREEBALANCE(k);
    k->height = TREEHEIGHT(k);

    if (bal > 1 && t->compare(m, k->left->data) < 0)
        return m_bstree_right_rotate(k);
    if (bal < -1 && t->compare(m, k->right->data) > 0)
        return m_bstree_left_rotate(k);
    if (bal > 1 && t->compare(m, k->left->data) > 0) {
        k->left = m_bstree_left_rotate(k->left);
        return m_bstree_right_rotate(k);
    }
    if (bal < -1 && t->compare(m, k->right->data) < 0) {
        k->right = m_bstree_right_rotate(k->right);
        return m_bstree_left_rotate(k);
    }

    return k;
}

int m_bstree_add(m_bstree* t, void* m) {
    m_bstree_entry* k = m_bstree_entry_add(t, t->troot, m);
    if (k != NULL) {
        t->troot = k;
        return 0;
    } else return -1;
}

void* m_bstree_find(m_bstree* t, void* m)
{
    m_bstree_entry *k = t->troot;
    while (k != NULL)
    {
        int cmp = t->compare(m, k->data);
        if (cmp > 0) {
            if (k->right == NULL)
                return NULL;
            else k = k->right;
        }
        else
        if (cmp < 0) {
            if (k->left == NULL)
                return NULL;
            else k = k->left;
        }
        else return k->data;
    }
}

int m_bstree_remove(m_bstree* t, void *m)
{
    m_bstree_entry **k = &(t->troot);
    while (k != NULL)
    {
        int cmp = t->compare((*k)->data, m);
        if (cmp > 0) {
            if ((*k)->right == NULL)
                return -1;
            else *k = (*k)->right;
        }
        else
        if (cmp < 0) {
            if ((*k)->left == NULL)
                return -1;
            else *k = (*k)->left;
        }
        else 
        {
            m_bstree_entry *left = (*k)->left;
            m_bstree_entry *right = (*k)->right;

            if (left == NULL) {
                if (right == NULL) {
                    free(*k);
                    *k = NULL;
                } else *k = right;
                return 0;
            }

            if (right == NULL) {
                *k = left;
                return 0;
            }

            *k = left;
            m_bstree_entry* kmax = m_bstree_get_max((*k)->right);
            if (kmax != NULL) {
                kmax->right = right;
                kmax->left = (*k)->right;
                (*k)->right = kmax;
            } else (*k)->right = right;
            return 0;
        }
    }
    return -1;
}

m_bstree* m_bstree_init(size_t size,
                        int (*cmp)(const void*, const void*),
                        char* (*str)(const void*))
{
    m_bstree* t = (m_bstree*)malloc(sizeof(m_bstree));

    t->size          = size;
    t->troot         = NULL;
    t->table_height  = 0;
    t->compare       = cmp;
    t->to_string     = str;

    return t;
}

static int
m_bstree_entry_destroy(m_bstree* m, m_bstree_entry* k)
{
    if (k->left != NULL)
        m_bstree_entry_destroy(m, k->left);
    if (k->right != NULL)
        m_bstree_entry_destroy(m, k->right);
    free(k->data);
    free(k);
}

int m_bstree_destroy(m_bstree* t)
{
    m_bstree_entry_destroy(t, t->troot);
    free(t);
    return 0;
}

static m_bstree_entry*
m_bstree_entry_copy(m_bstree_entry* src)
{
    m_bstree_entry* dst = NULL;

    if (src != NULL) {
        dst         = (m_bstree_entry*)malloc(sizeof(m_bstree_entry));
        dst->data   = src->data;
        dst->height = src->height;
        dst->left   = m_bstree_entry_copy(src->left);
        dst->right  = m_bstree_entry_copy(src->right);
    } 
    
    return dst;
}

m_bstree* m_bstree_copy(m_bstree* src)
{
    m_bstree* t = NULL;

    if (src != NULL) {
        t                = (m_bstree*)malloc(sizeof(m_bstree));
        t->size          = src->size;
        t->table_height  = src->table_height;
        t->compare       = src->compare;
        t->to_string     = src->to_string;
        t->troot         = m_bstree_entry_copy(src->troot);
    }

    return t;
}