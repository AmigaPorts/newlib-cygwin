/* A lock whose malloc() fails shares a fallback with every other such
   lock: a task must still be able to hold two of them, as it could two
   locks of their own.  The heap is exhausted first, so this is for the
   simulator, not a machine in use.  Try-acquire throughout, so that a
   regression fails instead of hanging.  A newlib built without locks,
   for a toolchain without threads, has nothing to test.  */
#include <stdio.h>
#include <stdlib.h>
#include <sys/lock.h>

#ifndef _RETARGETABLE_LOCKING
int main(void)
{
  return 0;
}
#else

struct block
{
  struct block *next;
};

static struct block *held;
static int failures;
static const char *failed[8];

static void check(int ok, const char *what)
{
  if (!ok && failures < 8)
    failed[failures++] = what;
}

/* Allocate until malloc() fails at every size down to a block's.  */
static void exhaust(void)
{
  size_t size;
  struct block *b;

  for (size = 1 << 20; size >= sizeof (struct block); size /= 2)
    while ((b = malloc(size)))
      {
        b->next = held;
        held = b;
      }
}

static void give_back(void)
{
  struct block *b;

  while ((b = held))
    {
      held = b->next;
      free(b);
    }
}

int main(void)
{
  _LOCK_RECURSIVE_T a, b;
  int i;

  /* Nothing may print until the heap is back: stdio allocates.  */
  exhaust();
  __lock_init_recursive(a);
  __lock_init_recursive(b);
  check(a == b, "the locks share the fallback");

  /* One lock, then a try of the other.  The blocking lock again only once
     trying shows that it can't block.  */
  __lock_acquire_recursive(a);
  check(__lock_try_acquire_recursive(b), "a second lock");
  if (!failures)
    {
      __lock_acquire_recursive(b);
      __lock_release_recursive(b);
      __lock_release_recursive(b);
    }
  __lock_release_recursive(a);

  /* All released: no owner left.  */
  check(!a->__r.__owner, "free after the releases");

  __lock_close_recursive(a);
  __lock_close_recursive(b);
  give_back();

  for (i = 0; i < failures; i++)
    printf("FAIL: %s\n", failed[i]);
  printf("lock-nomem: %d failures\n", failures);
  return failures;
}

#endif /* _RETARGETABLE_LOCKING */
