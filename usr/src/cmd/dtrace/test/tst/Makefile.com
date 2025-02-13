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
# Copyright (c) 2018, Joyent, Inc.

include $(SRC)/cmd/Makefile.cmd

.KEEP_STATE:

ROOTOPTPKG = $(ROOT)/opt/SUNWdtrt
ROOTTST = $(ROOTOPTPKG)/tst
SUBDIR :sh= basename `pwd`
TSTDIR = $(ROOTTST)/$(SUBDIR)
DSTYLE = $(ROOTOPTPKG)/bin/dstyle

EXES += $(CSRCS:%.c=%.exe)
EXES += $(SSRCS:%.S=%.exe)

ROOT_TSTS = $(TSTS:%=$(TSTDIR)/%)
ROOT_EXES = $(EXES:%=$(TSTDIR)/%)

$(ROOT_TSTS) := FILEMODE = 0444
$(ROOT_EXES) := FILEMODE = 0555

# The DTrace tests rely on "normal" behaviour from the compiler which
# agressive optimization of small, simple, one compilation-unit programs may
# utterly subvert.  We force the compiler to not optimize rather than engage
# in an arms race with increasingly belligerent optimizers.
COPTFLAG=	-xO0

CERRWARN +=	-_gcc=-Wno-switch
CERRWARN +=	-_gcc=-Wno-unused-variable
CERRWARN +=	-_gcc=-Wno-implicit-function-declaration
CERRWARN +=	-_gcc=-Wno-unused-function
CERRWARN +=	-_gcc=-Wno-unused-variable

# not linted
SMATCH=off

all: $(EXES)

clean lint:

clobber: FRC
	-$(RM) $(CSRCS:%.c=%.exe) $(CSRCS:%.c=%.o)
	-$(RM) $(SSRCS:%.S=%.exe) $(SSRCS:%.S=%.o)
	-$(RM) $(DSRCS:%.d=%.o)
	-$(RM) $(CLOBBERFILES)

install: $(ROOT_TSTS) $(ROOT_EXES)

$(ROOT_TSTS): $(TSTDIR)

$(ROOT_EXES): $(TSTDIR)

$(TSTDIR):
	$(INS.dir)

$(TSTDIR)/%: %
	$(INS) -s -d -m $(DIRMODE) $(@D)
	$(INS.file)

%.exe: %.c
	$(LINK.c) -o $@ $< $(LDLIBS)
	$(POST_PROCESS) ; $(STRIP_STABS)

%.exe: %.o
	$(LINK.c) -o $@ $< $(LDLIBS)
	$(POST_PROCESS) ; $(STRIP_STABS)

%.o: %.c
	$(COMPILE.c) -o $@ $<
	$(POST_PROCESS_O)

%.o: %.S
	$(COMPILE.s) -o $@ $<
	$(POST_PROCESS_O)

scripts: FRC
	@cd ../cmd/scripts; pwd; $(MAKE) install

DCHECKS = $(DSRCS:%.d=%.dcheck)

%.dcheck: %.d
	@$(ECHO) "checking $<"; $(DSTYLE) $<

dstyle: $(DCHECKS)

FRC:
