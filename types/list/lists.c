#include "lists.h"

/**
 * @brief Initialize a list.
 * 
 * @param size Size of each data element to be stored in list.
 * @param to_string Function converting data element to printable form.
 * @return Pointer to dynamically allocated pointer.
 */
m_list* m_list_init(size_t size) 
{
    m_list* m    = malloc(sizeof(m_list));
    m->size      = size;
    m->count     = 0;
    m->capacity  = 2;
    m->array     = malloc(m->capacity * m->size);

    return m;
}

/**
 * @brief Clear list.
 * 
 * @param m Pointer to list to be cleared.
 * @return 1 if sucessful, 0 otherwise.
 */
int m_list_clear(m_list *m) {
    memset(m->array, 0, m->count * m->size);
    m->count = 0;
    return 0;
}

/**
 * @brief Change list to normal array
 * 
 * @param m Pointer to list on which the operation is to be done.
 * @return A pointer pointing to a contiguous block of data, containing copies of members of the list.
 */
void* m_list_to_array(m_list* m)
{
    void* array = malloc(m->size * m->count);
    memcpy(array, m->array, m->count * m->size);
    return array;
}

/**
 * @brief Create a copy of a list.
 * 
 * @param src List to copy.
 * @return Pointer to copy of list.
 */
m_list* m_list_copy(m_list* src)
{
    m_list* dst = (m_list*)malloc(sizeof(m_list));

    dst->size       = src->size;
    dst->count      = src->count;
    dst->capacity   = src->capacity;
    dst->array      = malloc(dst->capacity * dst->size);
    memcpy(dst->array, src->array, src->count * src->size);

    return dst;
}

/**
 * @brief Fill target list with values from source list
 * 
 * @param tar Target list.
 * @param src Source list.
 * @return 1 if successful, 0 otherwise.
 */
int m_list_extend(m_list* tar, m_list* src)
{
    if (tar->size != src->size) return 0;
    tar->array = realloc(tar->array, (tar->count + src->count) * tar->size);
    if (memcpy(tar->array + (tar->count * tar->size), src->array, src->count * src->size))
        return 1;
    else return 0;
}

/**
 * @brief Destroy list
 * 
 * @param m Pointer to list to be destroyed.
 */
void m_list_destroy(m_list* m)
{
    free(m->array);
    free(m);
}

/**
 * @brief Append copy of element to the end of the list
 * 
 * @param m Pointer to list in which the operation is to be done.
 * @param i Pointer to element to be appended.
 * @return 1 if operation is successful, 0 otherwise.
 */
int m_list_append(m_list* m, void* i) {
    if (m->array == NULL) {
        m->capacity = 2;
        m->array = malloc(m->size * m->capacity);
    } else if (m->count >= m->capacity) {
        m->capacity *= 2;
        m->array = realloc(m->array, m->size * m->capacity);
    }
    if (memcpy(m->array + (m->size * m->count++), i, m->size))
        return 1;
    else return 0;
}

/**
 * @brief Pop an element from the back of the list
 * 
 * @param m Pointer to list in which the operation is done.
 * @return 0 if list was empty, 1 otherwise.
 */
int m_list_pop(m_list* m) {
    if (m->count > 0) {
        m->count--;
        return 1;
    }
    else return 0;
}

/**
 * @brief Get element in list by index.
 * 
 * @param m Pointer to list in which the search is done.
 * @param index Index to search in the list.
 * @return A void pointer to a copy of the element in the index if index is in range, NULL otherwise.
 */
void* m_list_get(m_list* m, int index) {
    void* res = NULL;
    if (m->count > index) {
        res = malloc(m->size);
        if (memcpy(res, m->array + (index * m->size), m->size) == NULL)
            return NULL;
        return res;
    } else return NULL;
}

/**
 * @brief Get element at the back of the list.
 * 
 * @param m Pointer to list in which the search is done.
 * @return A void pointer to a copy of the element at the back of the list, if list is not empty. NULL otherwise.
 */
void* m_list_back(m_list* m)
{
    void* res = NULL;
    if (m->count > 0) {
        res = malloc(m->size);
        if (memcpy(res, m->array + ((m->count-1) * m->size), m->size) == NULL)
            return NULL;
        return res;
    } else return NULL;
}

/**
 * @brief Remove element in index from list.
 * 
 * @param m Pointer to list in which the deletion is done.
 * @param index Index containing the element to be removed.
 * @return 0 if successful, -1 otherwise.
 */
int m_list_remove(m_list* m, int index) {
    if (m->count > index) {
        memcpy(
            m->array + (index * m->size),
            m->array + ((index+1) * m->size),
            (m->count - index+1) * m->size
        );
        m->count--;
        return 0;
    } else return -1;
}

/**
 * @brief Replace element in index in list.
 * 
 * @param m Pointer to list in which the replacement is done.
 * @param index Index of element to be replacement.
 * @param i Pointer to replacement item.
 * @return 0 if successful, -1 otherwise.
 */
int m_list_replace(m_list* m, int index, void* i)
{
    if (m->count > index) {
        memcpy(m->array + (index * m->size), i, m->size);
        return 0;
    } else return -1;
}