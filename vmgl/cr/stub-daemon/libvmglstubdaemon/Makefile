# Copyright (c) H. Andres Lagar-Cavilla, University of Toronto, 2006-2007, vmgl project */

TOP = ../..

LIBRARY = vmglstubdaemon

FILES = vmgl-stub-daemon

include $(TOP)/cr.mk

ifeq ($(ARCH), SunOS)
LDFLAGS += -lsocket -lnsl
endif
