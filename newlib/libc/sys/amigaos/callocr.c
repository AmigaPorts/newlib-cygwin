/* _calloc_r on top of the amiga allocator in mallocr.cpp; replaces
   stdlib/callocr.c (dlmalloc).  */
#include <stdlib.h>
#include <string.h>

void *	_calloc_r (struct _reent * r, size_t a, size_t b) {
	size_t sz = a * b;
	void * p = _malloc_r(r, sz);
	if (p)
		memset(p, 0, sz);
	return p;
}
