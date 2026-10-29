/* strtold() must return the long double the compiler reads from the same
   text: on m68k, the result was written in the 10-byte x87 layout into the
   12-byte long double, so every value came back as garbage.  */
#include <float.h>
#include <stdlib.h>

static void check(const char *text, long double want)
{
  char *end;
  long double got = strtold(text, &end);

  if (*end != '\0' || got != want)
    abort();
}

int main(void)
{
#if LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
  check("1.0", 1.0L);
  check("0x8.111111111111113p0", 0x8.111111111111113p0L);
  check("118973149535723176505351158982948.86679662540046955672e4900", LDBL_MAX);
  check("0x1p-16445", 0x1p-16445L);
  check("1e5000", __builtin_infl());
  check("-2.5", -2.5L);
#endif
  return 0;
}
