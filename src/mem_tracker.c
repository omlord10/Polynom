#include "../include/mem_tracker.h"

// отключаем макросы для самой реализации
#undef malloc
#undef calloc
#undef free

size_t g_total_allocated = 0;
size_t g_total_freed = 0;
size_t g_current_allocated = 0;

void* track_malloc(size_t size)
{
    void* ptr = malloc(size);
    if(ptr)
    {
        g_total_allocated += size;
        g_current_allocated += size;
    }
    return ptr;
}

void* track_calloc(size_t num, size_t size)
{
    void* ptr = calloc(num, size);
    if(ptr)
    {
        g_total_allocated += num * size;
        g_current_allocated += num * size;
    }
    return ptr;
}

void track_free(void* ptr, size_t size)
{
    if(ptr)
    {
        g_total_freed += size;
        g_current_allocated -= size;
        free(ptr);
    }
}
