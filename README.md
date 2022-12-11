8514/A Blitter Code for the ATI Mach 8 (and Mach 32)
====================================================

This is code for the video, "I finally got this ATI
VGA Blitter working!" on my YouTube Channel,
PCRetroTech.

The code is very messy and just hacked together.

The interest in this surprised me, as I'd assumed
I'd be one of very few people with this chipset.

If anyone is interested in having me work on this
in return for some VERY nominal monetary support
for the project, here is what I think I could still
add:

* Clean up this code and fully document it
* Make it work on Mach8/Mach32 without the IBM
  adapter interface (who knows, maybe that'd be
  enough to get it to work on an XT, but no
  guarantees there)
* Support some of the ATI specific video modes
  including 16 bit colour
* Add keyboard control
* Sync to vertical retrace (might be needed on
  CRTs)

If you'd be interested in supporting a project
like that, please get in contact with me via
email (goodwillhart@googlemail.com) and if there's
sufficient interest I'll get to work.

Depending on the level of interest, I could be
induced to add support for loading arbitrary
(simple, compact) models created using Blender.

Building and Installation
=========================

This code can be built using Turbo C 2.0 and
Turbo Assembler 2.01 (WinPCWorld is your friend).

To build it, rename blitx.asm to triangle.asm
and blitx.c to hdidemo.c (where x is the version
you are interested in) and run make. There
are seven different versions of the program
which do progressively more complicated things.

Note that all except version 7 should run on
any card that is compatible with IBM 8514/A
*at the register level*. This includes the
Mach 8 and Mach 32, the WD9500 and the
82C480. Version 7 is ATI specific.

To build the code you will need afidata.h,
callafi.h and ibmafi.h from the original IBM
driver disk distributed with the 8514/A. See
8514/A Demo Diskette on this page
[http://mail.lipsia.de/~enigma/neu/ibmpc.html](http://mail.lipsia.de/~enigma/neu/ibmpc.html)

You will also need to run hdiload.exe from the
same diskette before running this program.

Enjoy!

William from PCRetroTech
