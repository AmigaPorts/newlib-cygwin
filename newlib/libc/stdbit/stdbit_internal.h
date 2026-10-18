/*
 * Newlib specific addition to ensure the C23 *_WIDTH constants
 * used in the stdbits implementation are available even when
 * compiling using another version of the C Programming Language.
 */

/*
 *  Written by Joel Sherrill <joel.sherrill@OARcorp.com>.
 *
 *  COPYRIGHT (c) 2026.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Permission to use, copy, modify, and distribute this software for any
 *  purpose without fee is hereby granted, provided that this entire notice
 *  is included in all copies of any software which is or includes a copy
 *  or modification of this software.
 *
 *  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 *  WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
 *  OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
 *  SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#ifndef STDBIT_INTERNAL_h
#define STDBIT_INTERNAL_h

/* The __*_WIDTH__ predefines date from GCC 7; older compilers get the
   widths from the type sizes, which are predefined since GCC 4.3 and
   usable in #if like the widths.  */
#ifndef UCHAR_WIDTH
#define UCHAR_WIDTH __CHAR_BIT__
#endif

#ifndef USHRT_WIDTH
#ifdef __SHRT_WIDTH__
#define USHRT_WIDTH __SHRT_WIDTH__
#else
#define USHRT_WIDTH (__CHAR_BIT__ * __SIZEOF_SHORT__)
#endif
#endif

#ifndef UINT_WIDTH
#ifdef __INT_WIDTH__
#define UINT_WIDTH __INT_WIDTH__
#else
#define UINT_WIDTH (__CHAR_BIT__ * __SIZEOF_INT__)
#endif
#endif

#ifndef ULONG_WIDTH
#ifdef __LONG_WIDTH__
#define ULONG_WIDTH __LONG_WIDTH__
#else
#define ULONG_WIDTH (__CHAR_BIT__ * __SIZEOF_LONG__)
#endif
#endif

#ifndef ULLONG_WIDTH
#ifdef __LONG_LONG_WIDTH__
#define ULLONG_WIDTH __LONG_LONG_WIDTH__
#else
#define ULLONG_WIDTH (__CHAR_BIT__ * __SIZEOF_LONG_LONG__)
#endif
#endif

#endif
