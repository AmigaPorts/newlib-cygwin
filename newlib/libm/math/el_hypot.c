/* Copyright (C) 2015 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

long double
__ieee754_hypotl (long double x, long double y)
{
#ifdef _LDBL_EQ_DBL
  return __ieee754_hypot (x, y);
#else
  long double a = fabsl (x), b = fabsl (y), t;
  int e;

  if (isinf (a) || isinf (b))
    return __builtin_infl ();	/* even when the other one is a NaN */
  if (isnan (a) || isnan (b))
    return a + b;
  if (a < b)
    t = a, a = b, b = t;
  if (b == 0)
    return a;

  /* Scale both by a power of two, so that a*a neither overflows nor
     underflows.  b*b can still underflow, but only when it is too small
     to change the sum.  */
  frexpl (a, &e);
  a = ldexpl (a, -e);
  b = ldexpl (b, -e);
  return ldexpl (sqrtl ((a * a) + (b * b)), e);
#endif
}
