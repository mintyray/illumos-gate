#include <sys/cdefs.h>
#include <limits.h>
__FBSDID("$FreeBSD$");

#define _type		float
#define	roundit		roundf
#define dtype		long
#define	DTYPE_MIN	LONG_MIN
#define	DTYPE_MAX	LONG_MAX
#define	fn		lroundf

#include "s_lround.c"
