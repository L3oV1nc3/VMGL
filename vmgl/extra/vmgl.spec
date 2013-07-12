%define binary_version 0.2

Name:		 vmgl
Summary:	 VMGL, OpenGL Hardware Acceleration for Virtual Machines
Version:	 0.2
Release:	 2
License:	 GNU, BSD
Vendor:		 H. Andres Lagar-Cavilla, Joshua Humpich
Group:		 User Interface/X Hardware Support
Source:		 %{name}-%{version}.tar.bz2
BuildRoot:	 %{_tmppath}/%{name}.%{version}-buildroot
URL:		 http://www.cs.toronto.edu/~andreslc/vmgl

%description

%package host
Summary:	 VMGL, OpenGL Hardware Acceleration for Virtual Machines, host package
Group:		 User Interface/X Hardware Support
%description host

%package guest
Summary:	 VMGL, OpenGL Hardware Acceleration for Virtual Machines, guest package
Group:		 User Interface/X Hardware Support
%description guest

%prep
%setup -q

%build
cd vmgl
make

%install
cd vmgl
rm -rf $RPM_BUILD_ROOT

install -d $RPM_BUILD_ROOT/usr/bin \
	   $RPM_BUILD_ROOT/usr/lib64/vmgl \
	   $RPM_BUILD_ROOT/usr/lib64/xorg/modules/extensions \
	   $RPM_BUILD_ROOT/usr/share/doc/vmgl \
	   $RPM_BUILD_ROOT/usr/share/doc/vmgl/cr \
	   $RPM_BUILD_ROOT/usr/X11R6/lib/X11 \
	   $RPM_BUILD_ROOT/usr/share/X11/ \
	   $RPM_BUILD_ROOT/usr/include/X11/extensions \
	   $RPM_BUILD_ROOT/usr/share/pkgconfig \
	   $RPM_BUILD_ROOT/etc/ld.so.conf.d 	 

install dist/bin/glstub \
	$RPM_BUILD_ROOT/usr/bin/glstub

install dist/bin/stub-daemon \
	$RPM_BUILD_ROOT/usr/bin/stub-daemon

install dist/bin/vncviewer \
	$RPM_BUILD_ROOT/usr/bin/vncviewer_VMGL

install dist/bin/Xvnc \
	$RPM_BUILD_ROOT/usr/bin/Xvnc_VMGL 

install dist/lib/libarrayspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libarrayspu.so

install dist/lib/libcrutil.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libcrutil.so

install dist/lib/liberrorspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/liberrorspu.so

install dist/lib/libfeedbackspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libfeedbackspu.so

install dist/lib/libpackspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libpackspu.so

install dist/lib/libpassthroughspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libpassthroughspu.so

install dist/lib/librenderspu.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/librenderspu.so

install dist/lib/libspuload.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libspuload.so

install dist/lib/libvmgl.so \
	$RPM_BUILD_ROOT/usr/lib64/vmgl/libvmgl.so

install dist/libvmglext.so \
	$RPM_BUILD_ROOT/usr/lib64/xorg/modules/extensions/libvmglext.so

install vmglproto/vmgl_ext.h \
	$RPM_BUILD_ROOT/usr/include/X11/extensions/

install vmglproto/vmgl_extproto.h \
	$RPM_BUILD_ROOT/usr/include/X11/extensions/

install vmglproto/vmglproto.pc \
	$RPM_BUILD_ROOT/usr/share/pkgconfig/

install -m 0644 cr/doc/autostart.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/autostart.html

install -m 0644 cr/doc/banner.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/banner.html

install -m 0644 cr/doc/benchmarking.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/benchmarking.html

install -m 0644 cr/doc/chromium2.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/chromium2.html

install -m 0644 cr/doc/chromium.css \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/chromium.css
 
install -m 0644 cr/doc/codegen.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/codegen.html
 
install -m 0644 cr/doc/configapp.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configapp.html
 
install -m 0644 cr/doc/configglobal.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configglobal.html

install -m 0644 cr/doc/configoptions.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configoptions.html
 
install -m 0644 cr/doc/configscript.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configscript.html
 
install -m 0644 cr/doc/configserver.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configserver.html
 
install -m 0644 cr/doc/configtool.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/configtool.html
 
install -m 0644 cr/doc/conformance.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/conformance.html
 
install -m 0644 cr/doc/contexts.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/contexts.html
 
install -m 0644 cr/doc/contribute.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/contribute.html

install -m 0644 cr/doc/COPYRIGHT.LLNL \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/COPYRIGHT.LLNL

install -m 0644 cr/doc/COPYRIGHT.REDHAT \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/COPYRIGHT.REDHAT

install -m 0644 cr/doc/crdemo.conf \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/crdemo.conf

install -m 0644 cr/doc/crdemo_full.conf \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/crdemo_full.conf

install -m 0644 cr/doc/crexten.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/crexten.html

install -m 0644 cr/doc/crut.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/crut.html

install -m 0644 cr/doc/daughterships.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/daughterships.html

install -m 0644 cr/doc/dmx.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/dmx.html
 
install -m 0644 cr/doc/dynamichost.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/dynamichost.html
 
install -m 0644 cr/doc/extensions.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/extensions.html
 
install -m 0644 cr/doc/FAQ.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/FAQ.html
 
install -m 0644 cr/doc/glassquake.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/glassquake.html

install -m 0644 cr/doc/helloworld.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/helloworld.html

install -m 0644 cr/doc/help.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/help.html

install -m 0644 cr/doc/howitworks.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/howitworks.html

install -m 0644 cr/doc/index.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/index.html

install -m 0644 cr/doc/intro.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/intro.html

install -m 0644 cr/doc/invert.conf \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/invert.conf
 
install -m 0644 cr/doc/LICENSE \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/LICENSE
 
install -m 0644 cr/doc/LLNLcopy.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/LLNLcopy.html
  
install -m 0644 cr/doc/nav.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/nav.html
 
install -m 0644 cr/doc/newspu.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/newspu.html
  
install -m 0644 cr/doc/nonplanar.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/nonplanar.html
  
install -m 0644 cr/doc/packertest.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/packertest.html
 
install -m 0644 cr/doc/parallelapplication.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/parallelapplication.html
 
install -m 0644 cr/doc/performance.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/performance.html
 
install -m 0644 cr/doc/provided.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/provided.html
 
install -m 0644 cr/doc/reassembly_template.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/reassembly_template.html
 
install -m 0644 cr/doc/release.txt \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/release.txt
 
install -m 0644 cr/doc/sortlast_template.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/sortlast_template.html

install -m 0644 cr/doc/sourcecode.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/sourcecode.html
 
install -m 0644 cr/doc/spudebug.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/spudebug.html

install -m 0644 cr/doc/stereo.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/stereo.html

install -m 0644 cr/doc/testplan.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/testplan.html

install -m 0644 cr/doc/testprogs.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/testprogs.html
 
install -m 0644 cr/doc/threadedapplication.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/threadedapplication.html

install -m 0644 cr/doc/tilesort.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/tilesort.html
 
install -m 0644 cr/doc/tilesort_template.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/tilesort_template.html

install -m 0644 cr/doc/TO-DO \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/TO-DO

install -m 0644 cr/doc/versions.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/versions.html
 
install -m 0644 cr/doc/warped_tile.html \
 	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/warped_tile.html

install -m 0644 cr/LICENCE.txt \
	$RPM_BUILD_ROOT/usr/share/doc/vmgl/cr/

install -m 0644 README \
	$RPM_BUILD_ROOT/usr/share/doc/vmgl/
	
install -m 0644 LICENCE.txt \
	$RPM_BUILD_ROOT/usr/share/doc/vmgl/
	
install -m 0644 vnc_unixsrc/LICENCE.TXT -T \
	$RPM_BUILD_ROOT/usr/share/doc/vmgl/LICENCE.tightvnc

install -m 0644 dist/vmgl.conf \
	$RPM_BUILD_ROOT/etc/ld.so.conf.d/

ln -sf /usr/share/X11/rgb.txt $RPM_BUILD_ROOT/usr/X11R6/lib/X11/rgb.txt
ln -sf /usr/share/X11/fonts/ $RPM_BUILD_ROOT/usr/X11R6/lib/X11/fonts 

%clean
rm -rf $RPM_BUILD_ROOT

%post host
/sbin/ldconfig

%postun host
rm -rf /usr/lib64/vmgl
rm -f /usr/share/pkgconfig/vmglproto.pc
/sbin/ldconfig

%post guest
ln -s libvmgl.so /usr/lib64/vmgl/libGL.so.1.2
ln -s libGL.so.1.2 /usr/lib64/vmgl/libGL.so.1
ln -s libGL.so.1 /usr/lib64/vmgl/libGL.so 
/sbin/ldconfig

%postun guest
rm -rf /usr/lib64/vmgl
rm -rf /usr/X11R6
rm -f /usr/include/X11/extensions/vmgl_ext.h 
rm -f /usr/include/X11/extensions/vmgl_extproto.h
rm -f /usr/share/pkgconfig/vmglproto.pc
/sbin/ldconfig

%files host
%defattr(-, root, root)

%docdir /usr/share/doc/vmgl
/usr/share/doc/vmgl

#expected doc files
/usr/share/doc/vmgl/cr/autostart.html
/usr/share/doc/vmgl/cr/banner.html
/usr/share/doc/vmgl/cr/benchmarking.html
/usr/share/doc/vmgl/cr/chromium2.html
/usr/share/doc/vmgl/cr/chromium.css
/usr/share/doc/vmgl/cr/codegen.html
/usr/share/doc/vmgl/cr/configapp.html
/usr/share/doc/vmgl/cr/configglobal.html
/usr/share/doc/vmgl/cr/configoptions.html
/usr/share/doc/vmgl/cr/configscript.html
/usr/share/doc/vmgl/cr/configserver.html
/usr/share/doc/vmgl/cr/configtool.html
/usr/share/doc/vmgl/cr/conformance.html
/usr/share/doc/vmgl/cr/contexts.html
/usr/share/doc/vmgl/cr/contribute.html
/usr/share/doc/vmgl/cr/COPYRIGHT.LLNL
/usr/share/doc/vmgl/cr/COPYRIGHT.REDHAT
/usr/share/doc/vmgl/cr/crdemo.conf
/usr/share/doc/vmgl/cr/crdemo_full.conf
/usr/share/doc/vmgl/cr/crexten.html
/usr/share/doc/vmgl/cr/crut.html
/usr/share/doc/vmgl/cr/daughterships.html
/usr/share/doc/vmgl/cr/dmx.html
/usr/share/doc/vmgl/cr/dynamichost.html
/usr/share/doc/vmgl/cr/extensions.html
/usr/share/doc/vmgl/cr/FAQ.html
/usr/share/doc/vmgl/cr/glassquake.html
/usr/share/doc/vmgl/cr/helloworld.html
/usr/share/doc/vmgl/cr/help.html
/usr/share/doc/vmgl/cr/howitworks.html
/usr/share/doc/vmgl/cr/index.html
/usr/share/doc/vmgl/cr/intro.html
/usr/share/doc/vmgl/cr/invert.conf
/usr/share/doc/vmgl/cr/LICENSE
/usr/share/doc/vmgl/cr/LLNLcopy.html
/usr/share/doc/vmgl/cr/nav.html
/usr/share/doc/vmgl/cr/newspu.html
/usr/share/doc/vmgl/cr/nonplanar.html
/usr/share/doc/vmgl/cr/packertest.html
/usr/share/doc/vmgl/cr/parallelapplication.html
/usr/share/doc/vmgl/cr/performance.html
/usr/share/doc/vmgl/cr/provided.html
/usr/share/doc/vmgl/cr/reassembly_template.html
/usr/share/doc/vmgl/cr/release.txt
/usr/share/doc/vmgl/cr/sortlast_template.html
/usr/share/doc/vmgl/cr/sourcecode.html
/usr/share/doc/vmgl/cr/spudebug.html
/usr/share/doc/vmgl/cr/stereo.html
/usr/share/doc/vmgl/cr/testplan.html
/usr/share/doc/vmgl/cr/testprogs.html
/usr/share/doc/vmgl/cr/threadedapplication.html
/usr/share/doc/vmgl/cr/tilesort.html
/usr/share/doc/vmgl/cr/tilesort_template.html
/usr/share/doc/vmgl/cr/TO-DO
/usr/share/doc/vmgl/cr/versions.html
/usr/share/doc/vmgl/cr/warped_tile.html
#end doc files

#expected binaries
/usr/bin/vncviewer_VMGL
/usr/bin/glstub
/usr/bin/stub-daemon
#end binaries

#expected libaries
/usr/lib64/vmgl/libcrutil.so
/usr/lib64/vmgl/libspuload.so
/usr/lib64/vmgl/librenderspu.so
/usr/lib64/vmgl/liberrorspu.so
#end libaries

#expected vmglproto files
/usr/include/X11/extensions/vmgl_ext.h
/usr/include/X11/extensions/vmgl_extproto.h
/usr/share/pkgconfig/vmglproto.pc
#end vmglproto files

/etc/ld.so.conf.d/vmgl.conf

%files guest
%defattr(-, root, root)

%docdir /usr/share/doc/vmgl
/usr/share/doc/vmgl

