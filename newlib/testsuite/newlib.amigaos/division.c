/* Exercise both the C library division functions and compiler helpers.
   Referencing both must not introduce duplicate div/ldiv symbols.  */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern int __divsi3(int, int);
extern int __modsi3(int, int);
extern unsigned __udivsi3(unsigned, unsigned);
extern unsigned __umodsi3(unsigned, unsigned);
extern unsigned __mulsi3(unsigned, unsigned);

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
    div_t (*volatile division)(int, int) = div;
    ldiv_t (*volatile longDivision)(long, long) = ldiv;
    unsigned checks = 0, failures = 0;
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        div_t a = division(cases[i].numerator, cases[i].denominator);
        ldiv_t b = longDivision(cases[i].numerator, cases[i].denominator);
        ++checks;
        if (a.quot != cases[i].quotient || a.rem != cases[i].remainder)
            ++failures;
        ++checks;
        if (b.quot != cases[i].quotient || b.rem != cases[i].remainder)
            ++failures;
        ++checks;
        if (__divsi3(cases[i].numerator, cases[i].denominator) != cases[i].quotient
            || __modsi3(cases[i].numerator, cases[i].denominator) != cases[i].remainder)
            ++failures;
    }
    ++checks;
    if (__udivsi3(UINT_MAX, 3) != 1431655765U || __umodsi3(UINT_MAX, 3))
        ++failures;
    ++checks;
    if (__udivsi3(0x80000000U, 0x7fffffffU) != 1 || __umodsi3(0x80000000U, 0x7fffffffU) != 1)
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
        ++checks;
        if (__udivsi3(wide[i].numerator, wide[i].denominator) != wide[i].quotient
            || __umodsi3(wide[i].numerator, wide[i].denominator) != wide[i].remainder)
            ++failures;
    }

    static const struct { unsigned a, b, product; } products[] = {
        { 0, 0xffffffffU, 0 }, { 1, 0x80000000U, 0x80000000U },
        { 0xffffU, 0xffffU, 0xfffe0001U }, { 0x12345678U, 0x9abcdefU, 0xe242d208U },
        { 0xffffffffU, 0xffffffffU, 1 }, { 123457U, 34640U, 0xfee6fb50U },
    };
    for (unsigned i = 0; i < sizeof products / sizeof products[0]; ++i) {
        ++checks;
        if (__mulsi3(products[i].a, products[i].b) != products[i].product)
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
