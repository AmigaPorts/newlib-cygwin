/* A program starts with C's default FPU environment, whatever FPCR and
   FPSR the process had: a shell runs its commands in its own process, one
   after the other, and the IEEE maths libraries set FPCR when they are
   opened.  The test runs a copy of itself in its own process with
   RunCommand(), as a shell does.

   It needs AmigaOS: volamos 0.10 gives a RunCommand() child an FPU state
   of its own, so there the test passes either way, and the testsuite
   boards map no host directory for the program to load itself from.  */

#include <stdio.h>
#include <string.h>
#include <proto/dos.h>

#ifdef __HAVE_68881__

static unsigned long get_fpcr(void)
{
  unsigned long v;
  __asm__ volatile ("fmove.l %%fpcr,%0" : "=d"(v));
  return v;
}

static unsigned long get_fpsr(void)
{
  unsigned long v;
  __asm__ volatile ("fmove.l %%fpsr,%0" : "=d"(v));
  return v;
}

static void set_fpu(unsigned long cr, unsigned long sr)
{
  __asm__ volatile ("fmove.l %0,%%fpcr\n\tfmove.l %1,%%fpsr" :: "d"(cr), "d"(sr));
}

static int child(void)
{
  unsigned long cr = get_fpcr(), sr = get_fpsr();
  volatile double one = 1.0, ten = 10.0;
  union { double d; unsigned long l[2]; } q;
  int fail = 0;

  q.d = one / ten;
  if (cr != 0 || sr != 0)
    {
      printf ("started with FPCR=$%lx FPSR=$%lx\n", cr, sr);
      fail = 1;
    }
  /* 0x3fb999999999999a rounded to nearest, ...9999 toward zero */
  if (q.l[0] != 0x3fb99999 || q.l[1] != 0x9999999a)
    {
      printf ("1.0/10.0 = %08lx%08lx\n", q.l[0], q.l[1]);
      fail = 1;
    }
  return fail;
}

int main(int argc, char **argv)
{
  BPTR seg;
  long rc;

  if (argc > 1 && !strcmp (argv[1], "child"))
    return child ();

  seg = LoadSeg ((STRPTR) argv[0]);
  if (!seg)
    {
      printf ("cannot load %s, not tested\n", argv[0]);
      return 0;
    }
  /* Round toward zero, double precision, and all five accrued exception
     flags: what a previous command could leave behind.  */
  set_fpu (0x90, 0xf8);
  rc = RunCommand (seg, 16384, (STRPTR) "child\n", 6);
  set_fpu (0, 0);
  UnLoadSeg (seg);
  return rc != 0;
}

#else

/* Nothing to check without an FPU.  */
int main(void)
{
  return 0;
}

#endif