#expected doc files
/usr/share/doc/vmgl/cr/autostart.html
/usr/share/doc/vmgl/cr/banner.html
/usr/share/doc/vmgl/cr/benchmarking.html
/usr/share/doc/vmgl/cr/chromium2.html
/usr/share/doc/vmgl/cr/chromium.css
/usr/share/doc/vmgl/cr/codegen.html
/usr/share/doc/vmgl/cr/configapp.html
/usr/share/doc/vmgl/cr/configglobal.html
/usr/share/doc/vmgl/cr/configoptions.html
/usr/share/doc/vmgl/cr/configscript.html
/usr/share/doc/vmgl/cr/configserver.html
/usr/share/doc/vmgl/cr/configtool.html
/usr/share/doc/vmgl/cr/conformance.html
/usr/share/doc/vmgl/cr/contexts.html
/usr/share/doc/vmgl/cr/contribute.html
/usr/share/doc/vmgl/cr/COPYRIGHT.LLNL
/usr/share/doc/vmgl/cr/COPYRIGHT.REDHAT
/usr/share/doc/vmgl/cr/crdemo.conf
/usr/share/doc/vmgl/cr/crdemo_full.conf
/usr/share/doc/vmgl/cr/crexten.html
/usr/share/doc/vmgl/cr/crut.html
/usr/share/doc/vmgl/cr/daughterships.html
/usr/share/doc/vmgl/cr/dmx.html
/usr/share/doc/vmgl/cr/dynamichost.html
/usr/share/doc/vmgl/cr/extensions.html
/usr/share/doc/vmgl/cr/FAQ.html
/usr/share/doc/vmgl/cr/glassquake.html
/usr/share/doc/vmgl/cr/helloworld.html
/usr/share/doc/vmgl/cr/help.html
/usr/share/doc/vmgl/cr/howitworks.html
/usr/share/doc/vmgl/cr/index.html
/usr/share/doc/vmgl/cr/intro.html
/usr/share/doc/vmgl/cr/invert.conf
/usr/share/doc/vmgl/cr/LICENSE
/usr/share/doc/vmgl/cr/LLNLcopy.html
/usr/share/doc/vmgl/cr/nav.html
/usr/share/doc/vmgl/cr/newspu.html
/usr/share/doc/vmgl/cr/nonplanar.html
/usr/share/doc/vmgl/cr/packertest.html
/usr/share/doc/vmgl/cr/parallelapplication.html
/usr/share/doc/vmgl/cr/performance.html
/usr/share/doc/vmgl/cr/provided.html
/usr/share/doc/vmgl/cr/reassembly_template.html
/usr/share/doc/vmgl/cr/release.txt
/usr/share/doc/vmgl/cr/sortlast_template.html
/usr/share/doc/vmgl/cr/sourcecode.html
/usr/share/doc/vmgl/cr/spudebug.html
/usr/share/doc/vmgl/cr/stereo.html
/usr/share/doc/vmgl/cr/testplan.html
/usr/share/doc/vmgl/cr/testprogs.html
/usr/share/doc/vmgl/cr/threadedapplication.html
/usr/share/doc/vmgl/cr/tilesort.html
/usr/share/doc/vmgl/cr/tilesort_template.html
/usr/share/doc/vmgl/cr/TO-DO
/usr/share/doc/vmgl/cr/versions.html
/usr/share/doc/vmgl/cr/warped_tile.html
#end doc files

#expected binaries
/usr/bin/Xvnc_VMGL
#end binaries

#expected libaries
/usr/lib64/vmgl/libcrutil.so
/usr/lib64/vmgl/libspuload.so
/usr/lib64/vmgl/libvmgl.so
/usr/lib64/vmgl/libfeedbackspu.so
/usr/lib64/vmgl/libarrayspu.so
/usr/lib64/vmgl/libpackspu.so
/usr/lib64/vmgl/liberrorspu.so
/usr/lib64/vmgl/libpassthroughspu.so
/usr/lib64/xorg/modules/extensions/libvmglext.so
#end libaries

#expected vmglproto files
/usr/include/X11/extensions/vmgl_ext.h
/usr/include/X11/extensions/vmgl_extproto.h
/usr/share/pkgconfig/vmglproto.pc
#end vmglproto files

#expected Xvnv path and files
/usr/X11R6/lib/X11/rgb.txt
/usr/X11R6/lib/X11/fonts 
#end Xvnc path and files

/etc/ld.so.conf.d/vmgl.conf

%changelog
* Fri May 17 2012 Joshua Humpich <l3ov1nc3@googlemail.com>
- Version 0.2 released, ported VMGL on current X (X11 R7.7)

* Tue Feb 13 2007 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
- Still battling, 0.2

* Tue Sep 14 2006 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
- Fixes, still 0.1

* Mon Aug 29 2006 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
- Initial version
