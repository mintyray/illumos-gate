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
# Copyright 2017 Hayashi Naoyuki
# Copyright 2014 Gary Mills
#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include Makefile.com

i386_SUBDIRS =
sparc_SUBDIRS =
aarch64_SUBDIRS = libfdt fs/zfs
SUBDIRS = fs/hsfs fs/nfs fs/ufs inet sa sock tcpstubs xdr \
	tcp $($(MACH)_SUBDIRS)

.PARALLEL: $(SUBDIRS)

.KEEP_STATE:

all	:=	TARGET = all
clean	:=	TARGET = clean
clobber	:=	TARGET = clobber
install	:=	TARGET = install

all clean clobber install: $(SUBDIRS)

$(SUBDIRS): FRC
	cd $@; pwd; $(MAKE) $(TARGET)

FRC:

#
# Cross-reference customization: set the include path to be reasonable.
#
XRINCDIRS = $(CPPINCS:-YI,%=-I%)
