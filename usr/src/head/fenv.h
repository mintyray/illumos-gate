/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright 2017 Hayashi Naoyuki
 */

#ifndef _FENV_H
#define	_FENV_H

#include <sys/feature_tests.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Rounding modes
 */
#if defined(__sparc)

#define	FE_TONEAREST	0
#define	FE_TOWARDZERO	1
#define	FE_UPWARD	2
#define	FE_DOWNWARD	3

#elif defined(__i386) || defined(__amd64)

#define	FE_TONEAREST	0
#define	FE_DOWNWARD	1
#define	FE_UPWARD	2
#define	FE_TOWARDZERO	3

#elif defined(__aarch64__)

#define	FE_TONEAREST	0
#define	FE_UPWARD	1
#define	FE_DOWNWARD	2
#define	FE_TOWARDZERO	3

#else

#error Unknown platform

#endif

extern int fegetround(void);
extern int fesetround(int);

#if (defined(__i386) || defined(__amd64)) && \
	(!defined(_STRICT_STDC) || defined(__EXTENSIONS__))

#define	FE_FLTPREC	0
#define	FE_DBLPREC	2
#define	FE_LDBLPREC	3

extern int fegetprec(void);
extern int fesetprec(int);

#endif

/*
 * Exception flags
 */
#if defined(__sparc)

#define	FE_INEXACT	0x01
#define	FE_DIVBYZERO	0x02
#define	FE_UNDERFLOW	0x04
#define	FE_OVERFLOW	0x08
#define	FE_INVALID	0x10
#define	FE_ALL_EXCEPT	0x1f

#elif defined(__i386) || defined(__amd64)

#define	FE_INVALID	0x01
#define	FE_DIVBYZERO	0x04
#define	FE_OVERFLOW	0x08
#define	FE_UNDERFLOW	0x10
#define	FE_INEXACT	0x20
#define	FE_ALL_EXCEPT	0x3d

#elif defined(__aarch64__)
#define	FE_INVALID	0x0001
#define	FE_DIVBYZERO	0x0002
#define	FE_OVERFLOW	0x0004
#define	FE_UNDERFLOW	0x0008
#define	FE_INEXACT	0x0010
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | \
	FE_OVERFLOW | FE_UNDERFLOW)

#else

#error Unknown ISA

#endif

typedef int fexcept_t;

extern int feclearexcept(int);
extern int feraiseexcept(int);
extern int fetestexcept(int);
extern int fegetexceptflag(fexcept_t *, int);
extern int fesetexceptflag(const fexcept_t *, int);

#if !defined(_STRICT_STDC) || defined(__EXTENSIONS__)

/*
 * Exception handling extensions
 */
#define	FEX_NOHANDLER	-1
#define	FEX_NONSTOP	0
#define	FEX_ABORT	1
#define	FEX_SIGNAL	2
#define	FEX_CUSTOM	3

#define	FEX_INEXACT	0x001
#define	FEX_DIVBYZERO	0x002
#define	FEX_UNDERFLOW	0x004
#define	FEX_OVERFLOW	0x008
#define	FEX_INV_ZDZ	0x010
#define	FEX_INV_IDI	0x020
#define	FEX_INV_ISI	0x040
#define	FEX_INV_ZMI	0x080
#define	FEX_INV_SQRT	0x100
#define	FEX_INV_SNAN	0x200
#define	FEX_INV_INT	0x400
#define	FEX_INV_CMP	0x800
#define	FEX_INVALID	0xff0
#define	FEX_COMMON	(FEX_INVALID | FEX_DIVBYZERO | FEX_OVERFLOW)
#define	FEX_ALL		(FEX_COMMON | FEX_UNDERFLOW | FEX_INEXACT)
#define	FEX_NONE	0

#define	FEX_NUM_EXC	12

/* structure to hold a numeric value in any format used by the FPU */
typedef struct {
	enum fex_nt {
		fex_nodata	= 0,
		fex_int		= 1,
		fex_llong	= 2,
		fex_float	= 3,
		fex_double	= 4,
		fex_ldouble	= 5
	} type;
	union {
		int		i;
#if !defined(_STRICT_STDC) && !defined(_NO_LONGLONG) || defined(_STDC_C99) || \
	defined(__C99FEATURES__)
		long long	l;
#else
		struct {
			int	l[2];
		} l;
#endif
		float		f;
		double		d;
		long double	q;
	} val;
} fex_numeric_t;

/* structure to supply information about an exception to a custom handler */
typedef struct {
	enum fex_op {
		fex_add		= 0,
		fex_sub		= 1,
		fex_mul		= 2,
		fex_div		= 3,
		fex_sqrt	= 4,
		fex_cnvt	= 5,
		fex_cmp		= 6,
		fex_other	= 7
	} op;			/* operation that caused the exception */
	int		flags;	/* flags to be set */
	fex_numeric_t	op1, op2, res;	/* operands and result */
} fex_info_t;

typedef struct fex_handler_data {
	int	__mode;
	void	(*__handler)();
} fex_handler_t[FEX_NUM_EXC];

extern int fex_get_handling(int);
extern int fex_set_handling(int, int, void (*)());

extern void fex_getexcepthandler(fex_handler_t *, int);
extern void fex_setexcepthandler(const fex_handler_t *, int);

#ifdef __STDC__
#include <stdio_tag.h>
#ifndef	_FILEDEFED
#define	_FILEDEFED
typedef	__FILE FILE;
#endif
#endif
extern FILE *fex_get_log(void);
extern int fex_set_log(FILE *);
extern int fex_get_log_depth(void);
extern int fex_set_log_depth(int);
extern void fex_log_entry(const char *);

#define	__fex_handler_t	fex_handler_t

#else

typedef struct {
	int	__mode;
	void	(*__handler)();
} __fex_handler_t[12];

#endif /* !defined(_STRICT_STDC) || defined(__EXTENSIONS__) */

/*
 * Environment as a whole
 */
typedef struct {
	__fex_handler_t	__handlers;
	unsigned long	__fsr;
} fenv_t;

#ifdef __STDC__
extern const fenv_t __fenv_dfl_env;
#else
extern fenv_t __fenv_dfl_env;
#endif

#define	FE_DFL_ENV	(&__fenv_dfl_env)

extern int fegetenv(fenv_t *);
extern int fesetenv(const fenv_t *);
extern int feholdexcept(fenv_t *);
extern int feupdateenv(const fenv_t *);

#if !defined(_STRICT_STDC) || defined(__EXTENSIONS__)
extern void fex_merge_flags(const fenv_t *);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _FENV_H */
