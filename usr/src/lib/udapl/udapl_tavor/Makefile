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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/lib/Makefile.lib

ROOTUSR =	$(ROOT)/usr
ROOTUSRSHARE =	$(ROOTUSR)/share
ROOTUSRSHAREDAT =	$(ROOTUSRSHARE)/dat
CONFFILES =	SUNWudaplt.conf
ROOTCONFFILES =	$(CONFFILES:%=$(ROOTUSRSHAREDAT)/%)

$(BUILD32)SUBDIRS = $(MACH)
$(BUILD64)SUBDIRS += $(MACH64)

all :=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber
debug :=	TARGET= debug

.KEEP_STATE:

all debug install: $(ROOTCONFFILES) .WAIT $(SUBDIRS)

$(ROOTCONFFILES): $(ROOTUSRSHAREDAT)

clean clobber: $(SUBDIRS)

install_h:

check:

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

$(ROOTUSRSHAREDAT)/%: %
	$(INS.file)

$(ROOTUSRSHAREDAT):
	$(INS.dir)

include $(SRC)/lib/Makefile.targ

FRC:

