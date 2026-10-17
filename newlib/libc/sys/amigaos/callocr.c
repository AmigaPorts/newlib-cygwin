/* _calloc_r on top of the amiga allocator in mallocr.cpp; replaces
   stdlib/callocr.c (dlmalloc).  */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <reent.h>

void *	_calloc_r (struct _reent * r, size_t a, size_t b) {
	size_t sz;
	void * p;

	/* a * b must not wrap into a small allocation.  */
	if (__builtin_mul_overflow(a, b, &sz)) {
		_REENT_ERRNO(r) = ENOMEM;
		return NULL;
	}
	p = _malloc_r(r, sz);
	if (p)
		memset(p, 0, sz);
	return p;
}
