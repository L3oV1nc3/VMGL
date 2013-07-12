VMGL

Hardware accelerated OpenGL graphics for Virtual Machines
H. Andres Lagar-Cavilla, University of Toronto, 2006
http://www.cs.toronto.edu/~andreslc/vmgl
bugs,requests,hatemail,lovemail: andreslc@cs.toronto.edu


* Licensing:
  BSD-modified licence. This is Free and Open Source Software.
  The file LICENCE.txt specifies the terms of use and distribution.


* Prerequisites:
  The usual suspects: make, gcc. Plus Imake, python and OpenGL library
  headers.


* Installation:

Type make. This will build a number of libraries and binaries that you
can fin in the top-level dist/ directory. You have to build them both
in your guest (VM or domain) and host (domain0 or OS hosting VMware
VMM).  Then make install-host (in the host, as you suspected) and make
install-guest in your guest (as you also suspected.) By default things
are installed in /usr/local/, but that can be overriden with
INSTALLPATH for make install-{guest|host}.

*) Xen prerequisite: DRM kernel modules for Xen domain0

If your graphics card belongs to the Intel family (i810, i915, etc)
dom0 kernels already contain DRM drivers. Unfortunately, most humans
use ATI and Nvidia cards, whose kernel modules need modifications to
get along with dom0.

For ATI: cd fglrx, and use that README to build a dom0-friendly
fglrx.ko kernel module. The module should go in /lib/modules/`uname
-r`/kernel/drivers/char/drm Run depmod -a, make sure to change
/etc/X11/xorg.conf to use the fglrx driver, and restart X.

For Nvidia: There are similar patches for nvidia's driver. I cannot
attest to their effectivenes as I don't usually have an Nvidia card
lying around.  Therefore, I forward you to the following sites

http://lists.xensource.com/archives/html/xen-devel/2006-03/msg00615.html
http://rpm.livna.org

The net result should be that the output of glxinfo, in dom0,
indicates "Direct Rendering: yes". This means that you can run your
OpenGL apps in dom0 with hardware acceleration, but not yet in domU.


* Patches:
  Go into extra/patches dir. Apply and rebuild at your risk :) Patches provided for:
  + Tightvnc X server if you don't want to use our Xvnc server
  + Tightvnc vncviewer if you don't want to use our vncviewer.
  + Xorg 7.1 tree to compile libvmglext.so if you don't want to use
    our enclosed binary.
  + Xen 3 and above ioemu patch for hvm SDL viewer. We don't build that.
  + Xen 3.0.4 and above sdlfb patch for paravirt framebuffer SDL
    viewer. We don't build that.


* Use.  

Ok, so now you have all the pieces in place. Make sure ports in the
range of the 7000's are open between guest and host.

There are three ways to use:

-- Guest using Xvnc

0) Typically a Xen paravirt domain with no graphics device. Also use
   this for VMware until we hack our way around their closed-source viewer.
1) Start Xvnc in guest, usually by typing vncserver. 
2) Connect to it from the host with vncviewer
3) After authentication, vncviewer barfs something like
    "Set GLSTUB var in domU to point to port <port>"
4) First thing you do in your guest desktop is set the environment var 
    GLSTUB = <host_hostname>:<port_vncviewer_barfed>
5) Start your GL app
6) Fun

-- Xen Paravirt domain using virtual framebuffer, or
   hvm domain (using qemu-dm)
   
These two types of linux Xen domains run a standard X server
internally; by different means, you, the user, end up interacting with
the domain through a viewer (vnc or sdl). Support for this model is
provided by an X server loadable extension module called
libvmglext.so. The binary will be placed in
/usr/lib/xorg/modules/extensions and you need to invoke it on Xorg
startup by adding the following to /etc/X11/xorg.conf:

Section "Module"
    ...
    Load "vmglext"
    ...

Once you have started your VM X server with the vmglext extension,
follow step 2) to 6) above. If using an sdl viewer (qemu-dm in sdl
mode, or sdlfb) look for the "Set GLSTUB var in domU to point to port
<port>" string at the tail of the corresponding log
(~/.stub-daemon.log for sdlfb, /var/log/xen/qemu-dm.<pid>.log for
qemu-dm). No better way yet.

We could "coerce" VMware to do the same thing with the X server inside
the VM. Just mail me for the dirty trickery.

-- X forwarding. Independent of the type of VM:

1) In the host, start stub-daemon. As implied, it will daemonize. Log
   messages go to ~/.stub-daemon.log
2) Set up X forwarding from guest to host
  2.1) Using ssh
       in the host, ssh -X to guest
  2.2) Traditional
       in the guest, set DISPLAY=<host_hostname>:0
       make sure the host allows incoming X connections from the guest.
3) Set GLSTUB = <host_hostname>:7000
4) Start your GL app
5) Fun

There is a simple reason why X forwarding is there. Quake 3, which I
fancy playing, doesn't play nice with input from VNC. Now, when you do
X forwarding, Quake 3 will attempt (like many games) to take control
over your X server video settings. Typically, remote clients are not
allowed to do so. If you're using one of those apps, go to
/etc/X11/xorg.conf in the host and add:

Section "ServerFlags"
#    Option "DisableVidModeExtension" "yes"
    Option "AllowNonLocalXvidtune" "yes"
EndSection

Restart X and you're good to go. The commented option is there 
for the paranoids among us.


* Acknowledgements
 - Greg Humphries, Mike Houston, Ian Buck, Matthew Eldridge and others,
   for the wonderful work in starting Chromium.
 - Brian Paul and others, for their relentless effort in maintaining
   mesa and chromium as high quality open-source projects.
 - Jacob Gorm-Hansen, whose patches for DRM kernel modules have been
   adapted here.
 - Anil Madhavapeddy and Dave Scott from Xensource, for mentoring.
 - Google, for Summer of Code 2006.
 - Niraj Tolia, Eyal de Lara and M. Satyanarayanan, research colleagues
   and advisors.

   
