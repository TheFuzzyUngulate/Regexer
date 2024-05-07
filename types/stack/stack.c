#include "stack.h"

m_stack _m_stack_init(size_t size)
{
    m_stack m;

    m.count    = 0;
    m.size     = size;
    m.capacity = 2;
    m.content  = malloc(m.capacity * m.size);

    return m;
}

int m_stack_push(m_stack* m, void* item)
{
    if (m->content == NULL) {
        m->capacity = 2;
        m->content = malloc(m->size * m->capacity);
    } else if (m->count >= m->capacity) {
        m->capacity *= 2;
        m->content = realloc(m->content, m->size * m->capacity);
    }
    if (memcpy(m->content + (m->size * m->count++), item, m->size))
        return 1;
    else return 0;
}

void* m_stack_pop(m_stack* m)
{
    if (m->count > 0) {
        m->count--;
        void* item = malloc(m->size);
        memcpy(item, m->content + (m->size * m->count), m->size);
        return item;
    } else return NULL;
}

void m_stack_delete(m_stack* m)
{
    free(m->content);
    free(m);
}