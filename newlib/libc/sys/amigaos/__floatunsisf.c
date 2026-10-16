#include <proto/mathieeesingbas.h>
extern double __floatunsidf(signed long x);
/* Through double, which holds any 32-bit value exactly, so that the
   result is rounded only once.  */
float __floatunsisf(signed long x)
{ if (x >= 0) return IEEESPFlt(x);
  return __floatunsidf(x);
}
