Auhtor: Joshua Humpich
Date: 22.05.2013


This is an short introduction to build the sources files with the VMGL-tarball and installing the VMGL-RPMs.
For more information take a look at the /vmgl-0.2/vmgl/README (this is from H. Andres Lagar-Cavilla)


Preparation
---------------------
Download the Tarball or the RPMs
For Tarball-Users: unzip/extract the the Tarball and copy the sources to any directory (should be /usr/local/share/ (as root!))


Building and installing the Tarball-Sources
---------------------
Head to the sources in the /vmgl-0.2/vmgl/ directory
Type 'make' to build a bunch of libaries and binaries
Type 'make install-host' to install the host sources on your system
Type 'make install-guest' to install the guest sources on your system

NOTE:	- Maybe there are some missing developement files (libaries), which are needed to compile the sources 
		-> e.g. missing devel files on my system: xaw-devel, xtrans-devel, xkbfile-devel, xfont-devel, pciaccess-devel- dmx-devel, xres-devel



Postproceccing
---------------------
After installing the binaries and libaries to your system you have to configure your X-Server.

(1) If you have a /etc/X11/xorg.conf file you just should add the following part, which loads the VMGL libary
Section "Module"
	...
	Load "vmgltext"
	...
EndSection

(2) If you haven't that file you have to generate a conf file.
Solve this problem by shutting down your X-Server and typing 'X -configure' as root
This generate a new xorg.conf file in /root/xorg.conf.new
Copy this file by typing 'cp /root/xorg.conf.new /etc/X11/xorg.conf' as root
Please check the new conf file for the module section which is described in step (1).
Their should be the load command for the vmgl libary!

After that, restart your X-Server...you can check the loading of the VMGL-libary by taking a look at the /var/log/Xorg.0.log file.
Their should be a part where your X-Server loads the VMGL-libary.


Building the VMGL RPMs
---------------------
If you want to generate your own VMGL RPMs you need two things
-> SPEC file from /vmgl-0.2/vmgl/extra/vmgl.spec
-> the Tarball 'vmgl-0.2.tar.bz2'

Copy the SPEC file in /home/<username>/rpmbuild/SPECS
Copy the tarball in /home/<username>/rpmbuild/SOURCES

Now open a terminal and type: 'rpmbuild -bb /home/<username>/rpmbuild/SPECS/vmgl.spec
After that you got two RPMs (host and guest) in /home/<username>/rpmbuild/RPMS/x86_64/



Installing the RPMs
---------------------
To install the guest and host RPM just type
for guest: 'rpm -ihv vmgl-guest-0.2-2.x86_64.rpm'
for host:  'rpm -ihv vmgl-host-0.2-2.x86_64.rpm'
