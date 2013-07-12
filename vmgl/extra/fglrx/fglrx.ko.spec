%define binary_version 0.1

Name:		 fglrx-kmod-xen
Summary:	 ATI fglrx kernel module for Xen dom0 kernels
Version:	 8.28.8
Release:	 1
License:	 GNU, BSD, Distributable containing closed-source components
Vendor:		 H. Andres Lagar-Cavilla
Group:		 User Interface/X Hardware Support
Source:		 %{name}-%{version}.tar.bz2
BuildRoot:	 %{_tmppath}/%{name}.%{version}-buildroot
URL:		 http://www.cs.toronto.edu/~andreslc/vmgl

%description

%prep
%setup -q -n fglrx

%build

%install
rm -rf $RPM_BUILD_ROOT

install -d $RPM_BUILD_ROOT/lib/modules/2.6.16.13-xen/kernel/drivers/char/drm

install fglrx.ko \
	$RPM_BUILD_ROOT/lib/modules/2.6.16.13-xen/kernel/drivers/char/drm/

%clean
rm -rf $RPM_BUILD_ROOT

%post
depmod -v 2.6.16.13-xen 

%postun 
depmod -v 2.6.16.13-xen

%files 
%defattr(-, root, root)

/lib/modules/2.6.16.13-xen/kernel/drivers/char/drm/fglrx.ko

%changelog
* Wed Sep 6 2006 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
- Initial version
