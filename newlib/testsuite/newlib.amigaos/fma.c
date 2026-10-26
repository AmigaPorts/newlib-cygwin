/* fma() rounds the exact x * y + z once.  With z == 0 that is x * y rounded
   once to double: the 68881 evaluates in extended, so x * y as written
   rounds twice there.  */

#include <math.h>
#include <stdio.h>

static int failures;

static void check(int ok, const char *what)
{
  if (!ok)
    {
      printf("FAIL: %s\n", what);
      failures++;
    }
}

int main(void)
{
  volatile double one = 1.0, eps = 0x1p-52;
  volatile double r;

  /* (1 + 2^-52)(1 - 2^-52) = 1 - 2^-104: only the fused sum sees the 2^-104 */
  r = fma(one + eps, one - eps, -one);
  check(r == -0x1p-104, "fma(1+e, 1-e, -1) == -2^-104");

  r = fma(0x1.27365a6456ba5p+0, 0x1.63fdaf7759a4fp+0, 0.0);
  check(r == 0x1.9a84ea6842dcdp+0, "fma(x, y, +0) rounds x * y once");
  r = fma(0x1.27365a6456ba5p+0, 0x1.63fdaf7759a4fp+0, -0.0);
  check(r == 0x1.9a84ea6842dcdp+0, "fma(x, y, -0) rounds x * y once");
  r = fma(-0x1p-600, 0x1p-600, 0.0);
  check(r == 0.0 && signbit(r), "fma(-2^-600, 2^-600, +0) == -0");

  printf("%d failures\n", failures);
  return failures != 0;
}
