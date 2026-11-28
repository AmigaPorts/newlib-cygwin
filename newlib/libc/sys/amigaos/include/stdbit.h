/*-
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __STDC_VERSION_STDBIT_H__
#define __STDC_VERSION_STDBIT_H__ 202311L

#include <sys/cdefs.h>

/* ixemul's <sys/cdefs.h> has none */
#ifndef __pure2
#define __pure2 __attribute__((__pure__))
#endif

/* byte order */
#define  __STDC_ENDIAN_LITTLE__ __ORDER_LITTLE_ENDIAN__
#define  __STDC_ENDIAN_BIG__ __ORDER_BIG_ENDIAN__
#define  __STDC_ENDIAN_NATIVE__ __BYTE_ORDER__

/* The functions are in newlib's libc.a, for (stdc_leading_zeros_ui)(x) and
   their address.  A call is gcc's builtin, inline, so every runtime has
   them; newlib's own build defines the functions, and gets no macros.  */

__BEGIN_DECLS
__stdargs unsigned int stdc_leading_zeros_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_leading_zeros_us(unsigned short) __pure2;
__stdargs unsigned int stdc_leading_zeros_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_leading_zeros_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_leading_zeros_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_leading_ones_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_leading_ones_us(unsigned short) __pure2;
__stdargs unsigned int stdc_leading_ones_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_leading_ones_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_leading_ones_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_trailing_zeros_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_trailing_zeros_us(unsigned short) __pure2;
__stdargs unsigned int stdc_trailing_zeros_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_trailing_zeros_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_trailing_zeros_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_trailing_ones_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_trailing_ones_us(unsigned short) __pure2;
__stdargs unsigned int stdc_trailing_ones_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_trailing_ones_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_trailing_ones_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_first_leading_zero_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_first_leading_zero_us(unsigned short) __pure2;
__stdargs unsigned int stdc_first_leading_zero_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_first_leading_zero_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_first_leading_zero_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_first_leading_one_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_first_leading_one_us(unsigned short) __pure2;
__stdargs unsigned int stdc_first_leading_one_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_first_leading_one_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_first_leading_one_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_first_trailing_zero_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_first_trailing_zero_us(unsigned short) __pure2;
__stdargs unsigned int stdc_first_trailing_zero_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_first_trailing_zero_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_first_trailing_zero_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_first_trailing_one_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_first_trailing_one_us(unsigned short) __pure2;
__stdargs unsigned int stdc_first_trailing_one_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_first_trailing_one_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_first_trailing_one_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_count_zeros_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_count_zeros_us(unsigned short) __pure2;
__stdargs unsigned int stdc_count_zeros_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_count_zeros_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_count_zeros_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_count_ones_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_count_ones_us(unsigned short) __pure2;
__stdargs unsigned int stdc_count_ones_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_count_ones_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_count_ones_ull(unsigned long long) __pure2;

__stdargs _Bool stdc_has_single_bit_uc(unsigned char) __pure2;
__stdargs _Bool stdc_has_single_bit_us(unsigned short) __pure2;
__stdargs _Bool stdc_has_single_bit_ui(unsigned int) __pure2;
__stdargs _Bool stdc_has_single_bit_ul(unsigned long) __pure2;
__stdargs _Bool stdc_has_single_bit_ull(unsigned long long) __pure2;

__stdargs unsigned int stdc_bit_width_uc(unsigned char) __pure2;
__stdargs unsigned int stdc_bit_width_us(unsigned short) __pure2;
__stdargs unsigned int stdc_bit_width_ui(unsigned int) __pure2;
__stdargs unsigned int stdc_bit_width_ul(unsigned long) __pure2;
__stdargs unsigned int stdc_bit_width_ull(unsigned long long) __pure2;

__stdargs unsigned char stdc_bit_floor_uc(unsigned char) __pure2;
__stdargs unsigned short stdc_bit_floor_us(unsigned short) __pure2;
__stdargs unsigned stdc_bit_floor_ui(unsigned int) __pure2;
__stdargs unsigned long stdc_bit_floor_ul(unsigned long) __pure2;
__stdargs unsigned long long stdc_bit_floor_ull(unsigned long long) __pure2;

__stdargs unsigned char stdc_bit_ceil_uc(unsigned char) __pure2;
__stdargs unsigned short stdc_bit_ceil_us(unsigned short) __pure2;
__stdargs unsigned int stdc_bit_ceil_ui(unsigned int) __pure2;
__stdargs unsigned long stdc_bit_ceil_ul(unsigned long) __pure2;
__stdargs unsigned long long stdc_bit_ceil_ull(unsigned long long) __pure2;
__END_DECLS

