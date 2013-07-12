# Top-level VMGL makefile
# Copyright (c) H. Andres Lagar-Cavilla, University of Toronto, 2006-2007

MAKE = make
XMKMF = xmkmf
INSTALL = install
INSTALL_FLAGS := -s -m 0755
CONFIGURE = ./configure
RM = rm

TOP = $(shell pwd)
SUBDIRS = cr vnc_unixsrc vnc_unixsrc/Xvnc xorg-server-1.7.7

DOM0_BIN = vncviewer glstub stub-daemon
DOMU_BIN = Xvnc
DOM0_LIB = libcrutil.so libspuload.so liberrorspu.so librenderspu.so
DOMU_LIB = libcrutil.so libspuload.so liberrorspu.so libvmgl.so libarrayspu.so libfeedbackspu.so libpackspu.so libpassthroughspu.so

ifdef VMGLPATH
INSTALLPATH = $(VMGLPATH)
else
INSTALLPATH = /usr/
endif

all:
	$(INSTALL) -d $(TOP)/dist/lib/
	$(INSTALL) -d $(TOP)/dist/bin/
	(cd cr ; \
	    $(MAKE) ; \
	    $(INSTALL) $(INSTALL_FLAGS) dist/lib/*.so $(TOP)/dist/lib/ ; \
	    $(INSTALL) $(INSTALL_FLAGS) dist/bin/* $(TOP)/dist/bin/ ; \
	) || exit 1
	(cd vnc_unixsrc ; \
	    $(MAKE) ; \
	    $(INSTALL) $(INSTALL_FLAGS) vncviewer/vncviewer ../dist/bin/vncviewer ; \
	) || exit 1
	(cd vnc_unixsrc/Xvnc ; \
	    $(CONFIGURE) ; \
	    $(MAKE) ; \
	    $(INSTALL) $(INSTALL_FLAGS) programs/Xserver/Xvnc ../../dist/bin/Xvnc ; \
	) || exit 1
	(cd vmglproto ; \
		$(CONFIGURE) ; \
		$(MAKE) ; \
		$(INSTALL)  $(TOP)/vmglproto/vmgl_ext.h $(INSTALLPATH)/include/X11/extensions ; \
		$(INSTALL)  $(TOP)/vmglproto/vmgl_extproto.h $(INSTALLPATH)/include/X11/extensions ; \
		$(INSTALL)  $(TOP)/vmglproto/vmglproto.pc $(INSTALLPATH)/share/pkgconfig ; \
	) || exit 1
	(cd xorg-server-1.7.7 ; \
		$(CONFIGURE) ; \
		$(MAKE) ; \
		$(INSTALL) $(INSTALL_FLAGS) hw/xfree86/dixmods/.libs/libvmglext.so ../dist/ ;\
	) || exit 1

clean:
	@for dir in $(SUBDIRS) ; do \
		(cd $$dir ; $(MAKE) clean) ; \
	done

install-host: all
	$(INSTALL) -d $(INSTALLPATH)/bin/
	for binary in $(DOM0_BIN); do \
	    $(INSTALL) $(TOP)/dist/bin/$$binary $(INSTALLPATH)/bin/$$binary ; \
	done
	$(INSTALL) -d $(INSTALLPATH)/lib64/vmgl
	for library in $(DOM0_LIB); do \
	    $(INSTALL) $(TOP)/dist/lib/$$library $(INSTALLPATH)/lib64/vmgl/$$library ; \
	done
	$(INSTALL)  $(TOP)/dist/libvmglext.so $(INSTALLPATH)/lib64/xorg/modules/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmgl_ext.h $(INSTALLPATH)/include/X11/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmgl_extproto.h $(INSTALLPATH)/include/X11/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmglproto.pc $(INSTALLPATH)/share/pkgconfig
	echo "$(INSTALLPATH)/lib64/vmgl/" > /etc/ld.so.conf.d/vmgl.conf
	/sbin/ldconfig

install-guest: all
	$(INSTALL) -d $(INSTALLPATH)/bin/
	for binary in $(DOMU_BIN); do \
	    $(INSTALL) $(TOP)/dist/bin/$$binary $(INSTALLPATH)/bin/$$binary ; \
	done
	$(INSTALL) -d $(INSTALLPATH)/lib64/vmgl
	for library in $(DOMU_LIB); do \
	    $(INSTALL) $(TOP)/dist/lib/$$library $(INSTALLPATH)/lib64/vmgl/$$library ; \
	done
	ln -sf libvmgl.so $(INSTALLPATH)/lib64/vmgl/libGL.so.1.2
	ln -sf libGL.so.1.2 $(INSTALLPATH)/lib64/vmgl/libGL.so.1
	ln -sf libGL.so.1 $(INSTALLPATH)/lib64/vmgl/libGL.so
	$(INSTALL)  $(TOP)/dist/libvmglext.so $(INSTALLPATH)/lib64/xorg/modules/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmgl_ext.h $(INSTALLPATH)/include/X11/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmgl_extproto.h $(INSTALLPATH)/include/X11/extensions
	$(INSTALL)  $(TOP)/vmglproto/vmglproto.pc $(INSTALLPATH)/share/pkgconfig
	$(INSTALL) -d /usr/X11R6/lib/X11/
	ln -sf /usr/share/X11/rgb.txt $(INSTALLPATH)/X11R6/lib/X11/rgb.txt
	ln -sf /usr/share/X11/fonts $(INSTALLPATH)/X11R6/lib/X11/fonts
	echo "$(INSTALLPATH)/lib64/vmgl/" > /etc/ld.so.conf.d/vmgl.conf
	/sbin/ldconfig

uninstall-host: 
	for binary in $(DOM0_BIN); do \
	    $(RM) -f $(INSTALLPATH)/bin/$$binary ; \
	done
	$(RM) -rf $(INSTALLPATH)/lib64/vmgl
	$(RM) -f $(INSTALLPATH)/include/X11/extensions/vmgl_ext.h
	$(RM) -f $(INSTALLPATH)/include/X11/extensions/vmgl_extproto.h
	$(RM) -f $(INSTALLPATH)/share/pkgconfig/vmglproto.pc
	$(RM) /etc/ld.so.conf.d/vmgl.conf
	/sbin/ldconfig

uninstall-guest: 
	for binary in $(DOMU_BIN); do \
	    $(RM) -f $(INSTALLPATH)/bin/$$binary ; \
	done
	$(RM) -rf $(INSTALLPATH)/lib64/vmgl
	$(RM) -f /usr/lib64/xorg/modules/extensions/libvmglext.so
	$(RM) -f $(INSTALLPATH)/include/X11/extensions/vmgl_ext.h
	$(RM) -f $(INSTALLPATH)/include/X11/extensions/vmgl_extproto.h
	$(RM) -f $(INSTALLPATH)/share/pkgconfig/vmglproto.pc
	unlink /usr/X11R6/lib/X11/rgb.txt
	unlink /usr/X11R6/lib/X11/fonts
	$(RM) -rf /usr/X11R6
	$(RM) /etc/ld.so.conf.d/vmgl.conf
	/sbin/ldconfig

