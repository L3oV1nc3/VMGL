# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SPU = 1

SHARED = 1
LIBRARY = arrayspu
FILES = arrayspu \
	arrayspu_config \
	arrayspu_init

LIBRARIES = spuload crutil crstate

LIB_DEFS += arrayspu.def

STATE_STRING = (STATE)

include $(TOP)/cr.mk
