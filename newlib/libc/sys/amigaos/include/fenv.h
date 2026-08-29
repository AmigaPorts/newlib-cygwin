/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 AmigaDev
 */

#ifndef _FENV_H
#define _FENV_H

typedef unsigned int fexcept_t;

#if defined(__HAVE_68881__)

/* FPSR accrued exception byte.  */
#define FE_INEXACT   (1 << 3)
#define FE_DIVBYZERO (1 << 4)
#define FE_UNDERFLOW (1 << 5)
#define FE_OVERFLOW  (1 << 6)
#define FE_INVALID   (1 << 7)
#define FE_ALL_EXCEPT \
  (FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* FPCR rounding mode field.  */
#define FE_TONEAREST  0
#define FE_TOWARDZERO (1 << 4)
#define FE_DOWNWARD   (2 << 4)
#define FE_UPWARD     (3 << 4)

typedef struct
{
  unsigned int __control_register;
  unsigned int __status_register;
  unsigned int __instruction_address;
} fenv_t;

#else

/* Soft-float multilibs have no floating-point status or exceptions.  */
#define FE_ALL_EXCEPT 0
#define FE_TONEAREST  0

typedef struct
{
  unsigned int __unused;
} fenv_t;

#endif

#define FE_DFL_ENV ((const fenv_t *) -1)

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__HAVE_68881__)

static __inline__ unsigned int
__amiga_fenv_get_fpcr(void)
{
  unsigned int value;
  __asm__ ("fmove%.l %/fpcr,%0" : "=dm" (value));
  return value;
}

static __inline__ void
__amiga_fenv_set_fpcr(unsigned int value)
{
  __asm__ __volatile__ ("fmove%.l %0,%/fpcr" : : "dm" (value));
}

static __inline__ unsigned int
__amiga_fenv_get_fpsr(void)
{
  unsigned int value;
  __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (value));
  return value;
}

static __inline__ void
__amiga_fenv_set_fpsr(unsigned int value)
{
  __asm__ __volatile__ ("fmove%.l %0,%/fpsr" : : "dm" (value));
}

static __inline__ unsigned int
__amiga_fenv_get_fpiar(void)
{
  unsigned int value;
  __asm__ ("fmove%.l %/fpiar,%0" : "=dm" (value));
  return value;
}

static __inline__ void
__amiga_fenv_set_fpiar(unsigned int value)
{
  __asm__ __volatile__ ("fmove%.l %0,%/fpiar" : : "dm" (value));
}

static __inline__ int
feclearexcept(int excepts)
{
  unsigned int fpsr = __amiga_fenv_get_fpsr();
  fpsr &= ~(excepts & FE_ALL_EXCEPT);
  __amiga_fenv_set_fpsr(fpsr);
  return 0;
}

static __inline__ int
fegetexceptflag(fexcept_t *flagp, int excepts)
{
  *flagp = __amiga_fenv_get_fpsr() & excepts & FE_ALL_EXCEPT;
  return 0;
}

static __inline__ int
feraiseexcept(int excepts)
{
  unsigned int fpsr = __amiga_fenv_get_fpsr();
  fpsr |= excepts & FE_ALL_EXCEPT;
  __amiga_fenv_set_fpsr(fpsr);
  return 0;
}

static __inline__ int
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  unsigned int mask = excepts & FE_ALL_EXCEPT;
  unsigned int fpsr = __amiga_fenv_get_fpsr();
  fpsr = (fpsr & ~mask) | (*flagp & mask);
  __amiga_fenv_set_fpsr(fpsr);
  return 0;
}

static __inline__ int
fetestexcept(int excepts)
{
  return __amiga_fenv_get_fpsr() & excepts & FE_ALL_EXCEPT;
}

static __inline__ int
fegetround(void)
{
  return __amiga_fenv_get_fpcr() & FE_UPWARD;
}

static __inline__ int
fesetround(int round)
{
  unsigned int fpcr;

  if ((round & ~FE_UPWARD) != 0)
    return 1;

  fpcr = __amiga_fenv_get_fpcr();
  fpcr = (fpcr & ~FE_UPWARD) | (unsigned int) round;
  __amiga_fenv_set_fpcr(fpcr);
  return 0;
}

static __inline__ int
fegetenv(fenv_t *envp)
{
  envp->__control_register = __amiga_fenv_get_fpcr();
  envp->__status_register = __amiga_fenv_get_fpsr();
  envp->__instruction_address = __amiga_fenv_get_fpiar();
  return 0;
}

static __inline__ int
feholdexcept(fenv_t *envp)
{
  unsigned int fpcr;
  unsigned int fpsr;

  fegetenv(envp);
  fpcr = envp->__control_register & ~0x0000ff00U;
  fpsr = envp->__status_register & ~FE_ALL_EXCEPT;
  __amiga_fenv_set_fpcr(fpcr);
  __amiga_fenv_set_fpsr(fpsr);
  return 0;
}

static __inline__ int
fesetenv(const fenv_t *envp)
{
  unsigned int fpcr = 0;
  unsigned int fpsr = 0;
  unsigned int fpiar = 0;

  if (envp != FE_DFL_ENV)
    {
      fpcr = envp->__control_register;
      fpsr = envp->__status_register;
      fpiar = envp->__instruction_address;
    }

  /* Restore status before re-enabling any exception traps.  */
  __amiga_fenv_set_fpsr(fpsr);
  __amiga_fenv_set_fpiar(fpiar);
  __amiga_fenv_set_fpcr(fpcr);
  return 0;
}

static __inline__ int
feupdateenv(const fenv_t *envp)
{
  int excepts = fetestexcept(FE_ALL_EXCEPT);
  fesetenv(envp);
  return feraiseexcept(excepts);
}

#else

static __inline__ int feclearexcept(int excepts) { (void) excepts; return 0; }
static __inline__ int fegetexceptflag(fexcept_t *flagp, int excepts)
{ (void) excepts; *flagp = 0; return 0; }
static __inline__ int feraiseexcept(int excepts) { (void) excepts; return 0; }
static __inline__ int fesetexceptflag(const fexcept_t *flagp, int excepts)
{ (void) flagp; (void) excepts; return 0; }
static __inline__ int fetestexcept(int excepts) { (void) excepts; return 0; }
static __inline__ int fegetround(void) { return FE_TONEAREST; }
static __inline__ int fesetround(int round) { return round != FE_TONEAREST; }
static __inline__ int fegetenv(fenv_t *envp) { envp->__unused = 0; return 0; }
static __inline__ int feholdexcept(fenv_t *envp) { return fegetenv(envp); }
static __inline__ int fesetenv(const fenv_t *envp) { (void) envp; return 0; }
static __inline__ int feupdateenv(const fenv_t *envp) { return fesetenv(envp); }

#endif

#ifdef __cplusplus
}
#endif

#endif /* _FENV_H */
