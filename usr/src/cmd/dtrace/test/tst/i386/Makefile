#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

CSRCS =				\
	pid/err.uregswrite1.c	\
	pid/err.uregswrite2.c	\
	pid/tst.uregswrite.c	\
	ustack/tst.annotated.c	\
	ustack/tst.helper.c

SSRCS =				\
	pid/tst.badinstr.S	\
	pid/tst.branch.S	\
	pid/tst.embedded.S	\
	pid/tst.ret.S		\
	pid/tst.retlist.S	\
	ustack/tst.circstack.S

DSRCS =					\
	arrays/tst.uregsarray.d		\
	error/tst.DTRACEFLT_DIVZERO.d	\
	funcs/tst.badcopyin.d		\
	funcs/tst.badcopyinstr.d	\
	funcs/tst.badcopyout.d		\
	funcs/tst.badcopyoutstr.d	\
	pid/err.uregswrite1.d		\
	pid/err.uregswrite2.d		\
	pid/tst.badinstr.d		\
	pid/tst.branch.d		\
	pid/tst.embedded.d		\
	pid/tst.ret.d			\
	pid/tst.uregswrite.d		\
	ustack/annotated_helper.d	\
	ustack/helper_helper.d		\
	ustack/tst.annotated.d		\
	ustack/tst.circstack.d		\
	ustack/tst.helper.d

TSTS =					\
	arrays/tst.uregsarray.d		\
	error/tst.DTRACEFLT_DIVZERO.d	\
	funcs/tst.badcopyin.d		\
	funcs/tst.badcopyinstr.d	\
	funcs/tst.badcopyout.d		\
	funcs/tst.badcopyoutstr.d	\
	pid/err.uregswrite1.d		\
	pid/err.uregswrite2.d		\
	pid/tst.badinstr.d		\
	pid/tst.branch.d		\
	pid/tst.embedded.d		\
	pid/tst.ret.d			\
	pid/tst.retlist.ksh		\
	pid/tst.uregswrite.d		\
	ustack/tst.annotated.d		\
	ustack/tst.annotated.d.out	\
	ustack/tst.circstack.d		\
	ustack/tst.helper.d		\
	ustack/tst.helper.d.out

include ../Makefile.com

ASFLAGS += -D_ASM $(CPPFLAGS)
CLOBBER_FILES += helper_helper.o annotated_helper.o

# For the helper test we require an executable data section.
ustack/tst.helper.exe := MAPFILE.NED=

ustack/tst.helper.exe: ustack/tst.helper.o ustack/helper_helper.o
	$(LINK.c) -o ustack/tst.helper.exe \
		 ustack/tst.helper.o ustack/helper_helper.o $(LDLIBS)
	$(POST_PROCESS) ; $(STRIP_STABS)

ustack/helper_helper.o: ustack/helper_helper.d
	$(COMPILE.d) -o ustack/helper_helper.o -s ustack/helper_helper.d

ustack/tst.annotated.exe: ustack/tst.annotated.o ustack/annotated_helper.o
	$(LINK.c) -o ustack/tst.annotated.exe  \
		 ustack/tst.annotated.o ustack/annotated_helper.o $(LDLIBS)
	$(POST_PROCESS) ; $(STRIP_STABS)

ustack/annotated_helper.o: ustack/annotated_helper.d
	$(COMPILE.d) -o ustack/annotated_helper.o -s ustack/annotated_helper.d
