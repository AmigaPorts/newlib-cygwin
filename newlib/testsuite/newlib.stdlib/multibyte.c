/* Without _MB_CAPABLE a byte is the character with the same code: one above
   0x7f must not sign-extend, a wide character above 0xff has no multibyte
   form, and a null target only counts.  */

#include <errno.h>
#include <newlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static int failures;

static void check(int ok, const char *what)
{
  if (!ok)
    {
      printf ("FAIL: %s\n", what);
      failures++;
    }
}

#define CHECK(cond) check (cond, #cond)

int main (void)
{
#ifndef _MB_CAPABLE
  static const wchar_t latin1[] = { 0xe4, 'b', 0 };
  static const wchar_t euro[] = { 'a', 0x20ac, 0 };
  wchar_t w = 0, ws[8];
  char b[8];
  int i;

  CHECK (mbtowc (&w, "\xe4", 1) == 1 && w == 0xe4);
  CHECK (mbtowc (&w, "\xff", 1) == 1 && w == 0xff);
  CHECK (mbtowc (&w, "", 1) == 0 && w == 0);

  for (i = 0; i < 8; i++)
    ws[i] = 0x55;
  CHECK (mbstowcs (ws, "\xe4\xff", 8) == 2 && ws[0] == 0xe4 && ws[1] == 0xff && ws[2] == 0);
  CHECK (mbstowcs (NULL, "abcd", 0) == 4);

  CHECK (wctomb (b, 0xe4) == 1 && (unsigned char) b[0] == 0xe4);
  errno = 0;
  CHECK (wctomb (b, 0x20ac) == -1 && errno == EILSEQ);

  memset (b, 0x55, sizeof (b));
  CHECK (wcstombs (b, latin1, 8) == 2 && (unsigned char) b[0] == 0xe4 && b[2] == 0);
  CHECK (wcstombs (NULL, latin1, 0) == 2);
  errno = 0;
  CHECK (wcstombs (b, euro, 8) == (size_t) -1 && errno == EILSEQ);
#endif
  return failures != 0;
}
