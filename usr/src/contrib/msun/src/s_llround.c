#include <sys/cdefs.h>
#include <limits.h>
__FBSDID("$FreeBSD$");

#define _type		double
#define	roundit		round
#define dtype		long long
#define	DTYPE_MIN	LLONG_MIN
#define	DTYPE_MAX	LLONG_MAX
#define	fn		llround

#include "s_lround.c"
