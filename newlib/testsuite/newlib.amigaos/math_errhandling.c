/* The math functions report errors as math_errhandling says: errno with
   MATH_ERRNO, and the <fenv.h> exception flags with MATH_ERREXCEPT, which
   the 68881 builds support.  */

#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

/* Without an FPU, <fenv.h> defines none of these, and math_errhandling
   doesn't promise MATH_ERREXCEPT. */
#ifndef FE_INVALID
#define FE_INVALID 0
#define FE_DIVBYZERO 0
#define FE_OVERFLOW 0
#define FE_UNDERFLOW 0
#endif

static volatile double m1 = -1, z = 0, big = DBL_MAX, two = 2, k1000 = 1000;
static int failures;

static double f_sqrt(void) { return sqrt(m1); }
static double f_log0(void) { return log(z); }
static double f_logm(void) { return log(m1); }
static double f_exp(void) { return exp(big); }
static double f_expm(void) { return exp(-k1000); }
static double f_pow(void) { return pow(z, m1); }
static double f_acos(void) { return acos(two); }
static double f_cosh(void) { return cosh(k1000); }

static const struct {
  const char *name;
  double (*f)(void);
  int err;
  int except;
} tests[] = {
  { "sqrt(-1)", f_sqrt, EDOM, FE_INVALID },
  { "log(0)", f_log0, ERANGE, FE_DIVBYZERO },
  { "log(-1)", f_logm, EDOM, FE_INVALID },
  { "exp(DBL_MAX)", f_exp, ERANGE, FE_OVERFLOW },
  { "exp(-1000)", f_expm, ERANGE, FE_UNDERFLOW },
  { "pow(0, -1)", f_pow, ERANGE, FE_DIVBYZERO },
  { "acos(2)", f_acos, EDOM, FE_INVALID },
  { "cosh(1000)", f_cosh, ERANGE, FE_OVERFLOW },
};

int main(void)
{
  unsigned i;

  for (i = 0; i < sizeof tests / sizeof tests[0]; i++)
    {
      volatile double r;
      int raised;

      errno = 0;
      feclearexcept(FE_ALL_EXCEPT);
      r = tests[i].f();
      (void)r;
      raised = fetestexcept(FE_ALL_EXCEPT);

      if ((math_errhandling & MATH_ERRNO) && errno != tests[i].err)
        {
          printf("FAIL: %s: errno %d, want %d\n", tests[i].name, errno, tests[i].err);
          failures++;
        }
      if ((math_errhandling & MATH_ERREXCEPT) && !(raised & tests[i].except))
        {
          printf("FAIL: %s: exceptions %#x, want %#x\n", tests[i].name, raised, tests[i].except);
          failures++;
        }
    }
  return failures != 0;
}
