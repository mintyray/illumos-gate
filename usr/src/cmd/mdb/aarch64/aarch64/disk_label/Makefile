#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#
# Copyright (c) 2019, Joyent, Inc.
#

MODULE = disk_label.so
MDBTGT = raw

MODSRCS = disk_label.c

include ../../../../Makefile.cmd
include ../../../../Makefile.cmd.64
include ../../Makefile.aarch64
include ../../../Makefile.module

MODSRCS_DIR = ../../../common/modules/disk_label

CPPFLAGS += -I$(SRC)/uts/common -I$(SRC)/cmd/boot/installboot/i386

LDLIBS += -luuid
