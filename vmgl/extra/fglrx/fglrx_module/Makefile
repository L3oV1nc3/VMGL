## Copyright (c) H. Andres Lagar Cavilla, University of Toronto, 2006-2007. VMGL project.

CC		 = gcc
PAGE_ATTR_FIX	 = $(shell grep -c ' change_page_attr$$' /proc/kallsyms)
GCC_VER_MAJ      = $(shell $(CC) --version | head -n 1 | sed -e 's/[^0-9. ]//g;s/^ *//;s/^\(.\)\..*$$/\1/')
IS_SMP		 = $(shell grep -c SMP /proc/version)

obj-m           += fglrx.o
fglrx-libs      += libfglrx_ip.a.GCC$(GCC_VER_MAJ)
fglrx-c-objs    += firegl_public.o
fglrx-objs      += $(fglrx-c-objs) $(fglrx-libs)
fglrx-hdrs      += firegl_public.h
drm-hdrs        += drm.h drmP.h drm_os_linux.h drm_proc.h drm_compat.h

ifeq ($(PAGE_ATTR_FIX),)
PAGE_ATTR_FIX   =1
endif

EXTRA_CFLAGS    += \
                -I$(PWD) \
                -DFGL \
                -DFGL_LINUX \
                -DFGL_GART_RESERVED_SLOT \
                -DFGL_LINUX253P1_VMA_API \
                -DPAGE_ATTR_FIX=$(PAGE_ATTR_FIX) \

MODFLAGS	+= \
		-DMODULE
		-DATI_AGP_HOOK \
		-DATI \
		-DFGL -DFGL_RX \
		-DFGL_CUSTOM_MODULE \
		-DPAGE_ATTR_FIX=$(PAGE_ATTR_FIX)

ifeq ($(IS_SMP),1)
MODFLAGS	+= -D__SMP__
endif

ifeq ($(KERNELRELEASE),)
# on first call from remote location we get into this path
# whilst on second call all is managed by the embedding kernel makefile

KVER            = $(shell uname -r)

ifndef KERNEL_PATH
KERNEL_PATH     = /lib/modules/$(KVER)/build
endif

KDIR            = $(KERNEL_PATH)
PWD             = $(shell pwd)

fglrx-cfiles    = ${fglrx-c-objs:.o=.c}

# default:: kmod_build

kmod_build:: $(fglrx-libs) $(fglrx-cfiles) $(fglrx-hdrs) $(drm-hdrs)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

# distclean:: clean

clean::
	rm -rf *.o fglrx.mod.c *.ko .??*

endif

# ### EOF ###
