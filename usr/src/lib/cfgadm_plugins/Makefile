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
# Copyright (c) 1998, 2010, Oracle and/or its affiliates. All rights reserved.
# Copyright 2019 Joyent, Inc.
#

include $(SRC)/Makefile.master

COMMON_SUBDIRS= scsi pci usb ib fp shp ccid
$(NOT_AARCH64_BLD)COMMON_SUBDIRS += sbd
sparc_SUBDIRS=	ac sysctrl
i386_SUBDIRS= sata

SUBDIRS= $(COMMON_SUBDIRS) $($(MACH)_SUBDIRS)

ALL_SUBDIRS= $(COMMON_SUBDIRS) $(sparc_SUBDIRS) $(i386_SUBDIRS)

MSGSUBDIRS= $(ALL_SUBDIRS)

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber
_msg:=		TARGET= _msg

.KEEP_STATE:

all clean: $(SUBDIRS)


install: all $(SUBDIRS)

_msg clobber: $(MSGSUBDIRS)

$(ALL_SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

FRC:
