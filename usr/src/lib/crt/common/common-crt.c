/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2016, Richard Lowe.
 * Copyright 2017 Hayashi Naoyuki
 * Copyright 2020 OmniOS Community Edition (OmniOSce) Association.
 */

/*
 * That of the CRT startup routine which itself may be implemented in C.
 */

#include <sys/feature_tests.h>
#include <sys/types.h>

#include <stdlib.h>
#include <synch.h>
#include <unistd.h>

#pragma weak _DYNAMIC
extern uintptr_t _DYNAMIC;

#pragma weak environ = _environ
char **_environ = NULL;
char **___Argv = NULL;

extern int main(int argc, char **argv, char **envp);
extern void _init(void);
extern void _fini(void);

#pragma weak __start_crt_compiler
extern int __start_crt_compiler(int argc, char **argv);

#if defined(__x86)
int __longdouble_used = 0;
#endif

#if defined(__x86) || defined(__aarch64__)
extern void __fpstart(void);
#endif

#if defined(__i386)		/* Not amd64 */
#pragma weak __fsr_init_value
extern long __fsr_init_value;
extern void __fsr(uintptr_t);
#endif


/*
 * Defined here for ABI reasons, must match the definition in libc.
 * If it cannot, a new symbol must be created.
 */
mutex_t __environ_lock = DEFAULTMUTEX;

void
_start_crt(int argc, char **argv, void (*exit_handler)(void))
{
	int ret = 0;

	/*
	 * On x86, we check whether we're a dynamic executable to see whether
	 * we'll receive an exit_handler.
	 *
	 * On SPARC, we just need to check whether the handler was NULL.
	 */
#if defined(__x86) || defined(__aarch64__)
	if (&_DYNAMIC != NULL)
		(void) atexit(exit_handler);
#elif defined(__sparc)
	if (exit_handler != NULL)
		(void) atexit(exit_handler);
#endif

	(void) atexit(_fini);

	_environ = argv + (argc + 1);
	___Argv = argv;

	/*
	 * __start_crt_compiler() provides a hook for compilers to perform
	 * extra initialisation before main() is called. For example, gcc uses
	 * this to initialise profiling code for objects built with its -pg
	 * option by linking in an extra object that provides this function.
	 *
	 * This mechanism replaces the earlier separate gcrt1.o.
	 *
	 * If __start_crt_compiler() returns a non-zero value, then the
	 * process will exit with that value, without main() being called.
	 */
	if (&__start_crt_compiler != NULL)
		ret = __start_crt_compiler(argc, argv);

	if (ret == 0) {
#if defined(__x86) || defined(__aarch64__)
		__fpstart();
#endif
#if defined(__i386)		/* Not amd64 */
		/*
		 * Note that Studio cc(1) sets the _value of the symbol_, that
		 * is, its address.  Not the value _at_ that address.
		 */
		__fsr((uintptr_t)&__fsr_init_value);
#endif
		_init();
		ret = main(argc, argv, _environ);
	}
	exit(ret);
	_exit(ret);
}
