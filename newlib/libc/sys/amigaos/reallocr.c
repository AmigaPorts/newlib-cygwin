/* _realloc_r on top of the amiga allocator in mallocr.cpp, which keeps
   the block size in the word before each block; replaces
   stdlib/reallocr.c (dlmalloc).  */
#include <stdlib.h>
#include <string.h>

void *	_realloc_r (struct _reent * r, void * old, size_t sz) {
	void * p = _malloc_r(r, sz);
	if (p)
	{
		if (old) {
			size_t * oldp = (size_t*)old;
			size_t copy = *--oldp;
			if (copy > sz) copy = sz;
			memcpy(p, old, copy);
			_free_r(r, old);
		}
	}
	return p;
}
