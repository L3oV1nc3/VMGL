# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SPU = 1
SHARED = 1
LIBRARY = errorspu
FILES = errorspu errorspu_init

LIBRARIES = crutil

PRECOMP = errorspu.c
SLOP = $(PRECOMP)

LIB_DEFS += error.def
include $(TOP)/cr.mk

errorspu.c: error.py
	@$(ECHO) Building the Error SPU
	@$(PYTHON) error.py > $@
