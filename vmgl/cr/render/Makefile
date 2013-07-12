# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..
SPU = 1

SHARED = 1
LIBRARY = renderspu
FILES = renderspu \
	renderspu_config \
	renderspu_init \
	renderspu_glx

LIBRARIES += crutil spuload

LIB_DEFS += render.def

include $(TOP)/cr.mk

LDFLAGS += -lXmu -lX11
