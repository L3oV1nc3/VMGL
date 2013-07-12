# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SHARED = 1
LIBRARY = crutil
FILES = bbox \
	bufpool \
	calllists \
	devnull \
	dll \
	endian \
	environment \
	error \
	filenet \
	hash \
	hull \
	list \
	matrix \
	mem \
	net \
	pixel \
	process \
	rand \
	string \
	threads \
	tcpip \
	udptcpip \
	timer \
	url \
	warp
PRECOMP = debug_opcodes.c
SLOP += $(PRECOMP)

ifeq ($(ARCH), SunOS)
LDFLAGS= -lsocket, lnsl
endif

ifeq ($(ARCH),FreeBSD)
LDFLAGS += -lpthread
endif

LIB_DEFS = util.def

include $(TOP)/cr.mk

debug_opcodes.c: debug_opcodes.py $(APIFILES)
	@$(ECHO) Making the opcode debugger...
	@$(PYTHON) debug_opcodes.py > $@
