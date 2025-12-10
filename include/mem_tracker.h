#ifndef LAB3_MEM_TRACKER_H
#define LAB3_MEM_TRACKER_H

#include <stdlib.h>
#include <stddef.h>

extern size_t g_total_allocated;
extern size_t g_total_freed;
extern size_t g_current_allocated;

void* track_malloc(size_t size);
void* track_calloc(size_t num, size_t size);
void  track_free(void* ptr, size_t size);

#define malloc(x) track_malloc(x)
#define calloc(n,x) track_calloc(n,x)
#define free(ptr,x) track_free(ptr,x)

#endif //LAB3_MEM_TRACKER_H