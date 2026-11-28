/* Exercise both the C library division functions and the compiler's
   arithmetic: on the 68000, gcc calls libgcc's __mulsi3, __udivsi3,
   __umodsi3, __divsi3 and __modsi3 for *, / and % of volatile operands.
   Referencing both must not introduce duplicate div/ldiv symbols.  */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    static const struct { int numerator, denominator, quotient, remainder; } cases[] = {
        { 73, 7, 10, 3 }, { -73, 7, -10, -3 },
        { 73, -7, -10, 3 }, { -73, -7, 10, -3 },
        { 1, 2, 0, 1 }, { -1, 2, 0, -1 },
        { 0, 7, 0, 0 }, { INT_MIN, 1, INT_MIN, 0 },
        { INT_MAX, -1, -INT_MAX, 0 }, { INT_MIN, 3, -715827882, -2 },
        { INT_MIN, -3, 715827882, -2 }, { INT_MAX, 3, 715827882, 1 },
        { INT_MIN, INT_MIN, 1, 0 }, { INT_MAX, INT_MIN, 0, INT_MAX }
    };
    unsigned checks = 0, failures = 0;
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        volatile int n = cases[i].numerator, d = cases[i].denominator;
        div_t a = div(n, d);
        ldiv_t b = ldiv(n, d);
        ++checks;
        if (a.quot != cases[i].quotient || a.rem != cases[i].remainder)
            ++failures;
        ++checks;
        if (b.quot != cases[i].quotient || b.rem != cases[i].remainder)
            ++failures;
        ++checks;
        if (n / d != cases[i].quotient || n % d != cases[i].remainder)
            ++failures;
    }

    volatile unsigned u = UINT_MAX, v = 3;
    ++checks;
    if (u / v != 1431655765U || u % v)
        ++failures;
    u = 0x80000000U;
    v = 0x7fffffffU;
    ++checks;
    if (u / v != 1 || u % v != 1)
        ++failures;

    /* Divisors wider than 16 bits, and the last three, where a quotient
       estimated from the top 16 bits of the divisor is one too large. */
    static const struct { unsigned numerator, denominator, quotient, remainder; } wide[] = {
        { 0xfedcba98U, 123457U, 34634U, 0x10cceU },
        { 0xfedcba98U, 0x01234567U, 224U, 0x78U },
        { 0xfedcba98U, 0x40000001U, 3U, 0x3edcba95U },
        { 0x8000fffeU, 0x8000ffffU, 0U, 0x8000fffeU },
        { 0x8000fffdU, 0x40007fffU, 1U, 0x40007ffeU },
        { 0xfffffffeU, 0xffffffffU, 0U, 0xfffffffeU },
    };
    for (unsigned i = 0; i < sizeof wide / sizeof wide[0]; ++i) {
        u = wide[i].numerator;
        v = wide[i].denominator;
        ++checks;
        if (u / v != wide[i].quotient || u % v != wide[i].remainder)
            ++failures;
    }

    static const struct { unsigned a, b, product; } products[] = {
        { 0, 0xffffffffU, 0 }, { 1, 0x80000000U, 0x80000000U },
        { 0xffffU, 0xffffU, 0xfffe0001U }, { 0x12345678U, 0x9abcdefU, 0xe242d208U },
        { 0xffffffffU, 0xffffffffU, 1 }, { 123457U, 34640U, 0xfee6fb50U },
    };
    for (unsigned i = 0; i < sizeof products / sizeof products[0]; ++i) {
        u = products[i].a;
        v = products[i].b;
        ++checks;
        if (u * v != products[i].product)
            ++failures;
    }

    /* mktime() also references the C library div() implementation. */
    setenv("TZ", "UTC0", 1);
    tzset();
    struct tm date = { 0 };
    date.tm_year = 70;
    date.tm_mday = 2;
    time_t stamp = mktime(&date);
    ++checks;
    if (stamp != 86400 || date.tm_year != 70 || date.tm_mon || date.tm_mday != 2)
        ++failures;
    printf("Integer division: %u checks, %u failures; mktime=%ld\n", checks, failures, (long)stamp);
    return failures ? 20 : 0;
}
