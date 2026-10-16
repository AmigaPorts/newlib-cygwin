/* @(#)s_nextafter.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/* IEEE functions
 *	nextafter(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

#include <float.h>

#include "fpmath.h"
#include "math.h"
#include "math_private.h"

#if LDBL_MAX_EXP != 0x4000
#error "Unsupported long double format"
#endif

long double
nextafterl(long double x, long double y)
{
	volatile long double t;
	union IEEEl2bits ux, uy;

	ux.e = x;
	uy.e = y;

	if ((ux.bits.exp == 0x7fff &&
	     ((ux.bits.manh&~LDBL_NBIT)|ux.bits.manl) != 0) ||
	    (uy.bits.exp == 0x7fff &&
	     ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl) != 0))
	   return x+y;	/* x or y is nan */
	if(x==y) return y;		/* x=y, return y */
	if(x==0.0) {
	    ux.bits.manh = 0;			/* return +-minsubnormal */
	    ux.bits.manl = 1;
	    ux.bits.sign = uy.bits.sign;
	    t = ux.e*ux.e;
	    if(t==ux.e) return t; else return ux.e; /* raise underflow flag */
	}
#ifdef LDBL_EXP0_NOT_BUMPED
	/* An exponent of 0 covers the whole mantissa range, integer bit
	   included, so only a carry or a borrow at 2^63 moves the exponent.
	   The canonical infinity has no integer bit: add it, so that
	   stepping down from it gives LDBL_MAX.  */
	if(ux.bits.exp==0x7fff)
	    ux.xbits.man = 1ULL<<63;
	if(x>0.0 ^ x<y) {			/* x -= ulp */
	    if(ux.bits.exp!=0 && ux.xbits.man==1ULL<<63) {
		ux.bits.exp -= 1;
		ux.xbits.man = ~0ULL;
	    } else
		ux.xbits.man -= 1;
	} else {				/* x += ulp */
	    ux.xbits.man += 1;
	    if(ux.xbits.man==0) {
		ux.bits.exp += 1;
		ux.xbits.man = 1ULL<<63;
	    }
	}
	if(ux.bits.exp==0x7fff) return x+x;	/* overflow  */
	if(ux.bits.exp==0 && (ux.bits.manh&LDBL_NBIT)==0) { /* underflow */
	    t = ux.e * ux.e;
	    if(t!=ux.e)			/* raise underflow flag */
		return ux.e;
	}
	return ux.e;
#else
	if(x>0.0 ^ x<y) {			/* x -= ulp */
	    if(ux.bits.manl==0) {
		if ((ux.bits.manh&~LDBL_NBIT)==0)
		    ux.bits.exp -= 1;
		ux.bits.manh = (ux.bits.manh - 1) | (ux.bits.manh & LDBL_NBIT);
	    }
	    ux.bits.manl -= 1;
	} else {				/* x += ulp */
	    ux.bits.manl += 1;
	    if(ux.bits.manl==0) {
		ux.bits.manh = (ux.bits.manh + 1) | (ux.bits.manh & LDBL_NBIT);
		if ((ux.bits.manh&~LDBL_NBIT)==0)
		    ux.bits.exp += 1;
	    }
	}
	if(ux.bits.exp==0x7fff) return x+x;	/* overflow  */
	if(ux.bits.exp==0) {			/* underflow */
	    mask_nbit_l(ux);
	    t = ux.e * ux.e;
	    if(t!=ux.e)			/* raise underflow flag */
		return ux.e;
	}
	return ux.e;
#endif
}

__strong_reference(nextafterl, nexttowardl);
