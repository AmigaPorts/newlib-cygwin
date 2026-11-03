/* _realloc_r on top of the amiga allocator in mallocr.cpp, which keeps
   the block size in the word before each block; replaces
   stdlib/reallocr.c (dlmalloc).  */
#include <stdlib.h>
#include <string.h>

/* The size counts the block's MemMap::Node too: next, prev, leaf and size,
   which sit in front of the data.  */
#define NODE_SIZE (4 * sizeof(void *))

void *	_realloc_r (struct _reent * r, void * old, size_t sz) {
	void * p = _malloc_r(r, sz);
	if (p)
	{
		if (old) {
			size_t * oldp = (size_t*)old;
			size_t copy = *--oldp - NODE_SIZE;
			if (copy > sz) copy = sz;
			memcpy(p, old, copy);
			_free_r(r, old);
		}
	}
	return p;
}