#ifndef _LIBC
	#define stdc_leading_zeros(x) __builtin_stdc_leading_zeros(x)
	#define stdc_leading_zeros_uc(x) __builtin_stdc_leading_zeros((unsigned char)(x))
	#define stdc_leading_zeros_us(x) __builtin_stdc_leading_zeros((unsigned short)(x))
	#define stdc_leading_zeros_ui(x) __builtin_stdc_leading_zeros((unsigned int)(x))
	#define stdc_leading_zeros_ul(x) __builtin_stdc_leading_zeros((unsigned long)(x))
	#define stdc_leading_zeros_ull(x) __builtin_stdc_leading_zeros((unsigned long long)(x))

	#define stdc_leading_ones(x) __builtin_stdc_leading_ones(x)
	#define stdc_leading_ones_uc(x) __builtin_stdc_leading_ones((unsigned char)(x))
	#define stdc_leading_ones_us(x) __builtin_stdc_leading_ones((unsigned short)(x))
	#define stdc_leading_ones_ui(x) __builtin_stdc_leading_ones((unsigned int)(x))
	#define stdc_leading_ones_ul(x) __builtin_stdc_leading_ones((unsigned long)(x))
	#define stdc_leading_ones_ull(x) __builtin_stdc_leading_ones((unsigned long long)(x))

	#define stdc_trailing_zeros(x) __builtin_stdc_trailing_zeros(x)
	#define stdc_trailing_zeros_uc(x) __builtin_stdc_trailing_zeros((unsigned char)(x))
	#define stdc_trailing_zeros_us(x) __builtin_stdc_trailing_zeros((unsigned short)(x))
	#define stdc_trailing_zeros_ui(x) __builtin_stdc_trailing_zeros((unsigned int)(x))
	#define stdc_trailing_zeros_ul(x) __builtin_stdc_trailing_zeros((unsigned long)(x))
	#define stdc_trailing_zeros_ull(x) __builtin_stdc_trailing_zeros((unsigned long long)(x))

	#define stdc_trailing_ones(x) __builtin_stdc_trailing_ones(x)
	#define stdc_trailing_ones_uc(x) __builtin_stdc_trailing_ones((unsigned char)(x))
	#define stdc_trailing_ones_us(x) __builtin_stdc_trailing_ones((unsigned short)(x))
	#define stdc_trailing_ones_ui(x) __builtin_stdc_trailing_ones((unsigned int)(x))
	#define stdc_trailing_ones_ul(x) __builtin_stdc_trailing_ones((unsigned long)(x))
	#define stdc_trailing_ones_ull(x) __builtin_stdc_trailing_ones((unsigned long long)(x))

	#define stdc_first_leading_zero(x) __builtin_stdc_first_leading_zero(x)
	#define stdc_first_leading_zero_uc(x) __builtin_stdc_first_leading_zero((unsigned char)(x))
	#define stdc_first_leading_zero_us(x) __builtin_stdc_first_leading_zero((unsigned short)(x))
	#define stdc_first_leading_zero_ui(x) __builtin_stdc_first_leading_zero((unsigned int)(x))
	#define stdc_first_leading_zero_ul(x) __builtin_stdc_first_leading_zero((unsigned long)(x))
	#define stdc_first_leading_zero_ull(x) __builtin_stdc_first_leading_zero((unsigned long long)(x))

	#define stdc_first_leading_one(x) __builtin_stdc_first_leading_one(x)
	#define stdc_first_leading_one_uc(x) __builtin_stdc_first_leading_one((unsigned char)(x))
	#define stdc_first_leading_one_us(x) __builtin_stdc_first_leading_one((unsigned short)(x))
	#define stdc_first_leading_one_ui(x) __builtin_stdc_first_leading_one((unsigned int)(x))
	#define stdc_first_leading_one_ul(x) __builtin_stdc_first_leading_one((unsigned long)(x))
	#define stdc_first_leading_one_ull(x) __builtin_stdc_first_leading_one((unsigned long long)(x))

	#define stdc_first_trailing_zero(x) __builtin_stdc_first_trailing_zero(x)
	#define stdc_first_trailing_zero_uc(x) __builtin_stdc_first_trailing_zero((unsigned char)(x))
	#define stdc_first_trailing_zero_us(x) __builtin_stdc_first_trailing_zero((unsigned short)(x))
	#define stdc_first_trailing_zero_ui(x) __builtin_stdc_first_trailing_zero((unsigned int)(x))
	#define stdc_first_trailing_zero_ul(x) __builtin_stdc_first_trailing_zero((unsigned long)(x))
	#define stdc_first_trailing_zero_ull(x) __builtin_stdc_first_trailing_zero((unsigned long long)(x))

	#define stdc_first_trailing_one(x) __builtin_stdc_first_trailing_one(x)
	#define stdc_first_trailing_one_uc(x) __builtin_stdc_first_trailing_one((unsigned char)(x))
	#define stdc_first_trailing_one_us(x) __builtin_stdc_first_trailing_one((unsigned short)(x))
	#define stdc_first_trailing_one_ui(x) __builtin_stdc_first_trailing_one((unsigned int)(x))
	#define stdc_first_trailing_one_ul(x) __builtin_stdc_first_trailing_one((unsigned long)(x))
	#define stdc_first_trailing_one_ull(x) __builtin_stdc_first_trailing_one((unsigned long long)(x))

	#define stdc_count_zeros(x) __builtin_stdc_count_zeros(x)
	#define stdc_count_zeros_uc(x) __builtin_stdc_count_zeros((unsigned char)(x))
	#define stdc_count_zeros_us(x) __builtin_stdc_count_zeros((unsigned short)(x))
	#define stdc_count_zeros_ui(x) __builtin_stdc_count_zeros((unsigned int)(x))
	#define stdc_count_zeros_ul(x) __builtin_stdc_count_zeros((unsigned long)(x))
	#define stdc_count_zeros_ull(x) __builtin_stdc_count_zeros((unsigned long long)(x))

	#define stdc_count_ones(x) __builtin_stdc_count_ones(x)
	#define stdc_count_ones_uc(x) __builtin_stdc_count_ones((unsigned char)(x))
	#define stdc_count_ones_us(x) __builtin_stdc_count_ones((unsigned short)(x))
	#define stdc_count_ones_ui(x) __builtin_stdc_count_ones((unsigned int)(x))
	#define stdc_count_ones_ul(x) __builtin_stdc_count_ones((unsigned long)(x))
	#define stdc_count_ones_ull(x) __builtin_stdc_count_ones((unsigned long long)(x))

	#define stdc_has_single_bit(x) __builtin_stdc_has_single_bit(x)
	#define stdc_has_single_bit_uc(x) __builtin_stdc_has_single_bit((unsigned char)(x))
	#define stdc_has_single_bit_us(x) __builtin_stdc_has_single_bit((unsigned short)(x))
	#define stdc_has_single_bit_ui(x) __builtin_stdc_has_single_bit((unsigned int)(x))
	#define stdc_has_single_bit_ul(x) __builtin_stdc_has_single_bit((unsigned long)(x))
	#define stdc_has_single_bit_ull(x) __builtin_stdc_has_single_bit((unsigned long long)(x))

	#define stdc_bit_width(x) __builtin_stdc_bit_width(x)
	#define stdc_bit_width_uc(x) __builtin_stdc_bit_width((unsigned char)(x))
	#define stdc_bit_width_us(x) __builtin_stdc_bit_width((unsigned short)(x))
	#define stdc_bit_width_ui(x) __builtin_stdc_bit_width((unsigned int)(x))
	#define stdc_bit_width_ul(x) __builtin_stdc_bit_width((unsigned long)(x))
	#define stdc_bit_width_ull(x) __builtin_stdc_bit_width((unsigned long long)(x))

	#define stdc_bit_floor(x) __builtin_stdc_bit_floor(x)
	#define stdc_bit_floor_uc(x) __builtin_stdc_bit_floor((unsigned char)(x))
	#define stdc_bit_floor_us(x) __builtin_stdc_bit_floor((unsigned short)(x))
	#define stdc_bit_floor_ui(x) __builtin_stdc_bit_floor((unsigned int)(x))
	#define stdc_bit_floor_ul(x) __builtin_stdc_bit_floor((unsigned long)(x))
	#define stdc_bit_floor_ull(x) __builtin_stdc_bit_floor((unsigned long long)(x))

	#define stdc_bit_ceil(x) __builtin_stdc_bit_ceil(x)
	#define stdc_bit_ceil_uc(x) __builtin_stdc_bit_ceil((unsigned char)(x))
	#define stdc_bit_ceil_us(x) __builtin_stdc_bit_ceil((unsigned short)(x))
	#define stdc_bit_ceil_ui(x) __builtin_stdc_bit_ceil((unsigned int)(x))
	#define stdc_bit_ceil_ul(x) __builtin_stdc_bit_ceil((unsigned long)(x))
	#define stdc_bit_ceil_ull(x) __builtin_stdc_bit_ceil((unsigned long long)(x))
#endif

#endif /* __STDC_VERSION_STDBIT_H__ */
